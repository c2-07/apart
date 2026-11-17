#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Solid static block platform.
 * Demonstrates inheritance from Entity, encapsulation, and rendering
 * with top caps and deterministic position-hash color variation.
 */
class Platform : public Entity {
protected:
    int m_colorIndex;
    int m_worldId;

public:
    Platform(const Vec3& position, const Vec3& size, int colorIndex = 0, int worldId = 1);
    virtual ~Platform() override = default;

    int getColorIndex() const { return m_colorIndex; }
    void setColorIndex(int idx) { m_colorIndex = idx; }

    int getWorldId() const { return m_worldId; }
    void setWorldId(int wid) { m_worldId = wid; }

    // Polymorphic overrides
    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "Platform"; }
    virtual bool isSolid() const override { return true; }
};

} // namespace obby

using Platform = obby::Platform;
