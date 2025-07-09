/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Math utilities
*/

#include "zappy/graphics/utils/MathUtils.hpp"
#include <algorithm>

namespace MathUtils {

// Vector3f implementation
Vector3f::Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}

Vector3f Vector3f::operator+(const Vector3f& other) const
{
    return Vector3f(x + other.x, y + other.y, z + other.z);
}

Vector3f Vector3f::operator-(const Vector3f& other) const
{
    return Vector3f(x - other.x, y - other.y, z - other.z);
}

Vector3f Vector3f::operator*(float scalar) const
{
    return Vector3f(x * scalar, y * scalar, z * scalar);
}

float Vector3f::dot(const Vector3f& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3f Vector3f::cross(const Vector3f& other) const
{
    return Vector3f(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vector3f::length() const
{
    return std::sqrt(lengthSquared());
}

float Vector3f::lengthSquared() const
{
    return x * x + y * y + z * z;
}

Vector3f Vector3f::normalized() const
{
    float len = length();
    if (len < EPSILON) return Vector3f(0, 0, 0);
    return Vector3f(x / len, y / len, z / len);
}

void Vector3f::normalize()
{
    float len = length();
    if (len >= EPSILON) {
        x /= len;
        y /= len;
        z /= len;
    }
}

// Angle conversions
float degreesToRadians(float degrees)
{
    return degrees * DEG_TO_RAD;
}

float radiansToDegrees(float radians)
{
    return radians * RAD_TO_DEG;
}

// Basic math functions
float clamp(float value, float min, float max)
{
    return std::max(min, std::min(max, value));
}

float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

bool isEqual(float a, float b, float epsilon)
{
    return std::abs(a - b) < epsilon;
}

float smoothstep(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Distance and geometry
float distance(const Vector3f& a, const Vector3f& b)
{
    return (b - a).length();
}

float distanceSquared(const Vector3f& a, const Vector3f& b)
{
    return (b - a).lengthSquared();
}

Vector3f projectPointOnPlane(const Vector3f& point, const Vector3f& planeNormal, float planeDistance)
{
    float distanceToPlane = point.dot(planeNormal) - planeDistance;
    return point - planeNormal * distanceToPlane;
}

// Shadow calculations
Vector3f calculateShadowPosition(const Vector3f& objectPos, const Vector3f& lightPos, float shadowPlaneY)
{
    Vector3f lightToObject = objectPos - lightPos;
    
    // Prevent division by zero
    if (std::abs(lightToObject.y) < EPSILON) {
        return Vector3f(objectPos.x, shadowPlaneY, objectPos.z); // No valid shadow
    }
    
    float shadowScale = (shadowPlaneY - lightPos.y) / lightToObject.y;
    Vector3f shadowPos = lightPos + lightToObject * shadowScale;
    
    return Vector3f(shadowPos.x, shadowPlaneY, shadowPos.z);
}

bool isPointInBounds(const Vector3f& point, const Vector3f& minBounds, const Vector3f& maxBounds)
{
    return (point.x >= minBounds.x && point.x <= maxBounds.x &&
            point.y >= minBounds.y && point.y <= maxBounds.y &&
            point.z >= minBounds.z && point.z <= maxBounds.z);
}

// Camera and view calculations
float calculateFOVFromZoom(float zoom, float baseFOV)
{
    return baseFOV / zoom;
}

Vector3f calculateOrbitPosition(float angle, float distance, float height, const Vector3f& center)
{
    float x = center.x + std::cos(angle) * distance;
    float z = center.z + std::sin(angle) * distance;
    return Vector3f(x, center.y + height, z);
}

// Color utilities
sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t)
{
    t = clamp(t, 0.0f, 1.0f);
    return sf::Color(
        static_cast<sf::Uint8>(lerp(a.r, b.r, t)),
        static_cast<sf::Uint8>(lerp(a.g, b.g, t)),
        static_cast<sf::Uint8>(lerp(a.b, b.b, t)),
        static_cast<sf::Uint8>(lerp(a.a, b.a, t))
    );
}

sf::Color multiplyColor(const sf::Color& color, float factor)
{
    return sf::Color(
        static_cast<sf::Uint8>(clamp(color.r * factor, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(clamp(color.g * factor, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(clamp(color.b * factor, 0.0f, 255.0f)),
        color.a
    );
}

sf::Color addColors(const sf::Color& a, const sf::Color& b)
{
    return sf::Color(
        static_cast<sf::Uint8>(clamp(a.r + b.r, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(clamp(a.g + b.g, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(clamp(a.b + b.b, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(clamp(a.a + b.a, 0.0f, 255.0f))
    );
}

} // namespace MathUtils