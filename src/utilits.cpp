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
