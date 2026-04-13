#include "../include/uncoded_data.hpp"
#include "../include/encoded_data.hpp"

EncodedData UncodedData::encode() const
{
    EncodedData encoded_data;

    for (int i = 0; i < instructions.size(); i++)
    {
        const auto& instr_group = instructions[i];

        for (const auto& insn : instr_group->insns)
        {
            EncodedData::Instruction current_instruction;
            current_instruction.insn = insn.second;

            EncodedData::Field F_field = init_field(current_instruction.last_bit, i);
            current_instruction.fields.emplace_back(F_field);

            if (instr_group->insns.size() > 1)
            {
                EncodedData::Field opcode_field = add_opcode_field(current_instruction.last_bit, insn.first);
                current_instruction.fields.emplace_back(opcode_field);
            }

            for (const auto& oper : instr_group->operands)
            {
                if (oper == "code")
                {
                    EncodedData::Field code_field = add_operand_field(oper, fields, current_instruction.last_bit);
                    current_instruction.fields.emplace_back(code_field);
                    break;
                }
            }

            for (const auto& oper : instr_group->operands)
            {
                if (oper == "code") {continue;}

                EncodedData::Field encoded_field = add_operand_field(oper, fields, current_instruction.last_bit);

                current_instruction.fields.emplace_back(encoded_field);
            }

            if (current_instruction.last_bit >= 0)
            {
                expand_first_unfixed_operand(current_instruction);
                if (current_instruction.last_bit >= 0)
                {
                    add_res_operand(current_instruction);
                }
            }

            encoded_data.instructions.emplace_back(current_instruction);
        }
    }

    return encoded_data;
}

EncodedData::Field add_operand_field (const std::string& oper, const auto& fields, int& last_bit)
{
    EncodedData::Field encoded_field;
    std::string output_name = oper;

    if (oper == "code") output_name = "CODE";
    if (oper == "imm")  output_name = "IMM";
    if (oper == "disp") output_name = "DISP";

    encoded_field.set_name(output_name);
    encoded_field.set_value();

    bool found_flag = false;
    for (const auto& upcoded_field : fields)
    {
        if (oper == upcoded_field.operand)
        {
            if (upcoded_field.argument.is_min) {encoded_field.is_min = true;}

            encoded_field.msb = last_bit;
            last_bit -= (upcoded_field.argument.value - 1);
            encoded_field.lsb = last_bit;
            last_bit--;

            found_flag = true;
            break;
        }
    }

    if (!found_flag) {throw std::invalid_argument("Unknown operand!");}

    return encoded_field;
}

EncodedData::Field init_field (int& last_bit, int instruction_number)
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

EncodedData::Field add_opcode_field (int& last_bit, int opcode)
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

void expand_first_unfixed_operand (EncodedData::Instruction& instruction)
{
    bool is_oper_expand = 0;
    for (int i = 0; i < instruction.fields.size(); i++)
    {
        if ((instruction.fields[i].is_min) && (!is_oper_expand))
        {
            std::cout << instruction.fields[i].name << std::endl;
            if (instruction.fields[i].name == "CODE")
            {
                instruction.fields[i].lsb--;
                for (int j = i; j < instruction.fields.size() - 1; j++)
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

void add_res_operand (EncodedData::Instruction& instruction)
{
    EncodedData::Field res_field;
    res_field.set_name("RES0");

    res_field.msb        = instruction.last_bit;
    res_field.lsb        = 0;
    instruction.last_bit = -1;

    res_field.set_value(std::string(res_field.msb - res_field.lsb + 1, '0'));

    instruction.fields.emplace_back(res_field);
}

std::string to_binary (int number, int width)
{
    std::string binary_format;
    binary_format.reserve(width);

    for (int i = width - 1; i >= 0; --i)
    {
        binary_format.push_back(((number >> i) & 1) ? '1' : '0');
    }

    return binary_format;
}
