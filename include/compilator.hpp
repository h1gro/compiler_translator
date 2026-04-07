#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <json.hpp>

class CmdEncodingSystem
{
private:
    size_t length;

    class OperType
    {
    public:
        bool is_min;
        int value;

        OperType() : is_min(false), value(0) {}

        explicit OperType(const std::string& fields_value)
        {
            if (fields_value.substr(0,2) == ">=")
            {
                is_min = true;
                value = std::stoi(fields_value.substr(2));
            }
            else
            {
                is_min = false;
                value = std::stoi(fields_value);
            }
        }
    };

    class Field
    {
    public:
        std::string operand;
        OperType argument;

        Field(std::string field_name, const std::string& field_value) : operand(std::move(field_name)), argument(field_value) {}
    };

    struct LayoutField
    {
        std::string name;
        std::vector<bool> active_in_formats;
        //default values:
        int size        = 0;
        bool is_min     = false;
        bool is_service = false;
        int msb = -1;
        int lsb = -1;
    };

    std::vector<Field> fields;

    class Instruction
    {
    public:
        std::map<int, std::string> insns;
        std::vector<std::string> operands;
        std::string format;
        std::string comment;

        Instruction(const std::string& format_, const std::string& comment_, const nlohmann::json& data) : format(format_), comment(comment_)
        {
            if (data.contains("insns"))
            {
                int i = 0;
                for (const auto& cmd : data["insns"])
                {
                    insns[i] = cmd.get<std::string>();
                    ++i;
                }
            }

            if (data.contains("operands"))
            {
                for (const auto& oper : data["operands"])
                {
                    operands.push_back(oper.get<std::string>());
                }
            }
        }

        //Methods for printing data of CmdEncodingSystem in the standart output stream
        void print_instruction() const;
        void print_insns()       const;
        void print_operands()    const;

        bool uses_operand(const std::string& operand_name) const
        {
            return (std::find(operands.begin(), operands.end(), operand_name) != operands.end());
        }
    };

    std::vector<std::unique_ptr<Instruction>> instructions;

private:
    static std::string to_upper(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        return value;
    }

    static int bits_required(size_t count)
    {
        if (count <= 1){return 0;}

        int bits      = 0;
        size_t values = 1;

        while (values < count)
        {
            values <<= 1U;
            bits++;
        }
        return bits;
    }

    static std::string to_binary(int val, int width)
    {
        std::string binary_string;
        binary_string.reserve(width);

        for (int i = width - 1; i >= 0; --i)
        {
            binary_string.push_back(((val >> i) & 1) ? '1' : '0');
        }

        return binary_string;
    }

    static bool intersects(const LayoutField& lhs, const LayoutField& rhs)
    {
        const int left = std::max(lhs.lsb, rhs.lsb);
        const int right = std::min(lhs.msb, rhs.msb);
        return left <= right;
    }

    static bool conflicts(const LayoutField& lhs, const LayoutField& rhs)
    {
        const size_t limit = std::min(lhs.active_in_formats.size(), rhs.active_in_formats.size());
        for (size_t i = 0; i < limit; ++i)
        {
            if (lhs.active_in_formats[i] && rhs.active_in_formats[i])
            {
                return true;
            }
        }
        return false;
    }

    LayoutField make_service_field(const std::string& name, int size) const
    {
        LayoutField lf;

        lf.name       = name;
        lf.size       = size;
        lf.is_min     = false;
        lf.is_service = true;
        lf.active_in_formats.assign(instructions.size(), false);

        return lf;
    }

    LayoutField make_user_field(const Field& field_spec) const
    {
        LayoutField lf;

        lf.name       = field_spec.operand;
        lf.size       = field_spec.argument.value;
        lf.is_min     = field_spec.argument.is_min;
        lf.is_service = false;
        lf.active_in_formats.assign(instructions.size(), false);

        return lf;
    }

    void expand_min_fields(std::vector<LayoutField>& layout) const
    {
        if (instructions.empty())
        {
            return;
        }

        std::vector<int> free_bits(instructions.size(), static_cast<int>(length));

        for (size_t fmt = 0; fmt < instructions.size(); fmt++)
        {
            for (const auto& lf : layout)
            {
                if ((fmt < lf.active_in_formats.size()) && (lf.active_in_formats[fmt]))
                {
                    free_bits[fmt] -= lf.size;
                }
            }

            if (free_bits[fmt] < 0)
            {
                throw std::runtime_error(
                    "Format '" + instructions[fmt]->format + "' exceeds instruction length");
            }
        }

        bool changed = true;
        while (changed)
        {
            changed = false;

            for (auto& lf : layout)
            {
                if ((lf.is_service) || (!lf.is_min)){continue;}

                bool used_somewhere = false;
                bool can_grow       = true;

                for (size_t fmt = 0; fmt < instructions.size(); ++fmt)
                {
                    if (fmt < lf.active_in_formats.size() && lf.active_in_formats[fmt])
                    {
                        used_somewhere = true;
                        if (free_bits[fmt] <= 0)
                        {
                            can_grow = false;
                            break;
                        }
                    }
                }

                if (!used_somewhere || !can_grow){continue;}

                lf.size++;
                changed = true;

                for (size_t fmt = 0; fmt < instructions.size(); ++fmt)
                {
                    if ((fmt < lf.active_in_formats.size()) && (lf.active_in_formats[fmt]))
                    {
                        free_bits[fmt]--;
                    }
                }
            }
        }
    }

    std::vector<LayoutField> build_global_layout() const
    {
        if (instructions.empty()){return {};}

        std::vector<LayoutField> layout;
        layout.reserve(fields.size() + 2U);

        const int format_bits = bits_required(instructions.size());
        int opcode_bits = 0;

        for (const auto& instr : instructions)
        {
            if (!instr){throw std::invalid_argument("Invalid ref in instructions!");}
            opcode_bits = std::max(opcode_bits, bits_required(instr->insns.size()));
        }

        if (format_bits > 0)
        {
            auto f = make_service_field("F", format_bits);
            std::fill(f.active_in_formats.begin(), f.active_in_formats.end(), true);
            layout.push_back(std::move(f));
        }

        if (opcode_bits > 0)
        {
            auto opcode = make_service_field("OPCODE", opcode_bits);
            for (size_t i = 0; i < instructions.size(); ++i)
            {
                opcode.active_in_formats[i] = instructions[i]->insns.size() > 1;
            }
            layout.push_back(std::move(opcode));
        }

        for (const auto& field_spec : fields)
        {
            auto lf = make_user_field(field_spec);
            for (size_t i = 0; i < instructions.size(); ++i)
            {
                lf.active_in_formats[i] = instructions[i]->uses_operand(field_spec.operand);
            }

            const bool active_somewhere = std::any_of(lf.active_in_formats.begin(), lf.active_in_formats.end(), [](bool value) {return value;});

            if (active_somewhere)
            {
                layout.push_back(std::move(lf));
            }
        }

        for (size_t fmt_idx = 0; fmt_idx < instructions.size(); ++fmt_idx)
        {
            int total_bits = 0;
            for (const auto& lf : layout)
            {
                if (lf.active_in_formats[fmt_idx])
                {
                    total_bits += lf.size;
                }
            }

            if (total_bits > static_cast<int>(length))
            {
                throw std::runtime_error(
                    "Format '" + instructions[fmt_idx]->format + "' requires " + std::to_string(total_bits) +
                    " bits, but instruction length is " + std::to_string(length));
            }
        }

        expand_min_fields(layout);

        std::vector<size_t> order(layout.size());
        std::iota(order.begin(), order.end(), 0U);

        std::stable_sort(order.begin(), order.end(), [&](size_t lhs_idx, size_t rhs_idx) {
            const auto active_count = [](const LayoutField& lf) {
                return static_cast<int>(std::count(lf.active_in_formats.begin(), lf.active_in_formats.end(), true));
            };

            const LayoutField& lhs = layout[lhs_idx];
            const LayoutField& rhs = layout[rhs_idx];

            if (lhs.is_service != rhs.is_service)
            {
                return lhs.is_service > rhs.is_service;
            }

            const int lhs_active = active_count(lhs);
            const int rhs_active = active_count(rhs);
            if (lhs_active != rhs_active)
            {
                return lhs_active > rhs_active;
            }

            if (lhs.size != rhs.size)
            {
                return lhs.size > rhs.size;
            }

            return lhs_idx < rhs_idx;
        });

        std::vector<size_t> placed;
        placed.reserve(layout.size());

        for (size_t idx : order)
        {
            LayoutField& current = layout[idx];
            bool assigned = false;

            for (int msb = static_cast<int>(length) - 1; msb >= current.size - 1; --msb)
            {
                current.msb = msb;
                current.lsb = msb - current.size + 1;

                bool ok = true;
                for (size_t prev_idx : placed)
                {
                    const LayoutField& prev = layout[prev_idx];
                    if (conflicts(current, prev) && intersects(current, prev))
                    {
                        ok = false;
                        break;
                    }
                }

                if (ok)
                {
                    assigned = true;
                    placed.push_back(idx);
                    break;
                }
            }

            if (!assigned){throw std::runtime_error("Unable to place field '" + current.name + "' into the global layout");}
        }

        return layout;
    }

    std::vector<const LayoutField*> collect_active_layout(const Instruction& format_desc,
                                                          size_t format_index,
                                                          const std::vector<LayoutField>& global_layout) const
    {
        std::vector<const LayoutField*> active;
        active.reserve(global_layout.size());

        for (const auto& lf : global_layout)
        {
            if (format_index < lf.active_in_formats.size() && lf.active_in_formats[format_index])
            {
                active.push_back(&lf);
            }
        }

        std::sort(active.begin(), active.end(), [](const LayoutField* lhs, const LayoutField* rhs)
        {
            if (lhs->msb != rhs->msb)
            {
                return lhs->msb > rhs->msb;
            }
            return lhs->lsb > rhs->lsb;
        });

        if (!active.empty())
        {
            for (size_t i = 1; i < active.size(); ++i)
            {
                if (intersects(*active[i - 1], *active[i]))
                {
                    throw std::runtime_error(
                        "Overlapping fields in format '" + format_desc.format + "': '" + active[i - 1]->name +
                        "' and '" + active[i]->name + "'");
                }
            }
        }

        return active;
    }

    static std::string escape_json_string(const std::string& value)
    {
        return nlohmann::json(value).dump();
    }

    //Methods for writing encoding data in the output file
    void write_indent       (std::ostream& output_stream, int spaces) const;
    void write_inline_field (std::ostream& output_stream, const nlohmann::json& field_json) const;
    void write_fields_array (std::ostream& output_stream, const nlohmann::json& fields_json, int indent) const;

    static nlohmann::json make_json_field(const std::string& name, int msb, int lsb, const std::string& value)
    {
        nlohmann::json inner;
        inner["msb"] = msb;
        inner["lsb"] = lsb;
        inner["value"] = value;

        nlohmann::json obj;
        obj[name] = inner;
        return obj;
    }

    nlohmann::json output_fields(size_t format_index, int opcode, const std::vector<LayoutField>& global_layout) const
    {
        if ((format_index >= instructions.size()) || (!instructions[format_index]))
        {
            throw std::invalid_argument("Invalid format index in output_fields!");
        }

        const Instruction& format_desc = *instructions[format_index];
        const auto active = collect_active_layout(format_desc, format_index, global_layout);

        nlohmann::json arr = nlohmann::json::array();

        int current_msb = static_cast<int>(length) - 1;
        int res_index = 0;

        for (const LayoutField* lf : active)
        {
            if (current_msb > lf->msb)
            {
                arr.push_back(
                    make_json_field("RES" + std::to_string(res_index),
                                    current_msb,
                                    lf->msb + 1,
                                    std::string(current_msb - lf->msb, '0'))
                );
                ++res_index;
            }

            const int width = lf->msb - lf->lsb + 1;
            std::string value;

            if (lf->name == "F")
            {
                value = to_binary(static_cast<int>(format_index), width);
            }
            else if (lf->name == "OPCODE")
            {
                value = to_binary(opcode, width);
            }
            else
            {
                value = "+";
            }

            const std::string out_name =
                (lf->name == "F" || lf->name == "OPCODE" || lf->name.rfind("RES", 0) == 0)
                    ? lf->name
                    : to_upper(lf->name);

            arr.push_back(make_json_field(out_name, lf->msb, lf->lsb, value));
            current_msb = lf->lsb - 1;
        }

        if (current_msb >= 0)
        {
            arr.push_back(
                make_json_field("RES" + std::to_string(res_index),
                                current_msb,
                                0,
                                std::string(current_msb + 1, '0'))
            );
        }

        return arr;
    }

public:
    explicit CmdEncodingSystem(const std::string& file_path)
    {
        std::ifstream file_stream(file_path);
        if (!file_stream){throw std::invalid_argument("Error in opening file!");}

        nlohmann::json data;
        file_stream >> data;

        const std::string len_str = data.value("length", "0");
        length = static_cast<size_t>(std::stoi(len_str));

        if (data.contains("fields"))
        {
            for (const auto& field : data["fields"])
            {
                for (auto it = field.begin(), end = field.end(); it != end; ++it)
                {
                    fields.emplace_back(it.key(), it.value().get<std::string>());
                }
            }
        }

        if (data.contains("instructions"))
        {
            for (const auto& instr_i : data["instructions"])
            {
                Instruction data_elem(instr_i.value("format", std::string{}),
                                      instr_i.value("comment", std::string{}),
                                      instr_i);
                instructions.emplace_back(std::make_unique<Instruction>(std::move(data_elem)));
            }
        }
    }

    size_t get_length() const {return length;}

    void get_fields() const
    {
        for (const auto& field : fields)
        {
            std::cout << field.operand << " : ";
            if (field.argument.is_min)
            {
                std::cout << ">=";
            }
            std::cout << field.argument.value << std::endl;
        }
    }

    void dump_instructions() const;
    void dump_coding_format(std::string file_path) const;
};
