#include "Scenario.hpp"

Scenario::Scenario(const char* filepath) {
    this->cubes = std::vector<Cube*>();
    this->anims = std::vector<Animation*>();

    // -- Load file and parse data into Cube and Animation objects --
    std::string raw;
    std::ifstream scenFile;
    std::stringstream stream;

    scenFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        scenFile.open(filepath);
        stream << scenFile.rdbuf();
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SCENARIO::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    bool parsedInitials = false;
    std::string line, value;
    int i;
    int buf[5]; // maximum of 5 values per line expected
    while (std::getline(stream, line)) {
        if (line.empty()) {
            parsedInitials = true;
            continue;
        }

        std::istringstream iss(line);
        for (i = 0; std::getline(iss, value, ','); i++) {
            buf[i] = std::stoi(value);
        }

        if (!parsedInitials) {
            std::cout << "Creating Cube with ID " << buf[0] << " at location " << buf[1] << ", " << buf[2] << ", " << buf[3] << std::endl;
            cubes.push_back(new Cube(buf[0], buf[1], buf[2], buf[3]));
        } else {
            std::cout << "Creating Animation of Cube ID " << buf[0] << " anchored to Cube " << buf[1] << " with delta position " << buf[2] << ", " << buf[3] << ", " << buf[4] << std::endl;

        }
    }
}
