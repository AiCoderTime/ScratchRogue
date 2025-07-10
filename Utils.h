#pragma once
#include <cstdlib>
#include <ctime>

namespace Utils {
    void seedRandom();
    int randInt(int min, int max);
    float randFloat(float min, float max);
}
