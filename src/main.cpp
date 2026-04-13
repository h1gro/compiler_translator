#include <iostream>
#include <fstream>
#include <json.hpp>

#include "../include/uncoded_data.hpp"
#include "../include/encoded_data.hpp"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input.json> <output.json>\n";
        return 1;
    }

    try
    {
        const std::string input_path  = argv[1];
        const std::string output_path = argv[2];

        UncodedData data(input_path);
        EncodedData encoded_data = data.encode();

        std::ofstream os(output_path);
        encoded_data.dump_instructions(os);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
