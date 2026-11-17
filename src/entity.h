#pragma once
#include "drawable.h"
#include "touchable.h"
#include "vec3.h"
#include "aabb.h"
#include "raylib.h"

struct Player;

namespace obby {

/**
 * @brief Abstract base class for all interactive game entities.
 * Demonstrates:
 * - Multiple interface inheritance (IDrawable, ITouchable)
 * - Pure virtual functions (update, draw, getTypeName)
 * - Virtual functions with default implementations (onPlayerTouch, getBounds, isSolid)
 * - Encapsulation (protected data members, public getters/setters)
 * - Static member variables and functions (tracking active entity count)
 * - Virtual destructor for proper polymorphic cleanup
 */
class Entity : public IDrawable, public ITouchable {
protected:
    Vec3 m_position;
    Vec3 m_size;
    Color m_color;

    // Static member variable counting live entities
    static int s_entityCount;

public:
    // Constructors using member initializer list
    Entity(const Vec3& position, const Vec3& size, Color color = WHITE);
    virtual ~Entity() override;

    // Static getter for entity count
    static int getEntityCount() { return s_entityCount; }

    // Encapsulated Getters & Setters
    const Vec3& getPosition() const { return m_position; }
    void setPosition(const Vec3& pos) { m_position = pos; }

    const Vec3& getSize() const { return m_size; }
    void setSize(const Vec3& size) { m_size = size; }

    Color getColor() const { return m_color; }
    void setColor(Color col) { m_color = col; }

    // Pure virtual methods
    virtual void update(float dt, Player& player) = 0;
    virtual void draw() const override = 0;
    virtual const char* getTypeName() const = 0;

    // Virtual methods with default implementations
    virtual void onPlayerTouch(Player& player) override;
    virtual AABB getBounds() const;
    virtual bool isSolid() const;
};

} // namespace obby

using Entity = obby::Entity;
