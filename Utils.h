#pragma once
#include <cstdlib>
#include <ctime>

// Utility functions for random number generation
namespace Utils {
    // Seeds the random number generator once per program run
    void seedRandom();

    // Returns a random integer in [min, max]
    int randInt(int min, int max);

    // Returns a random float in [min, max)
    float randFloat(float min, float max);
}
