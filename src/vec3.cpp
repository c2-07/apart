#include "vec3.h"
#include <algorithm>

namespace obby {

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3{ x + other.x, y + other.y, z + other.z };
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3{ x - other.x, y - other.y, z - other.z };
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3{ x * scalar, y * scalar, z * scalar };
}

Vec3 Vec3::operator/(float scalar) const {
    float inv = (std::abs(scalar) > 1e-8f) ? (1.0f / scalar) : 0.0f;
    return Vec3{ x * inv, y * inv, z * inv };
}

Vec3 Vec3::operator-() const {
    return Vec3{ -x, -y, -z };
}

Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vec3& Vec3::operator/=(float scalar) {
    float inv = (std::abs(scalar) > 1e-8f) ? (1.0f / scalar) : 0.0f;
    x *= inv;
    y *= inv;
    z *= inv;
    return *this;
}

bool Vec3::operator==(const Vec3& other) const {
    constexpr float eps = 1e-5f;
    return (std::abs(x - other.x) <= eps) &&
           (std::abs(y - other.y) <= eps) &&
           (std::abs(z - other.z) <= eps);
}

bool Vec3::operator!=(const Vec3& other) const {
    return !(*this == other);
}

Vec3 operator*(float scalar, const Vec3& v) {
    return Vec3{ v.x * scalar, v.y * scalar, v.z * scalar };
}

float Vec3::length() const {
    return std::sqrt(lengthSq());
}

float Vec3::lengthSq() const {
    return x * x + y * y + z * z;
}

Vec3 Vec3::normalized() const {
    float len = length();
    if (len > 1e-6f) {
        float inv = 1.0f / len;
        return Vec3{ x * inv, y * inv, z * inv };
    }
    return Vec3{ 0.0f, 0.0f, 0.0f };
}

float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3{
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

} // namespace obby
