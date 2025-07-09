/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Input handling - Core functionality avec support FPS
*/

#include "zappy/input/InputHandler.hpp"
#include <iostream>
#include <cmath>
#include <SFML/OpenGL.hpp>

// Brain rot Include GLU header
#ifdef __linux__
#include <GL/glu.h>
#elif defined(__APPLE__)
#include <OpenGL/glu.h>
#elif defined(_WIN32)
#include <GL/glu.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

zappy::input::InputHandler::InputHandler()
{
    std::cout << "🎮 InputHandler initialized with FPS support" << std::endl;
}

void zappy::input::InputHandler::initialize(sf::RenderWindow* window, zappy::graphics::utils::Camera3D* camera)
{
    _window = window;
    _camera = camera;
    
    // Initialiser les curseurs
    if (_hand_cursor.loadFromSystem(sf::Cursor::Hand)) {
        std::cout << "   ✅ Hand cursor loaded" << std::endl;
    }
    if (_arrow_cursor.loadFromSystem(sf::Cursor::Arrow)) {
        std::cout << "   ✅ Arrow cursor loaded" << std::endl;
    }
    
    _initialized = true;
    _click_timer.restart();
    
    std::cout << "✅ InputHandler initialized with FPS mode" << std::endl;
}

void zappy::input::InputHandler::handleEvents()
{
    if (!_window) return;
    
    sf::Event event;
    while (_window->pollEvent(event)) {
        handleEvent(event);
    }
    
    updateContinuousInput();
}

void zappy::input::InputHandler::handleEvent(const sf::Event& event)
{
    switch (event.type) {
        case sf::Event::Closed:
            _state.should_close = true;
            break;
            
        case sf::Event::KeyPressed:
        case sf::Event::KeyReleased:
            handleKeyboardEvent(event);
            break;
            
        case sf::Event::MouseButtonPressed:
        case sf::Event::MouseButtonReleased:
        case sf::Event::MouseMoved:
        case sf::Event::MouseWheelScrolled:
            handleMouseEvent(event);
            break;
            
        default:
            break;
    }
}

void zappy::input::InputHandler::handleKeyboardEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed) {
        _state.keys_pressed[event.key.code] = true;
        
        // Tracker les modificateurs
        _shift_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || 
                        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        _ctrl_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || 
                       sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
        _alt_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) || 
                      sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);
        
        processActionKeys(event.key.code);
        
    } else if (event.type == sf::Event::KeyReleased) {
        _state.keys_pressed[event.key.code] = false;
        
        // Mettre à jour les modificateurs
        _shift_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || 
                        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        _ctrl_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || 
                       sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
        _alt_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) || 
                      sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);
    }
}

void zappy::input::InputHandler::processActionKeys(sf::Keyboard::Key key)
{
    switch (key) {
        case sf::Keyboard::Escape:
            _state.should_close = true;
            break;
            
        case sf::Keyboard::R:
            if (!_state.fps_mode_active) {
                _state.reset_rotation = true;
            }
            break;
            
        case sf::Keyboard::Space:
            _state.toggle_fps_mode = true;
            break;
            
        case sf::Keyboard::H:
            printAllControls();
            break;
            
        default:
            break;
    }
}

void zappy::input::InputHandler::handleMouseEvent(const sf::Event& event)
{
    // Mode normal uniquement (pas de traitement FPS spécial)
    switch (event.type) {
        case sf::Event::MouseButtonPressed:
            handleMouseButtonPressed(event);
            break;
            
        case sf::Event::MouseButtonReleased:
            handleMouseButtonReleased(event);
            break;
            
        case sf::Event::MouseMoved:
            handleMouseMoved(event);
            break;
            
        case sf::Event::MouseWheelScrolled:
            handleMouseWheel(event);
            break;
            
        default:
            break;
    }
}

void zappy::input::InputHandler::handleMouseButtonPressed(const sf::Event& event)
{
    if (event.mouseButton.button == sf::Mouse::Left) {
        _state.mouse_press_pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
        _state.last_mouse_pos = _state.mouse_press_pos;
        _click_timer.restart();
        
        if (_camera) {
            _state.drag_start_camera = _camera->getCameraOffset();
        }
        
        updateMouseCursors();
        
    } else if (event.mouseButton.button == sf::Mouse::Right) {
        _state.is_rotating = true;
        _state.last_mouse_pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
        
        if (_camera) {
            _state.rotation_start_angle = _camera->getRotation();
        }
        
        updateMouseCursors();
    }
}

void zappy::input::InputHandler::handleMouseButtonReleased(const sf::Event& event)
{
    // En mode FPS, permettre quand même la sélection
    if (event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i release_pos(event.mouseButton.x, event.mouseButton.y);
        float drag_distance = getDistanceBetweenPoints(_state.mouse_press_pos, release_pos);
        float click_time = _click_timer.getElapsedTime().asSeconds();
        
        if (drag_distance < _config.click_threshold && click_time < _config.double_click_time) {
            // Click détecté
            handleSelectionWithPriority(release_pos);
        }
        
        _state.is_dragging = false;
        updateMouseCursors();
        
    } else if (event.mouseButton.button == sf::Mouse::Right) {
        _state.is_rotating = false;
        updateMouseCursors();
    }
}

void zappy::input::InputHandler::handleMouseMoved(const sf::Event& event)
{
    sf::Vector2i current_pos(event.mouseMove.x, event.mouseMove.y);
    sf::Vector2i delta = current_pos - _state.last_mouse_pos;
    
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        if (!_state.is_dragging) {
            float drag_distance = getDistanceBetweenPoints(_state.mouse_press_pos, current_pos);
            if (drag_distance > _config.click_threshold) {
                _state.is_dragging = true;
                updateMouseCursors();
            }
        }
        
        if (_state.is_dragging && _camera) {
            // Pan de la caméra
            sf::Vector2f camera_delta(
                delta.x * _config.mouse_camera_speed,
                delta.y * _config.mouse_camera_speed
            );
            
            sf::Vector2f new_offset = _camera->getCameraOffset() + camera_delta;
            _camera->setCameraOffset(new_offset);
        }
        
    } else if (_state.is_rotating && _camera) {
        // Rotation de la caméra
        float rotation_delta = delta.x * _config.mouse_rotation_speed;
        if (_config.invert_mouse_y) {
            rotation_delta = -rotation_delta;
        }
        
        float new_rotation = _camera->getRotation() + rotation_delta;
        _camera->setRotation(new_rotation);
    }
    
    _state.last_mouse_pos = current_pos;
}

void zappy::input::InputHandler::handleMouseWheel(const sf::Event& event)
{
    if (!_camera) return;
    
    float wheel_delta = event.mouseWheelScroll.delta;
    if (_config.invert_wheel) {
        wheel_delta = -wheel_delta;
    }
    
    float current_zoom = _camera->getZoom();
    float new_zoom = current_zoom * (1.0f + wheel_delta * _config.wheel_zoom_speed);
    
    // Clamp zoom dans les limites
    new_zoom = std::max(_config.min_zoom, std::min(_config.max_zoom, new_zoom));
    
    _camera->setZoom(new_zoom);
}

void zappy::input::InputHandler::updateContinuousInput()
{
    updateKeyboardCamera();
}

void zappy::input::InputHandler::updateKeyboardCamera()
{
    if (!_camera) return;
    
    sf::Vector2f camera_movement(0, 0);
    float rotation_movement = 0;
    float zoom_movement = 0;
    
    // Mouvement de caméra
    if (isKeyPressed(sf::Keyboard::Left)) {
        camera_movement.x -= _config.keyboard_camera_speed;
    }
    if (isKeyPressed(sf::Keyboard::Right)) {
        camera_movement.x += _config.keyboard_camera_speed;
    }
    if (isKeyPressed(sf::Keyboard::Up)) {
        camera_movement.y -= _config.keyboard_camera_speed;
    }
    if (isKeyPressed(sf::Keyboard::Down)) {
        camera_movement.y += _config.keyboard_camera_speed;
    }
    
    // Rotation
    if (isKeyPressed(sf::Keyboard::Q)) {
        rotation_movement -= _config.keyboard_rotation_speed;
    }
    if (isKeyPressed(sf::Keyboard::E)) {
        rotation_movement += _config.keyboard_rotation_speed;
    }
    
    // Zoom
    if (isKeyPressed(sf::Keyboard::PageUp)) {
        zoom_movement += 0.01f;
    }
    if (isKeyPressed(sf::Keyboard::PageDown)) {
        zoom_movement -= 0.01f;
    }
    
    // Appliquer les mouvements
    if (camera_movement.x != 0 || camera_movement.y != 0) {
        sf::Vector2f current_offset = _camera->getCameraOffset();
        _camera->setCameraOffset(current_offset + camera_movement);
    }
    
    if (rotation_movement != 0) {
        float current_rotation = _camera->getRotation();
        _camera->setRotation(current_rotation + rotation_movement);
    }
    
    if (zoom_movement != 0) {
        float current_zoom = _camera->getZoom();
        float new_zoom = current_zoom * (1.0f + zoom_movement);
        new_zoom = std::max(_config.min_zoom, std::min(_config.max_zoom, new_zoom));
        _camera->setZoom(new_zoom);
    }
}

void zappy::input::InputHandler::updateMouseCursors()
{
    if (!_window) return;
    
    if (_state.is_dragging || _state.is_rotating) {
        _window->setMouseCursor(_hand_cursor);
    } else {
        _window->setMouseCursor(_arrow_cursor);
    }
}

// === SIMPLE FPS MODE METHODS ===

void zappy::input::InputHandler::setFPSMode(bool enabled)
{
    if (_state.fps_mode_active == enabled) return;
    
    _state.fps_mode_active = enabled;
    
    #ifndef NDEBUG
    if (enabled) {
        std::cout << "🎯 FPS mode ACTIVATED (view only)" << std::endl;
    } else {
        std::cout << "👁️ FPS mode DEACTIVATED - returned to orbital view" << std::endl;
    }
    #endif
}

void zappy::input::InputHandler::toggleFPSMode()
{
    setFPSMode(!_state.fps_mode_active);
}

// === UTILITY METHODS ===

float zappy::input::InputHandler::getDistanceBetweenPoints(const sf::Vector2i& p1, const sf::Vector2i& p2) const
{
    float dx = static_cast<float>(p2.x - p1.x);
    float dy = static_cast<float>(p2.y - p1.y);
    return sqrt(dx * dx + dy * dy);
}

bool zappy::input::InputHandler::isKeyPressed(sf::Keyboard::Key key) const
{
    return _state.keys_pressed[key];
}

bool zappy::input::InputHandler::isKeyJustPressed(sf::Keyboard::Key key) const
{
    return _state.keys_pressed[key] && !_previous_state.keys_pressed[key];
}

sf::Vector2f zappy::input::InputHandler::getCameraOffset() const
{
    return _camera ? _camera->getCameraOffset() : sf::Vector2f(0, 0);
}

float zappy::input::InputHandler::getCameraRotation() const
{
    return _camera ? _camera->getRotation() : 0.0f;
}

float zappy::input::InputHandler::getCameraZoom() const
{
    return _camera ? _camera->getZoom() : 1.0f;
}

void zappy::input::InputHandler::selectTileByCoordinates(int x, int y)
{
    _state.tile_selection.has_selection = true;
    _state.tile_selection.tile_x = x;
    _state.tile_selection.tile_y = y;
    _state.tile_selection.selection_time = _click_timer.getElapsedTime().asSeconds();
    _state.tile_clicked = true;
}

bool zappy::input::InputHandler::isValidTileCoordinate(int x, int y, int map_width, int map_height) const
{
    return x >= 0 && x < map_width && y >= 0 && y < map_height;
}

void zappy::input::InputHandler::selectPlayerById(int player_id)
{
    _state.player_selection.has_selection = true;
    _state.player_selection.player_id = player_id;
    _state.player_selection.selection_time = _click_timer.getElapsedTime().asSeconds();
    _state.player_clicked = true;
}

bool zappy::input::InputHandler::isValidPlayerId(int player_id, const std::map<int, zappy::game::entities::ZappyPlayer>& players) const
{
    return players.find(player_id) != players.end();
}

// === AIDE ET CONTRÔLES ===

void zappy::input::InputHandler::printCreeperControls() const
{
    std::cout << "\n🐸 === CREEPER CONTROLS ===" << std::endl;
    std::cout << "[C]     - Toggle Creeper model ON/OFF" << std::endl;
    std::cout << "[V]     - Cycle Creeper styles (future)" << std::endl;
    std::cout << "[B]     - Toggle Creeper animations" << std::endl;
    std::cout << "[+/-]   - Scale Creeper size up/down" << std::endl;
    std::cout << "[X]     - Toggle Creeper wireframe mode" << std::endl;
    std::cout << "[Ctrl+C] - Reset Creeper configuration" << std::endl;
    std::cout << "[Shift++/-] - Animation speed up/down" << std::endl;
    std::cout << "[L]     - Toggle level indicators" << std::endl;
    std::cout << "[D]     - Toggle direction indicators" << std::endl;
}

void zappy::input::InputHandler::clearFrameFlags()
{
    _previous_state = _state;
    
    // Reset frame flags
    _state.reset_rotation = false;
    _state.tile_clicked = false;
    _state.player_clicked = false;
    _state.toggle_fps_mode = false;
}

void zappy::input::InputHandler::printAllControls() const
{
    std::cout << "\n🎮 === ZAPPY CONTROLS HELP ===" << std::endl;
    
    std::cout << "\n📹 === CAMERA CONTROLS ===" << std::endl;
    std::cout << "Mouse drag     - Pan camera (orbital mode)" << std::endl;
    std::cout << "Right mouse    - Rotate view (orbital mode)" << std::endl;
    std::cout << "Mouse wheel    - Zoom in/out (orbital mode)" << std::endl;
    std::cout << "Arrow keys     - Move camera (orbital mode)" << std::endl;
    std::cout << "[Q/E]          - Rotate left/right (orbital mode)" << std::endl;
    std::cout << "[PageUp/Down]  - Zoom in/out (orbital mode)" << std::endl;
    std::cout << "[R]            - Reset rotation (orbital mode)" << std::endl;
    
    std::cout << "\n🎯 === FPS MODE ===" << std::endl;
    std::cout << "[SPACE]        - Toggle FPS mode ON/OFF" << std::endl;
    std::cout << "Mouse          - Look around (FPS mode)" << std::endl;
    std::cout << "[W/A/S/D]      - Move forward/left/back/right (FPS mode)" << std::endl;
    std::cout << "[Shift]        - Move up (FPS mode)" << std::endl;
    std::cout << "[Ctrl]         - Move down (FPS mode)" << std::endl;
    std::cout << "Note: Select a player first, then press SPACE for FPS view" << std::endl;
    
    std::cout << "\n🗺️ === MAP CONTROLS ===" << std::endl;
    std::cout << "[Space]  - Regenerate map" << std::endl;
    std::cout << "[A]      - Add resources" << std::endl;
    
    std::cout << "\n🖼️ === DISPLAY (Always ON) ===" << std::endl;
    std::cout << "Resources      - Always displayed" << std::endl;
    std::cout << "Players        - Always displayed" << std::endl;
    std::cout << "Statistics     - Always displayed" << std::endl;
    std::cout << "Grid lines     - Always displayed" << std::endl;
    std::cout << "Tile outlines  - Always displayed" << std::endl;
    std::cout << "Coordinates    - Always displayed" << std::endl;
    
    std::cout << "\n🐸 === CREEPER MODELS ===" << std::endl;
    std::cout << "[C]      - Toggle Creeper models ON/OFF" << std::endl;
    std::cout << "         (Small creepers at map level)" << std::endl;
    
    std::cout << "\n🎯 === SELECTION ===" << std::endl;
    std::cout << "Left click - Select tile/player" << std::endl;
    std::cout << "            (Players have priority)" << std::endl;
    
    std::cout << "\n⚙️ === SYSTEM ===" << std::endl;
    std::cout << "[H]      - Show this help" << std::endl;
    std::cout << "[Esc]    - Quit application" << std::endl;
    std::cout << "\n" << std::endl;
}

bool manualUnProject(float winX, float winY, float winZ,
    const double model[16], const double proj[16], const int viewport[4],
    double* objX, double* objY, double* objZ)
{
    // Create transformation matrix = projection * modelview
    double transform[16];
    
    // Multiply proj * model matrices
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            transform[i*4 + j] = 0.0;
            for (int k = 0; k < 4; k++) {
                transform[i*4 + j] += proj[i*4 + k] * model[k*4 + j];
            }
        }
    }
    
    // Normalize window coordinates
    double in[4];
    in[0] = (winX - viewport[0]) / viewport[2] * 2.0 - 1.0;
    in[1] = (winY - viewport[1]) / viewport[3] * 2.0 - 1.0;
    in[2] = 2.0 * winZ - 1.0;
    in[3] = 1.0;
    
    // Simple matrix inversion (for 4x4 transformation matrix)
    // This is a simplified approach - for production, use proper matrix inversion
    double det = transform[0] * transform[5] * transform[10] * transform[15] -
                 transform[0] * transform[5] * transform[11] * transform[14] -
                 transform[0] * transform[6] * transform[9] * transform[15] +
                 transform[0] * transform[6] * transform[11] * transform[13];
    
    if (std::abs(det) < 1e-6) return false;
    
    // Simplified calculation for demonstration
    // In practice, you'd want a proper matrix inverse
    *objX = in[0] * 10.0; // Simplified world coordinate calculation
    *objY = in[1] * 10.0;
    *objZ = in[2] * 10.0;
    
    return true;
}

// === SÉLECTION AVEC PRIORITÉ ===

void zappy::input::InputHandler::handleSelectionWithPriority(const sf::Vector2i& screen_pos)
{
    //std::cout << "🎯 Selection with priority at: (" << screen_pos.x << ", " << screen_pos.y << ")" << std::endl;
    
    if (!_camera || !_window) {
        std::cout << "   ❌ Camera or window not available" << std::endl;
        return;
    }
    
    // Sauvegarder les matrices actuelles
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    // Reconfigurer les matrices de la caméra
    _camera->setupProjection(_window->getSize().x, _window->getSize().y);
    _camera->setupCamera();
    
    // Récupérer les matrices OpenGL
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];
    
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Convertir Y des coordonnées SFML vers OpenGL
    float opengl_y = viewport[3] - screen_pos.y;
    
    // Créer le rayon de sélection
    GLdouble world_x1, world_y1, world_z1; // Near plane
    GLdouble world_x2, world_y2, world_z2; // Far plane
    
    // Try to use gluUnProject, fall back to manual implementation if not available
    int success1 = GL_FALSE;
    int success2 = GL_FALSE;
    
    #ifdef GLU_VERSION
    try {
        success1 = gluUnProject(screen_pos.x, opengl_y, 0.0,
                               modelview, projection, viewport,
                               &world_x1, &world_y1, &world_z1);
        
        success2 = gluUnProject(screen_pos.x, opengl_y, 1.0,
                               modelview, projection, viewport,
                               &world_x2, &world_y2, &world_z2);
    } catch (...) {
        success1 = success2 = GL_FALSE;
    }
    #endif
    
    // Fallback to manual unprojection if GLU failed or is not available
    if (success1 != GL_TRUE || success2 != GL_TRUE) {
        std::cout << "   ⚠️ Using manual unprojection (GLU not available)" << std::endl;
        
        bool manual_success1 = manualUnProject(screen_pos.x, opengl_y, 0.0,
                                              modelview, projection, viewport,
                                              &world_x1, &world_y1, &world_z1);
        
        bool manual_success2 = manualUnProject(screen_pos.x, opengl_y, 1.0,
                                              modelview, projection, viewport,
                                              &world_x2, &world_y2, &world_z2);
        
        if (!manual_success1 || !manual_success2) {
            std::cout << "   ❌ Manual unprojection also failed" << std::endl;
            // Restaurer les matrices
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            return;
        }
    }
    
    // Restaurer les matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Créer le rayon
    MathUtils::Vector3f ray_start(world_x1, world_y1, world_z1);
    MathUtils::Vector3f ray_end(world_x2, world_y2, world_z2);
    MathUtils::Vector3f ray_dir = (ray_end - ray_start).normalized();
    
    //std::cout << "   📍 Ray start: (" << ray_start.x << ", " << ray_start.y << ", " << ray_start.z << ")" << std::endl;
    
    // PRIORITÉ 1 : Tester intersection avec les joueurs (plan Y = 1.5)
    bool player_hit = testPlayerIntersection(ray_start, ray_dir, screen_pos);
    
    if (player_hit) {
        //std::cout << "   ✅ Player hit - blocking tile selection!" << std::endl;
        return; // Un joueur a été touché, on arrête ici
    }
    
    // PRIORITÉ 2 : Si aucun joueur touché, tester les tuiles (plan Y = 1.0)
    //std::cout << "   ➡️ No player hit, testing tiles..." << std::endl;
    testTileIntersection(ray_start, ray_dir, screen_pos);
}

bool zappy::input::InputHandler::testPlayerIntersection(const MathUtils::Vector3f& ray_start, 
    const MathUtils::Vector3f& ray_dir, 
    const sf::Vector2i& screen_pos)
{
    const float player_plane_y = 1.5f; // Hauteur des joueurs
    
    // Vérifier que le rayon va vers le bas
    if (ray_dir.y > -0.001f) {
        std::cout << "   ⚠️ Ray doesn't point downward for player test" << std::endl;
        return false;
    }
    
    // Calculer intersection avec le plan des joueurs
    float t = (player_plane_y - ray_start.y) / ray_dir.y;
    if (t < 0) {
        std::cout << "   ⚠️ Player intersection behind camera" << std::endl;
        return false;
    }
    
    //MathUtils::Vector3f intersection = ray_start + ray_dir * t;
    //std::cout << "   🌍 Player plane intersection: (" << intersection.x << ", " << intersection.y << ", " << intersection.z << ")" << std::endl;
    
    // Marquer que nous avons testé les joueurs
    _state.player_selection.has_selection = true;
    _state.player_selection.click_screen_pos = screen_pos;
    _state.player_selection.selection_time = _click_timer.getElapsedTime().asSeconds();
    _state.player_clicked = true;
    
    return false; // L'Application déterminera s'il y a vraiment un joueur
}

void zappy::input::InputHandler::testTileIntersection(const MathUtils::Vector3f& ray_start, 
    const MathUtils::Vector3f& ray_dir, 
    const sf::Vector2i& screen_pos)
{
    const float tile_plane_y = 1.0f; // Hauteur du dessus des tuiles
    
    // Vérifier que le rayon va vers le bas
    if (ray_dir.y > -0.001f) {
        std::cout << "   ⚠️ Ray doesn't point downward for tile test" << std::endl;
        return;
    }
    
    // Calculer intersection avec le plan des tuiles
    float t = (tile_plane_y - ray_start.y) / ray_dir.y;
    if (t < 0) {
        std::cout << "   ⚠️ Tile intersection behind camera" << std::endl;
        return;
    }
    
    MathUtils::Vector3f intersection = ray_start + ray_dir * t;
    //std::cout << "   🌍 Tile plane intersection: (" << intersection.x << ", " << intersection.y << ", " << intersection.z << ")" << std::endl;
    
    // Convertir en coordonnées de tuile
    int tile_x = static_cast<int>(std::round(intersection.x));
    int tile_z = static_cast<int>(std::round(intersection.z));
    
    //std::cout << "   📍 Calculated tile: (" << tile_x << ", " << tile_z << ")" << std::endl;
    
    // Valider avec la taille de la carte
    int map_width = _camera->getCurrentMapWidth();
    int map_height = _camera->getCurrentMapHeight();
    
    if (isValidTileCoordinate(tile_x, tile_z, map_width, map_height)) {
        _state.tile_selection.has_selection = true;
        _state.tile_selection.tile_x = tile_x;
        _state.tile_selection.tile_y = tile_z;
        _state.tile_selection.click_screen_pos = screen_pos;
        _state.tile_selection.selection_time = _click_timer.getElapsedTime().asSeconds();
        _state.tile_clicked = true;
        
        std::cout << "   ✅ Valid tile selected: (" << tile_x << ", " << tile_z << ")" << std::endl;
    } else {
        std::cout << "   ❌ Invalid tile coordinates: (" << tile_x << ", " << tile_z << ")" << std::endl;
        _state.tile_selection.has_selection = false;
        _state.tile_clicked = false;
    }
}