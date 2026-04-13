#include "../include/compilator.hpp"

std::string CmdEncodingSystem::to_binary(int val, int width) const
{
    std::string binary_string;
    binary_string.reserve(width);

    for (int i = width - 1; i >= 0; --i)
    {
        binary_string.push_back(((val >> i) & 1) ? '1' : '0');
    }

    return binary_string;
}

bool CmdEncodingSystem::Instruction::uses_operand(const std::string& operand_name) const
{
    return (std::find(operands.begin(), operands.end(), operand_name) != operands.end());
}

std::string CmdEncodingSystem::to_upper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

int CmdEncodingSystem::bits_required(size_t count)
{
    if (count <= 1){return 0;}

    int bits      = 0;
    size_t values = 1;

    while (values < count)
    {
        values <<= 1U;
        bits++;
    }
    return bits;
}

bool CmdEncodingSystem::intersects(const LayoutField& lhs, const LayoutField& rhs)
{
    const int left = std::max(lhs.lsb, rhs.lsb);
    const int right = std::min(lhs.msb, rhs.msb);
    return left <= right;
}

bool CmdEncodingSystem::conflicts(const LayoutField& lhs, const LayoutField& rhs)
{
    const size_t limit = std::min(lhs.active_in_formats.size(), rhs.active_in_formats.size());
    for (size_t i = 0; i < limit; ++i)
    {
        if (lhs.active_in_formats[i] && rhs.active_in_formats[i])
        {
            return true;
        }
    }
    return false;
}

std::string CmdEncodingSystem::escape_json_string(const std::string& value)
{
    return nlohmann::json(value).dump();
}
