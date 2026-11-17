#pragma once
#include <stdexcept>
#include <string>

namespace obby {

/**
 * @brief Exception thrown when encountering corrupted or invalid level records.
 * Demonstrates standard library exception inheritance (std::runtime_error)
 * and structured exception handling in C++.
 */
class LevelParseError : public std::runtime_error {
public:
    explicit LevelParseError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace obby

using LevelParseError = obby::LevelParseError;
