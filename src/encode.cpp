#include "../include/uncoded_data.hpp"
#include "../include/encoded_data.hpp"

// this struct contains canonical (similar operands must be in the same bit positions)
// information about operands in current format
struct CanonicalField
{
    std::string name;
    int  width;
    bool is_min;
    int  msb;
    int  lsb;

    // msb, lsb = -1 -- unused field
    CanonicalField() : name(), width(0), is_min(false), msb(-1), lsb(-1) {}
};

class CanonicalBitesPos
{
public:
    std::unordered_map<std::string, CanonicalField> canonical_positions;
    CanonicalBitesPos() : canonical_positions() {}

    void completion(const auto& fields)
    {
        for (const auto& src_field : fields)
        {
            CanonicalField field;
            field.name   = src_field.operand;
            field.width  = src_field.argument.value;
            field.is_min = src_field.argument.is_min;

            canonical_positions.emplace(field.name, std::move(field));
        }
    }
};

EncodedData::Field add_operand_field (const std::string& oper, CanonicalBitesPos& canon, int& last_bit);

//=======================================================================================================
// the key function of the project, that performs encoding
EncodedData UncodedData::encode() const
{
    //create instances of classes
    EncodedData       encoded_data;
    CanonicalBitesPos canon_instr_map;

    encoded_data.length = length; //from UncodedData

    //completing map of canonical fields information from input fields
    canon_instr_map.completion(fields);

    for (size_t i = 0; i < instructions.size(); i++)
    {
        const auto& instr_group = instructions[i];

        for (const auto& insn : instr_group->insns)
        {
            EncodedData::Instruction current_instruction;
            current_instruction.last_bit = static_cast<int>(length) - 1;
            current_instruction.insn     = insn.second;

            EncodedData::Field F_field = encoded_data.init_field(current_instruction.last_bit, i);
            current_instruction.fields.emplace_back(F_field);

            //-----------------CODE/OPCODE--------------------------------
            //in current tests json files this condition don't progress in case: "insn" : "branch.cond"
            if (instr_group->insns.size() > 1)
            {
                EncodedData::Field opcode_field = encoded_data.add_opcode_field(current_instruction.last_bit, insn.first);
                current_instruction.fields.emplace_back(opcode_field);
            }

            //check if instructure use operand "code"
            for (const auto& oper : instr_group->operands)
            {
                // as same as a previous: for "branch.cond"
                if (oper == "code")
                {
                    EncodedData::Field code_field = add_operand_field(oper, canon_instr_map, current_instruction.last_bit);
                    current_instruction.fields.emplace_back(code_field);
                    break;
                }
            }
            //-------------------------------------------------

            for (const auto& oper : instr_group->operands)
            {
                if (oper == "code") {continue;}

                EncodedData::Field encoded_field = add_operand_field(oper, canon_instr_map, current_instruction.last_bit);

                current_instruction.fields.emplace_back(encoded_field);
            }

            if (current_instruction.last_bit < 0) {throw std::invalid_argument("Length < sum of operands sizes in current instruction");}

            if (current_instruction.last_bit >= 0)
            {
                encoded_data.expand_first_unfixed_operand(current_instruction);
            }

            encoded_data.add_res_operands(current_instruction);
            encoded_data.instructions.emplace_back(current_instruction);
        }
    }

    return encoded_data;
}
//=======================================================================================================


EncodedData::Field add_operand_field (const std::string& oper, CanonicalBitesPos& canon, int& last_bit)
{
    auto it = canon.canonical_positions.find(oper);
    if (it == canon.canonical_positions.end()){throw std::invalid_argument("Unknown operand!");}
    auto& canonical = it->second;

    EncodedData::Field encoded_field;
    std::string output_name = oper;

    if (oper == "code") output_name = "CODE";
    if (oper == "imm" ) output_name = "IMM";
    if (oper == "disp") output_name = "DISP";

    encoded_field.set_name(output_name);
    encoded_field.set_value();
    encoded_field.is_min = canonical.is_min;

    if (canonical.msb == -1)
    {
        encoded_field.msb = last_bit;
        last_bit         -= (canonical.width - 1);
        encoded_field.lsb = last_bit;
        last_bit--;

        canonical.msb = encoded_field.msb;
        canonical.lsb = encoded_field.lsb;
    }
    else
    {
        encoded_field.msb = canonical.msb;
        encoded_field.lsb = canonical.lsb;
        last_bit = encoded_field.lsb - 1;
    }

    return encoded_field;
}

std::string to_binary (size_t number, int width)
{
    std::string binary_format;
    binary_format.reserve(static_cast<size_t>(width));

    for (int i = width - 1; i >= 0; --i)
    {
        binary_format.push_back(((number >> i) & 1) ? '1' : '0');
    }

    return binary_format;
}
