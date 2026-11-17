#pragma once
#include "entity.h"
#include <memory>
#include <string>
#include <sstream>

namespace obby {

/**
 * @brief Factory class implementing the Factory Method pattern for level entity generation.
 * Decouples level file token parsing from concrete entity instantiation.
 */
class EntityFactory {
public:
    static std::unique_ptr<Entity> createEntity(const std::string& type, std::istringstream& ss, int worldId = 1);
};

} // namespace obby

using EntityFactory = obby::EntityFactory;
