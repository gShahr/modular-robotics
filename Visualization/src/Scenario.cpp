#include "Scenario.hpp"

struct VisGroup {
    int color[3];
    int scale;

    VisGroup(int r, int g, int b, int scale) {
        this->color[0] = r;
        this->color[1] = g;
        this->color[2] = b;
        this->scale = scale;
    }
};

Scenario::Scenario(const char* filepath) {
    this->cubes = std::vector<Cube*>();
    this->moves = std::vector<Move*>();
    
    std::unordered_map<int, VisGroup*> visgroups;

    // -- Load file and parse data into Cube and Move objects --
    std::ifstream scenFile;
    std::stringstream stream;

    scenFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try {
        scenFile.open(filepath);
        stream << scenFile.rdbuf();
    } catch(std::ifstream::failure e) {
        std::cout << "ERROR::SCENARIO::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    int currentBlock = 0;       // Which block of the Scenario file we're currently parsing
    int buf[5];                 // Each line should have a maximum of 5 values. Any further will be ignored
    int i;                      // Helper iterator variable
    std::string line, value;    // Each line will be (temp) stored in string. Each component of a line will be (temp) stored in value
    glm::vec3 anchorDir;        // anchorDirs are encoded using ints; we will decode them when constructing each Move
    bool sliding;               // As above: sliding moves are encoded by sign of int. Extract to this helper variable
    VisGroup* vg;
    Cube* cube;

    while (std::getline(stream, line)) {
        line = line.substr(0, line.find("//"));
        if (line.empty()) {
            currentBlock++;
            std::cout << " -- Parsing new block -- " << currentBlock << std::endl;
            continue;
        }

        std::istringstream iss(line);
        for (i = 0; std::getline(iss, value, ','); i++) {
            buf[i] = std::stoi(value);
        }

        switch (currentBlock) {
            case 0: {
                for (i = 1; i < 4; i++) { buf[i] = glm::clamp(buf[i], 0, 255); }    // Clamp color values to 0-255
                buf[4] = glm::clamp(buf[4], 10, 100);                               // Clamp size value to 10-100
                std::cout << "Parsing Group ID " << buf[0] << " with color " << buf[1] << ", " << buf[2] << ", " << buf[3] << ", size " << buf[4] << std::endl;
                vg = new VisGroup(buf[1], buf[2], buf[3], buf[4]);
                visgroups.insert(std::pair<int, VisGroup*>(buf[0], vg));
                break; // Switch break
            }
            case 1: {
                std::cout << "Creating Cube with ID " << buf[0] << " in group " << buf[1] << " at location " << buf[2] << ", " << buf[3] << ", " << buf[4] << std::endl;
                cube = new Cube(buf[0], buf[2], buf[3], buf[4]);
                vg = visgroups.at(buf[1]);
                cube->setColor(vg->color[0], vg->color[1], vg->color[2]); 
                cube->setScale(vg->scale);
                this->cubes.push_back(cube);
                break;
            }
            case 2: {
                switch (abs(buf[1])) {
                    case (0): { anchorDir = glm::vec3(0.0f); break; }
                    case (1): { anchorDir = glm::vec3(1.0f, 0.0f, 0.0f); break; }
                    case (2): { anchorDir = glm::vec3(0.0f, 1.0f, 0.0f); break; }
                    case (3): { anchorDir = glm::vec3(0.0f, 0.0f, 1.0f); break; }
                    case (4): { anchorDir = glm::vec3(-1.0f, 0.0f, 0.0f); break; }
                    case (5): { anchorDir = glm::vec3(0.0f, -1.0f, 0.0f); break; }
                    case (6): { anchorDir = glm::vec3(0.0f, 0.0f, -1.0f); break; }
                    default:  { throw std::invalid_argument("Invalid anchor-direction specified in Scenario file (must be between -3 and 6)"); }
                }
                sliding = buf[1] > 0 ? false : true;

                this->moves.push_back(new Move(buf[0], anchorDir, glm::vec3(buf[2], buf[3], buf[4]), sliding));
                if (sliding) { std::cout << "Creating Sliding Move of Cube ID " << buf[0] << " with anchorDir " << glm::to_string(anchorDir) << " with delta position " << buf[2] << ", " << buf[3] << ", " << buf[4] << std::endl; }
                else { std::cout << "Creating Pivot Move of Cube ID " << buf[0] << " with anchorDir " << glm::to_string(anchorDir) << " with delta position " << buf[2] << ", " << buf[3] << ", " << buf[4] << std::endl; }
                break; // Switch break
            }
        }
    }
}

ObjectCollection* Scenario::toObjectCollection(Shader* pshader, unsigned int vaoId, int texId) {
    ObjectCollection* cubes = new ObjectCollection(pshader, vaoId, texId);

    for (Cube* cube : this->cubes) {
        cubes->addObj(cube);
    }

    return cubes;
}

MoveSequence* Scenario::toMoveSequence() {
    return new MoveSequence(this->moves);
}
