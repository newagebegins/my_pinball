#include "File.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace File
{
    std::string readEntireFile(const char* path)
    {
        std::ifstream ifs{ path };
        if (!ifs.is_open())
        {
            std::cerr << "Failed to open the file: " << path << '\n';
            std::exit(EXIT_FAILURE);
        }
        std::stringstream s{};
        s << ifs.rdbuf();
        return s.str();
    }
}
