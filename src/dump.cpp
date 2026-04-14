#include "../include/uncoded_data.hpp"
#include "../include/encoded_data.hpp"
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

//========================================================================================================
//Methods for printing data of EncodedData in the standart output stream
//========================================================================================================

void EncodedData::dump_instructions(std::ofstream& os) const
{
    os << "[" << std::endl;
    const int key_width = 6;
    for (size_t j = 0; j < instructions.size(); j++)
    {
        const auto& insn = instructions[j];
        os << "{" << std::endl;
        os << "    \"insn\" : " << "\"" << insn.insn << "\"," << std::endl;
        os << "    \"fields\" : \n" << "        [" << std::endl;
        for (size_t i = 0; i < insn.fields.size(); i++)
        {
            int pad = key_width - static_cast<int>(insn.fields[i].name.size());
            if (pad < 0) { pad = 0; }

            os << "            {\""
                << insn.fields[i].name
                << "\""
                << std::string(static_cast<size_t>(pad), ' ')
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
