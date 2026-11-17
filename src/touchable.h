#pragma once

namespace obby {

class Player;

/**
 * @brief Pure abstract interface for entities that react to player interaction.
 * Used alongside IDrawable to demonstrate multiple interface inheritance.
 */
class ITouchable {
public:
    virtual ~ITouchable() = default;
    virtual void onPlayerTouch(Player& player) = 0;
};

} // namespace obby

using ITouchable = obby::ITouchable;
