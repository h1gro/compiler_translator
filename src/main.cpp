#include <iostream>
#include <fstream>
#include <json.hpp>

#include "../include/compilator.hpp"

int main()
{
    try
    {
        CmdEncodingSystem data("../test/input.json");
        std::cout << data.get_length() << std::endl;

        data.get_fields();
        data.dump_coding_format("../test/output.json");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
