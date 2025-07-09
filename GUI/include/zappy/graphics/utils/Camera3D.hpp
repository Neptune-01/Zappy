/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** 3D Camera for orbital rotation and FPS mode
*/

#ifndef CAMERA3D_HPP
#define CAMERA3D_HPP

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "zappy/graphics/utils/MathUtils.hpp"
#include "zappy/game/entities/Player.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zappy::graphics::utils {
class Camera3D {
public:
    enum class CameraMode {
        ORBITAL,
        FPS
    };

public:
    Camera3D();
    ~Camera3D() = default;

    // === ORBITAL MODE ===
    void setRotation(float angle) { _rotation_angle = angle; }
    void addRotation(float delta) { _rotation_angle += delta; }
    
    // Méthodes pour gérer la taille de la carte
    void setMapSize(int width, int height) {
        _map_width = width;
        _map_height = height;
        updateCameraForMapSize(); // Auto-ajustement
    }
    
    float getRotation() const { return _rotation_angle; }
    int getCurrentMapWidth() const { return _map_width; }
    int getCurrentMapHeight() const { return _map_height; }

    void setZoom(float zoom) { _zoom = zoom; }
    float getZoom() const { return _zoom; }

    void setCameraOffset(const sf::Vector2f& offset) { _camera_offset = offset; }
    sf::Vector2f getCameraOffset() const { return _camera_offset; }
    
    // === FPS MODE ===
    void setMode(CameraMode mode) { _camera_mode = mode; }
    CameraMode getMode() const { return _camera_mode; }
    bool isFPSMode() const { return _camera_mode == CameraMode::FPS; }
    
    // FPS position and orientation
    void setFPSPosition(const MathUtils::Vector3f& position) { _fps_position = position; }
    MathUtils::Vector3f getFPSPosition() const { return _fps_position; }
    
    void setFPSTarget(int player_id) { _fps_target_player_id = player_id; }
    int getFPSTarget() const { return _fps_target_player_id; }
    
    // FPS camera rotation (yaw/pitch)
    void setFPSYaw(float yaw) { _fps_yaw = yaw; }
    void setFPSPitch(float pitch) { 
        _fps_pitch = std::max(-89.0f, std::min(89.0f, pitch)); // Clamp pitch
    }
    
    float getFPSYaw() const { return _fps_yaw; }
    float getFPSPitch() const { return _fps_pitch; }
    
    // FPS movement
    void setFPSFromPlayer(const zappy::game::entities::ZappyPlayer& player);

    // === SHARED METHODS ===
    void setupProjection(int window_width, int window_height);
    void setupCamera();
    void resetMatrices();
    
    // Contrôle du FOV
    void setFOV(float fov) { _base_fov = fov; }
    float getFOV() const { return _base_fov; }
    
    // Limites de zoom adaptatif
    float getMinZoom() const { return _min_zoom; }
    float getMaxZoom() const { return _max_zoom; }
    
    // Auto-ajustement en fonction de la taille de carte
    void updateCameraForMapSize();

    // Constantes pour la compatibilité (si d'autres parties du code les utilisent)
    static constexpr float TILE_SIZE = 1.0f;
    static constexpr float TILE_HEIGHT = 0.5f;

private:
    void setupOrbitalCamera();
    void setupFPSCamera();
    MathUtils::Vector3f calculateFPSLookDirection() const;
    
    // === COMMON ===
    CameraMode _camera_mode = CameraMode::ORBITAL;
    
    // === ORBITAL MODE ===
    float _rotation_angle = 0.0f;
    float _zoom = 1.0f;
    sf::Vector2f _camera_offset;
    
    float _camera_distance = 15.0f;
    float _camera_height = 10.0f;
    
    // === FPS MODE ===
    MathUtils::Vector3f _fps_position{0.0f, 2.0f, 0.0f}; // Eye position
    float _fps_yaw = 0.0f;        // Horizontal rotation (degrees)
    float _fps_pitch = 0.0f;      // Vertical rotation (degrees)
    int _fps_target_player_id = -1; // ID du joueur en vue FPS
    
    // === PROJECTION ===
    float _base_fov = 45.0f;        // FOV de base
    float _min_fov = 5.0f;          // FOV minimum réduit (était 15°)
    float _max_fov = 75.0f;         // FOV maximum (zoom min)
    
    // Limites de zoom adaptatif (orbital uniquement)
    float _min_zoom = 0.1f;         // Zoom minimum (sera ajusté)
    float _max_zoom = 10.0f;        // Zoom maximum (sera ajusté)
    
    // Variables modifiables pour la taille de la carte
    int _map_width = 10;   // Valeur par défaut
    int _map_height = 10;  // Valeur par défaut
};
}

#endif /* !CAMERA3D_HPP */