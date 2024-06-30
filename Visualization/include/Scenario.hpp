#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <cstdlib>
#include "Cube.hpp"
#include "Animation.hpp"
#include "ObjectCollection.hpp"
#include "Shader.hpp"
#include "AnimationSequence.hpp"

class Scenario
{
public:
    Scenario(const char* filepath);
    ObjectCollection* toObjectCollection(Shader* shader, unsigned int vaoId, int texId);
    AnimationSequence* toAnimationSequence();
private:
    std::vector<Cube*> cubes;
    std::vector<Animation*> anims;
};

#endif
