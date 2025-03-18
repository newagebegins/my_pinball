#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <iostream>
#include <string>
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

#endif
