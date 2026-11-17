#pragma once
#include <string>

namespace obby {

/**
 * @brief Pure abstract interface for serializable game data components.
 * Demonstrates interface abstraction and polymorphic data persistence.
 */
class ISaveable {
public:
    virtual ~ISaveable() = default;
    virtual bool save(const std::string& path) const = 0;
    virtual bool load(const std::string& path) = 0;
};

} // namespace obby

using ISaveable = obby::ISaveable;
