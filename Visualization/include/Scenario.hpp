#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "Cube.hpp"
#include "Animation.hpp"

class Scenario
{
public:
    Scenario(const char* filepath);
private:
    std::vector<Cube*> cubes;
    std::vector<Animation*> anims;
};

#endif
