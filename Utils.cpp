#include "utils.h"

namespace Utils {
    void seedRandom() {
        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            seeded = true;
        }
    }

    int randInt(int min, int max) {
        seedRandom();
        return min + (std::rand() % (max - min + 1));
    }

    float randFloat(float min, float max) {
        seedRandom();
        float scale = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        return min + scale * (max - min);
    }
}
