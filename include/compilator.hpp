#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <list>
#include <string>
#include <unordered_map>

#include <json.hpp>

enum opers
{
    R0   = 0x0,
    R1   = 0x1,
    R2   = 0x2,
    imm  = 0x3,
    disp = 0x4,
    code = 0x5
};

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

    struct Instracture
    {
        std::list<std::string> insns;
        std::list<opers> operands;
        std::string format;
        std::string comment;
    };

    std::vector<std::unique_ptr<Instracture>> instructions;

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
};
