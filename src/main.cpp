#include <iostream>
#include <fstream>
#include <json.hpp>

#include "../include/uncoded_data.hpp"
#include "../include/encoded_data.hpp"

int main()
{
    try
    {
        UncodedData data("../test/input.json");
        std::cout << data.get_length() << std::endl;

        // data.print_fields();
        EncodedData encoded_data = data.encode();

        std::ofstream os("../test/output.json");
        encoded_data.dump_instructions(os);
        // data.dump_coding_format("../test/output.json");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
