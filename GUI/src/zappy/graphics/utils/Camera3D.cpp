/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** 3D Camera implementation with orbital rotation and FPS mode
*/

#include "zappy/graphics/utils/Camera3D.hpp"
#include <GL/glu.h>

zappy::graphics::utils::Camera3D::Camera3D()
{
    updateCameraForMapSize(); // Initialiser avec les valeurs par défaut
}

void zappy::graphics::utils::Camera3D::setupProjection(int window_width, int window_height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect_ratio = static_cast<float>(window_width) / static_cast<float>(window_height);
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    
    if (_camera_mode == CameraMode::ORBITAL) {
        // Mode orbital : FOV adaptatif basé sur la taille de la carte
        float effective_fov = _base_fov;
        float map_size_factor = std::max(_map_width, _map_height);
        
        // Ajuster le FOV pour les grandes cartes (FOV plus petit = moins de distorsion)
        if (map_size_factor > 20) {
            effective_fov = _base_fov * (20.0f / map_size_factor) * 0.8f; // Réduction progressive
        }
        
        // Limiter le FOV dans une plage raisonnable MAIS plus large pour le zoom
        float effective_min_fov = std::max(1.0f, _min_fov / map_size_factor * 10.0f); // FOV min adaptatif
        effective_fov = std::max(effective_min_fov, std::min(_max_fov, effective_fov));
        
        // Appliquer le zoom au FOV effectif
        effective_fov = effective_fov / _zoom;
        
        // CORRECTION: FOV minimum vraiment plus petit pour permettre plus de zoom
        effective_fov = std::max(effective_min_fov, std::min(_max_fov, effective_fov));
        
        // Setup perspective projection avec glFrustum pour orbital
        float fov_rad = effective_fov * M_PI / 180.0f;
        float f = 1.0f / tan(fov_rad / 2.0f);
        
        glFrustum(-aspect_ratio/f, aspect_ratio/f, -1.0f/f, 1.0f/f, near_plane, far_plane);
    } else {
        // Mode FPS : FOV fixe et optimisé pour éviter complètement le fisheye
        float fps_fov = 65.0f; // FOV plus conservateur pour éliminer le fisheye
        
        // Utiliser gluPerspective pour une projection FPS standard
        gluPerspective(fps_fov, aspect_ratio, near_plane, far_plane);
    }
    
    glMatrixMode(GL_MODELVIEW);
}

void zappy::graphics::utils::Camera3D::setupCamera()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (_camera_mode == CameraMode::ORBITAL) {
        setupOrbitalCamera();
    } else {
        setupFPSCamera();
    }
}

void zappy::graphics::utils::Camera3D::setupOrbitalCamera()
{
    // Calculer le centre de la carte en utilisant les variables d'instance
    float map_center_x = (_map_width - 1) / 2.0f;
    float map_center_z = (_map_height - 1) / 2.0f;
    
    // Appliquer l'offset de la caméra au centre
    float center_x = map_center_x + _camera_offset.x * 0.1f;
    float center_z = map_center_z + _camera_offset.y * 0.1f;
    
    // Position de la caméra dans l'espace 3D (orbite autour du centre)
    float camera_x = center_x + cos(_rotation_angle) * _camera_distance;
    float camera_z = center_z + sin(_rotation_angle) * _camera_distance;
    float camera_y = _camera_height;
    
    // Regarder vers le centre de la carte
    gluLookAt(camera_x, camera_y, camera_z,  // Position de la caméra
              center_x, 1.0f, center_z,       // Point regardé (centre de la carte, niveau des tuiles)
              0.0f, 1.0f, 0.0f);              // Vecteur "up"
}

void zappy::graphics::utils::Camera3D::setupFPSCamera()
{
    // Calculer la direction de vue directement depuis les angles yaw/pitch
    MathUtils::Vector3f look_direction = calculateFPSLookDirection();
    MathUtils::Vector3f target = _fps_position + look_direction;
    
    // Configuration de la caméra FPS
    gluLookAt(
        _fps_position.x, _fps_position.y, _fps_position.z,    // Position de l'œil
        target.x, target.y, target.z,                         // Point regardé
        0.0f, 1.0f, 0.0f                                      // Vecteur "up"
    );
}

MathUtils::Vector3f zappy::graphics::utils::Camera3D::calculateFPSLookDirection() const
{
    // Convertir yaw en radians (pitch reste fixe à -5° pour regarder légèrement vers le bas)
    float yaw_rad = _fps_yaw * M_PI / 180.0f;
    float pitch_rad = _fps_pitch * M_PI / 180.0f;
    
    // Calculer la direction de vue (système OpenGL standard)
    MathUtils::Vector3f direction;
    direction.x = sin(yaw_rad) * cos(pitch_rad);
    direction.y = sin(pitch_rad);
    direction.z = -cos(yaw_rad) * cos(pitch_rad); // Z négatif = vers l'avant
    
    return direction.normalized();
}

void zappy::graphics::utils::Camera3D::resetMatrices()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void zappy::graphics::utils::Camera3D::updateCameraForMapSize()
{
    // Ajuster la distance de la caméra en fonction de la taille de la carte
    float map_diagonal = sqrt(_map_width * _map_width + _map_height * _map_height);
    
    // Distance adaptée à la taille (formule empirique)
    _camera_distance = std::max(15.0f, map_diagonal * 0.8f);
    _camera_height = std::max(10.0f, map_diagonal * 0.6f);
    
    // NOUVEAU: Ajuster les limites de zoom selon la taille de la carte
    float map_size_factor = std::max(_map_width, _map_height);
    
    // Plus la carte est grande, plus on peut dézoomer (zoom minimum plus petit)
    _min_zoom = 0.1f / (map_size_factor / 10.0f); // Dézoomer plus sur grandes cartes
    _min_zoom = std::max(0.01f, _min_zoom); // Ne pas aller trop loin
    
    // Zoom maximum reste raisonnable
    _max_zoom = std::min(20.0f, 10.0f + (map_size_factor / 5.0f));
}

void zappy::graphics::utils::Camera3D::setFPSFromPlayer(const zappy::game::entities::ZappyPlayer& player)
{
    // Positionner la caméra à la position du joueur
    _fps_position.x = static_cast<float>(player.x);
    _fps_position.y = 1.8f; // Hauteur des yeux du joueur
    _fps_position.z = static_cast<float>(player.y);
    
    // Orienter la caméra selon l'orientation du joueur
    // Correspondance avec getDirectionVector() du joueur :
    // Nord (1): (0, -1) → regarder vers Z négatif (avant)
    // Est (2): (1, 0) → regarder vers X positif (droite)  
    // Sud (3): (0, 1) → regarder vers Z positif (arrière)
    // Ouest (4): (-1, 0) → regarder vers X négatif (gauche)
    
    switch (player.orientation) {
    case 1: // Nord
        _fps_yaw = 180.0f; // Inverser de 0.0f à 180.0f
        break;
    case 2: // Est  
        _fps_yaw = 270.0f; // Inverser de 90.0f à 270.0f
        break;
    case 3: // Sud
        _fps_yaw = 0.0f;   // Inverser de 180.0f à 0.0f
        break;
    case 4: // Ouest
        _fps_yaw = 90.0f;  // Inverser de 270.0f à 90.0f
        break;
    default:
        _fps_yaw = 180.0f;
        break;
    }
    
    // Regarder légèrement vers le bas pour avoir une meilleure vue du terrain
    _fps_pitch = -10.0f; // Un peu plus d'inclinaison pour mieux voir
    
    // Mémoriser l'ID du joueur
    _fps_target_player_id = player.id;
    
    /*std::cout << "🎯 FPS camera set to player " << player.id 
              << " at (" << _fps_position.x << ", " << _fps_position.y << ", " << _fps_position.z << ")"
              << " facing " << player.getOrientationName() 
              << " (yaw: " << _fps_yaw << "°)" << std::endl;*/
}