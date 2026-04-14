#include "../include/encoded_data.hpp"

EncodedData::Field EncodedData::init_field (int& last_bit, size_t instruction_number)
{
    EncodedData::Field init_field;
    init_field.set_name();

    const int width = 2;
    init_field.set_value(to_binary(instruction_number, width));

    init_field.msb = last_bit; // = 24
    init_field.lsb = init_field.msb - 1;
    last_bit -= 2; // := 22

    return init_field;
}
                                
EncodedData::Field EncodedData::add_opcode_field (int& last_bit, size_t opcode)
{
    EncodedData::Field opcode_field;
    opcode_field.set_name("OPCODE");

    const int width = 4;

    opcode_field.msb = last_bit;
    opcode_field.lsb = opcode_field.msb - width + 1;
    last_bit         = opcode_field.lsb - 1;

    opcode_field.set_value(to_binary(opcode, width));
    return opcode_field;
}

void EncodedData::expand_first_unfixed_operand (EncodedData::Instruction& instruction)
{
    bool is_oper_expand = 0;
    for (size_t i = 0; i < instruction.fields.size(); i++)
    {
        if ((instruction.fields[i].is_min) && (!is_oper_expand))
        {
            // std::cout << instruction.fields[i].name << std::endl;
            if (instruction.fields[i].name == "CODE")
            {
                instruction.fields[i].lsb--;
                for (size_t j = i; j < instruction.fields.size() - 1; j++)
                {
                    recalc_bites(instruction.fields[j+1], instruction.fields[j]);
                }

                instruction.last_bit = instruction.fields.back().lsb - 1;
            }
            else
            {
                instruction.fields[i].lsb -= (instruction.last_bit + 1);
                is_oper_expand = true;
            }

            continue;
        }

        if ((is_oper_expand) && (instruction.fields[i].msb != instruction.fields[i-1].lsb - 1))
        {
            recalc_bites(instruction.fields[i], instruction.fields[i-1]);
        }
    }

    instruction.last_bit = instruction.fields.back().lsb - 1;
}

void recalc_bites(EncodedData::Field& current_field, EncodedData::Field& previous_field)
{
    int msb_lsb_dif   = current_field.msb - current_field.lsb;
    current_field.msb = previous_field.lsb - 1;
    current_field.lsb = current_field.msb - msb_lsb_dif;
}

void EncodedData::add_res_operands (EncodedData::Instruction& instruction)
{
    int current_position = instruction.last_bit;
    int res_index        = 0;

    for (size_t i = 0; i < instruction.fields.size(); i++)
    {
        if (current_position > instruction.fields[i].msb)
        {
            EncodedData::Field res_field;
            res_field.set_name("RES" + std::to_string(res_index));
            res_index++;

            res_field.msb = current_position;
            res_field.lsb = instruction.fields[i].msb + 1;

            res_field.set_value(std::string(static_cast<size_t>(res_field.msb - res_field.lsb + 1), '0'));
            instruction.fields.insert(instruction.fields.begin() + static_cast<long int>(i), std::move(res_field));
            i++;
        }

        current_position = instruction.fields[i].lsb - 1;
    }

    if (current_position >= 0)
    {
        EncodedData::Field res_field;
        res_field.set_name("RES" + std::to_string(res_index));
        res_field.msb = current_position;
        res_field.lsb = 0;
        res_field.set_value(std::string(static_cast<size_t>(res_field.msb - res_field.lsb + 1), '0'));

        instruction.fields.emplace_back(std::move(res_field));
    }
}
