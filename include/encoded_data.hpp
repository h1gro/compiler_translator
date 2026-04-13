#pragma once

#include <iostream>
#include "json.hpp"
#include "uncoded_data.hpp"

class EncodedData
{
public:
    int length;

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
        int last_bit;
    };

    std::vector<Instruction> instructions;

    void dump_instructions            (std::ofstream& os) const;
    void add_res_operands             (EncodedData::Instruction& instruction);
    void expand_first_unfixed_operand (EncodedData::Instruction& instruction);

    EncodedData::Field init_field       (int& last_bit, int instruction_number);
    EncodedData::Field add_opcode_field (int& last_bit, int opcode);
};

std::string to_binary (int number, int width);
void recalc_bites(EncodedData::Field& current_field, EncodedData::Field& previous_field);
