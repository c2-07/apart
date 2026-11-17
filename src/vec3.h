#pragma once
#include "raylib.h"
#include <cmath>

namespace obby {

/**
 * @brief 3D vector value class with comprehensive operator overloading.
 * Demonstrates operator overloading (+, -, *, +=, -=, ==, !=, unary -),
 * constructors with member initializer lists, and raylib interoperability.
 */
class Vec3 {
public:
    float x;
    float y;
    float z;

    // Constructors using member initializer lists
    constexpr Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr Vec3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
    Vec3(const Vector3& r) : x(r.x), y(r.y), z(r.z) {}

    // Encapsulated / Functional Accessors
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    void setX(float inX) { x = inX; }
    void setY(float inY) { y = inY; }
    void setZ(float inZ) { z = inZ; }

    // Interoperability with raylib Vector3
    Vector3 toRaylib() const { return Vector3{ x, y, z }; }
    operator Vector3() const { return Vector3{ x, y, z }; }

    // Operator Overloading: Arithmetic
    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;
    Vec3 operator-() const;

    // Operator Overloading: Compound Assignment
    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);
    Vec3& operator*=(float scalar);
    Vec3& operator/=(float scalar);

    // Operator Overloading: Comparison
    bool operator==(const Vec3& other) const;
    bool operator!=(const Vec3& other) const;

    // Friend function for scalar * Vec3 syntax
    friend Vec3 operator*(float scalar, const Vec3& v);

    // Vector operations
    float length() const;
    float lengthSq() const;
    Vec3 normalized() const;
    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;
};

} // namespace obby

using Vec3 = obby::Vec3;
