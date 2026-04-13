#pragma once

#include <iostream>
#include "json.hpp"
#include "uncoded_data.hpp"

class EncodedData
{
public:
    class Field
    {
    public:
        std::string name;
        std::string value;
        int msb;
        int lsb;
        bool is_min = false;

        void set_name(std::string operand = "F") {name = operand;}
        void set_value(std::string value_ = "+"){value = value_;}
    };

    class Instruction
    {
    public:
        std::string insn;
        std::vector<Field> fields;
        int last_bit = 24;
    };

    std::vector<Instruction> instructions;

    void dump_instructions(std::ofstream& os) const
    {
        os << "[" << std::endl;
        const int key_width = 6;
        for (int j = 0; j < instructions.size(); j++)
        {
            const auto& insn = instructions[j];
            os << "{" << std::endl;
            os << "    \"insn\" : " << "\"" << insn.insn << "\"," << std::endl;
            os << "    \"fields\" : \n" << "        [" << std::endl;
            for (int i = 0; i < insn.fields.size(); i++)
            {
                int pad = key_width - static_cast<int>(insn.fields[i].name.size());
                if (pad < 0) { pad = 0; }

                os << "            {\""
                   << insn.fields[i].name
                   << "\""
                   << std::string(pad, ' ')
                   << " : {\"msb\" : "
                   << std::right << std::setw(2) << insn.fields[i].msb
                   << ", \"lsb\" : "
                   << std::right << std::setw(2) << insn.fields[i].lsb
                   << ", \"value\" : \""
                   << insn.fields[i].value
                   << "\"}}";

                if (i + 1 < insn.fields.size()) {os << ",";}
                os << "\n";
            }
            os << "        ]" << std::endl;
            os << "}";
            if (j + 1 < instructions.size()) {os << ",";}
            os << std::endl;
        }
        os << "]" << std::endl;
    }
};

EncodedData::Field init_field (int& last_bit, int instruction_number);
void expand_first_unfixed_operand (EncodedData::Instruction& instruction);
void add_res_operand (EncodedData::Instruction& instruction);
std::string to_binary (int number, int width);
EncodedData::Field add_opcode_field (int& last_bit, int opcode);
EncodedData::Field add_operand_field (const std::string& oper, const auto& fields, int& last_bit);
void recalc_bites(EncodedData::Field& current_field, EncodedData::Field& previous_field);
