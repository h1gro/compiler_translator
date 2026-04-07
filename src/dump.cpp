#include "../include/compilator.hpp"

//========================================================================================================
//Methods for printing data of CmdEncodingSystem in the standart output stream
//========================================================================================================

void CmdEncodingSystem::Instruction::print_instruction() const
{
    std::cout << "Format: " << format << "\nInsns:" << std::endl;
    print_insns();
    std::cout << "Operands:" << std::endl;
    print_operands();
    std::cout << "Comment: " << comment << "\n" << std::endl;
}

void CmdEncodingSystem::Instruction::print_operands() const
{
    for (const auto& ins : insns)
    {
        std::cout << ins.first << " : " << ins.second << std::endl;
    }
}

void CmdEncodingSystem::Instruction::print_insns() const
{
    for (const auto& ins : insns)
    {
        std::cout << ins.first << " : " << ins.second << std::endl;
    }
}

//========================================================================================================
//Methods for writing encoding data in the output file
//========================================================================================================

void CmdEncodingSystem::write_indent(std::ostream& output_stream, int spaces) const
{
    for (int i = 0; i < spaces; ++i)
    {
        output_stream.put(' ');
    }
}

void CmdEncodingSystem::write_inline_field(std::ostream& output_stream, const nlohmann::json& field_json) const
{
    const auto it = field_json.begin();
    const std::string& name = it.key();
    const auto& inner = it.value();

    output_stream  << "{"
        << std::left << std::setw(8) << escape_json_string(name)
        << " : {"
        << escape_json_string("msb") << " : " << std::right << std::setw(2) << inner.at("msb").get<int>()
        << ", "
        << escape_json_string("lsb") << " : " << std::right << std::setw(2) << inner.at("lsb").get<int>()
        << ", "
        << escape_json_string("value") << " : " << escape_json_string(inner.at("value").get<std::string>())
        << "}}";
}

void CmdEncodingSystem::write_fields_array(std::ostream& output_stream, const nlohmann::json& fields_json, int indent) const
{
    output_stream << "[\n";
    for (size_t i = 0; i < fields_json.size(); ++i)
    {
        write_indent(output_stream, indent + 4);
        write_inline_field(output_stream, fields_json[i]);
        if (i + 1 < fields_json.size())
        {
            output_stream << ",";
        }
        output_stream << "\n";
    }
    write_indent(output_stream, indent);
    output_stream << "]";
}

void CmdEncodingSystem::dump_instructions() const
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
        ++i;
    }
}

void CmdEncodingSystem::dump_coding_format(std::string file_path) const
{
    std::ofstream file_stream(file_path);

    if (!file_stream){throw std::invalid_argument("Error in opening file!");}

    const auto global_layout = build_global_layout();

    file_stream << "[\n";

    bool first_instruction = true;

    for (size_t format_index = 0; format_index < instructions.size(); ++format_index)
    {
        const auto& elem = instructions[format_index];

        if (!elem){throw std::invalid_argument("Invalid ref in instructions!");}

        for (const auto& instr : elem->insns)
        {
            if (!first_instruction){file_stream << ",\n";}
            first_instruction = false;

            const nlohmann::json fields_json = output_fields(format_index, instr.first, global_layout);

            file_stream << "    {\n" << "        \"insn\" : "
                        << nlohmann::json(instr.second).dump() << ",\n"
                        << "        \"fields\" :\n";

            write_indent(file_stream, 8);
            write_fields_array(file_stream, fields_json, 8);

            file_stream << "\n" << "    }";
        }
    }

    file_stream << "\n]\n";
}
