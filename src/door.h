#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Puzzle barrier door linked to a button ID.
 * Demonstrates state-dependent solidity (isSolid returns true when closed, false when open)
 * and animated barrier drawing.
 */
class Door : public Entity {
private:
    int m_id;
    bool m_isOpen;

public:
    Door(int id, const Vec3& center, const Vec3& size);
    virtual ~Door() override = default;

    int getId() const { return m_id; }
    bool isOpen() const { return m_isOpen; }
    void setOpen(bool open) { m_isOpen = open; }

    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "Door"; }
    virtual bool isSolid() const override { return !m_isOpen; }
};

} // namespace obby

using Door = obby::Door;
