#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <list>
#include <string>
#include <unordered_map>

#include <json.hpp>

class CmdEncodingSystem
{
private:
    size_t length; //bytes

    class OperType
    {
    public:
        bool is_min;
        int  value;

        OperType() : is_min(false), value(0) {}

        OperType(std::string fields_value)
        {
            if (fields_value.substr(0,2) == ">=")
            {
                is_min = true;
                value  = std::stoi(fields_value.substr(2));
            }

            else
            {
                is_min = false;
                value  = std::stoi(fields_value);
            }
        }
    };

    class Field
    {
    public:
        std::string operand;
        OperType argument;

        Field(std::string field_name, std::string field_value) : operand(field_name), argument(field_value) {}
    };

    std::vector<Field> fields;

    class Instructure
    {
    public:
        std::map<int, std::string> insns;
        std::vector<std::string> operands;
        std::string format;
        std::string comment;

        Instructure(const std::string& format_, const std::string& comment_, const nlohmann::json& data) : format(format_), comment(comment_)
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

        void print_instructure () const
        {
            std::cout << "Format: " << format << "\n" << "Insns:" << std::endl;
            print_insns();
            std::cout << "Operands:" << std::endl;
            print_operands();
            std::cout << "Comment: " << comment << "\n" <<std::endl;
        }

        void print_insns () const
        {
            for (const auto& ins : insns)
            {
                std::cout << ins.first << " : " << ins.second << std::endl;
            }
        }

        void print_operands () const
        {
            for (const auto& oper : operands)
            {
                std::cout << oper << std::endl;
            }
        }
    };

    std::vector<std::unique_ptr<Instructure>> instructions;

public:
    CmdEncodingSystem(std::string file_path)
    {
        std::ifstream file_stream(file_path);

        if (!file_stream) {throw std::invalid_argument("Error in opening file!");}

        nlohmann::json data;

        file_stream >> data;

        std::string len_str = data.value("length", "0");
        length = std::stoi(len_str);

        for (auto& field : data["fields"])
        {
            for (auto it = field.begin(), end = field.end(); it != end; it++)
            {
                std::string value = it.value();
                Field current_field(it.key(), value);
                fields.emplace_back(std::move(current_field));
            }
        }

        for (const auto& instr_i : data["instructions"])
        {
            Instructure data_elem(instr_i.value("format", std::string{}), instr_i.value("comment", std::string{}), instr_i);
            instructions.emplace_back(std::make_unique<Instructure>(std::move(data_elem)));
        }
    }

    size_t get_length () const {return length;}
    void   get_fields () const
    {
        for (auto& field : fields)
        {
            std::cout << field.operand << " : ";
            if (field.argument.is_min) {std::cout << ">=";}
            std::cout << field.argument.value << std::endl;
        }
    }

    void dump_instructions() const
    {
        size_t i = 0;
        for (const auto& elem : instructions)
        {
            if (!elem) {throw std::invalid_argument("Invalid ref on instructions!");}
            std::cout << "elem " << i << std::endl;
            elem->print_instructure();
            i++;
        }
    }
};
