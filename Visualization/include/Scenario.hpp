#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <cstdlib>
#include <map>
#include <algorithm>
#include "Cube.hpp"
#include "Move.hpp"
#include "ObjectCollection.hpp"
#include "Shader.hpp"
#include "MoveSequence.hpp"
#include "glm/glm.hpp"

class Scenario
{
public:
    Scenario(const char* filepath);
    ObjectCollection* toObjectCollection(Shader* shader, unsigned int vaoId, int texId);
    MoveSequence* toMoveSequence();
private:
    std::vector<Cube*> cubes;
    std::vector<Move*> moves;
};

#endif
