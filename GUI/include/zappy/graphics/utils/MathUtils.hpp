/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Math utilities for 3D calculations
*/

#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include <SFML/Graphics.hpp>
#include <cmath>

namespace MathUtils {
    
    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float EPSILON = 1e-6f;
    
    struct Vector3f {
        float x, y, z;
        
        Vector3f(float x = 0.0f, float y = 0.0f, float z = 0.0f);
        Vector3f operator+(const Vector3f& other) const;
        Vector3f operator-(const Vector3f& other) const;
        Vector3f operator*(float scalar) const;
        float dot(const Vector3f& other) const;
        Vector3f cross(const Vector3f& other) const;
        float length() const;
        float lengthSquared() const;
        Vector3f normalized() const;
        void normalize();
    };
    
    float degreesToRadians(float degrees);
    float radiansToDegrees(float radians);
    
    float clamp(float value, float min, float max);
    float lerp(float a, float b, float t);
    bool isEqual(float a, float b, float epsilon = EPSILON);
    float smoothstep(float edge0, float edge1, float x);
    
    float distance(const Vector3f& a, const Vector3f& b);
    float distanceSquared(const Vector3f& a, const Vector3f& b);
    Vector3f projectPointOnPlane(const Vector3f& point, const Vector3f& planeNormal, float planeDistance);
    
    Vector3f calculateShadowPosition(const Vector3f& objectPos, const Vector3f& lightPos, float shadowPlaneY);
    bool isPointInBounds(const Vector3f& point, const Vector3f& minBounds, const Vector3f& maxBounds);
    
    float calculateFOVFromZoom(float zoom, float baseFOV = 45.0f);
    Vector3f calculateOrbitPosition(float angle, float distance, float height, const Vector3f& center);
    
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t);
    sf::Color multiplyColor(const sf::Color& color, float factor);
    sf::Color addColors(const sf::Color& a, const sf::Color& b);
}

#endif /* !MATHUTILS_HPP */