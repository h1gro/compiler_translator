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

class EncodedData;

class UncodedData
{
private:
    size_t length;

    class OperType
    {
    public:
        bool is_min;
        int value;

        OperType() : is_min(false), value(0) {}

        explicit OperType(const std::string& fields_value) : is_min(false), value(0)
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

    std::vector<Field> fields;

    class Instruction
    {
    public:
        std::map<size_t, std::string> insns;
        std::vector<std::string> operands;
        std::string format;
        std::string comment;

        Instruction(const std::string& format_, const std::string& comment_, const nlohmann::json& data)
            : insns(), operands(), format(format_), comment(comment_)
        {
            if (data.contains("insns"))
            {
                size_t i = 0;
                for (const auto& cmd : data["insns"])
                {
                    insns[i] = cmd.get<std::string>();
                    i++;
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
    };

    std::vector<std::unique_ptr<Instruction>> instructions;

public:
    explicit UncodedData(const std::string& file_path) : length(0), fields(), instructions()
    {
        std::ifstream file_stream(file_path);
        if (!file_stream){throw std::invalid_argument("Error in opening file!");}

        nlohmann::json data;
        file_stream >> data;

        const std::string len_str = data.value("length", "0");
        length = static_cast<size_t>(std::stoi(len_str));

        if (length <= 0)
        {
            throw std::invalid_argument("Input JSON is invalid: length must be positive");
        }

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

    EncodedData encode() const;

    size_t get_length() const {return length;}

    void print_fields() const;
    void print_instructions() const;
};
