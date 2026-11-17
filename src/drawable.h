#pragma once

namespace obby {

/**
 * @brief Pure abstract interface for all renderable game objects.
 * Demonstrates interface abstraction and virtual destructors.
 */
class IDrawable {
public:
    virtual ~IDrawable() = default;
    virtual void draw() const = 0;
};

} // namespace obby

using IDrawable = obby::IDrawable;
