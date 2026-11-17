#pragma once
#include <string>
#include <vector>

struct ValidationResult {
    std::string filename;
    std::string levelName;
    int worldId;
    int boxCount;
    int moverCount;
    int hazardCount;
    float maxObservedGap;
    float maxAllowedGap;
    bool hasFinish;
    bool passed;
};

class LevelValidator {
public:
    static ValidationResult ValidateFile(const std::string& filepath);
    static bool ValidateAllLevels(const std::vector<std::string>& files);
};
