#include "../include/uncoded_data.hpp"

//========================================================================================================
//Methods for printing data of UncodedData in the standart output stream
//========================================================================================================

void UncodedData::Instruction::print_instruction() const
{
    std::cout << "Format: " << format << "\nInsns:" << std::endl;
    print_insns();
    std::cout << "Operands:" << std::endl;
    print_operands();
    std::cout << "Comment: " << comment << "\n" << std::endl;
}

void UncodedData::Instruction::print_operands() const
{
    for (const auto& ins : insns)
    {
        std::cout << ins.first << " : " << ins.second << std::endl;
    }
}

void UncodedData::Instruction::print_insns() const
{
    for (const auto& ins : insns)
    {
        std::cout << ins.first << " : " << ins.second << std::endl;
    }
}

void UncodedData::print_fields() const
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

void UncodedData::print_instructions() const
{
    size_t i = 0;
    for (const auto& elem : instructions)
    {
        if (!elem)
        {
            throw std::invalid_argument("Invalid ref on instructions!");
        }
        std::cout << "elem " << i << std::endl;
        elem->print_instruction();
        i++;
    }
}
