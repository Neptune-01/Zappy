/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Input handling avec contrôles Creeper, sélection prioritaire et vue FPS
*/

#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include "zappy/graphics/utils/Camera3D.hpp"
#include "zappy/graphics/utils/MathUtils.hpp"
#include "zappy/game/entities/Player.hpp"

namespace zappy::input {
class InputHandler {
public:
    struct InputConfig {
        float mouse_camera_speed = 0.01f;
        float mouse_rotation_speed = 0.01f;
        float wheel_zoom_speed = 0.15f;
        
        float keyboard_camera_speed = 0.1f;
        float keyboard_rotation_speed = 0.1f;
        
        float min_zoom = 0.2f;
        float max_zoom = 15.0f;
        
        bool invert_mouse_y = false;
        bool invert_wheel = false;
        
        // Selection settings
        float click_threshold = 5.0f; // pixels - max distance for click vs drag
        float double_click_time = 0.5f; // seconds
        float player_selection_radius = 0.8f; // world units - how close to player to select
    };
    
    struct TileSelection {
        bool has_selection = false;
        int tile_x = -1;
        int tile_y = -1;
        sf::Vector2i click_screen_pos;
        float selection_time = 0.0f;
    };
    
    struct PlayerSelection {
        bool has_selection = false;
        int player_id = -1;
        sf::Vector2i click_screen_pos;
        float selection_time = 0.0f;
        float distance_to_player = 0.0f;
    };
    
    struct InputState {
        bool is_dragging = false;
        bool is_rotating = false;
        sf::Vector2i last_mouse_pos;
        sf::Vector2i mouse_press_pos;
        sf::Vector2f drag_start_camera;
        float rotation_start_angle = 0.0f;
        
        bool keys_pressed[sf::Keyboard::KeyCount] = {false};
        
        bool should_close = false;
        bool regenerate_map = false;
        bool add_resources = false;
        bool toggle_resources = false;
        bool toggle_players = false;
        bool toggle_stats = false;
        bool reset_rotation = false;
        
        // Grid et outline controls
        bool toggle_grid = false;
        bool toggle_outlines = false;
        
        // Coordinate display control
        bool toggle_coordinates = false;
        
        // Tile et player selection
        TileSelection tile_selection;
        bool tile_clicked = false;
        
        PlayerSelection player_selection;
        bool player_clicked = false;
        
        // Simple Creeper control - just toggle ON/OFF
        bool toggle_creeper_model = false;      // Touche [C] - Activer/désactiver Creeper
        
        // === SIMPLE FPS MODE ===
        bool toggle_fps_mode = false;           // Touche [Space] - Activer/désactiver vue FPS
        bool fps_mode_active = false;           // État actuel du mode FPS
    };

public:
    InputHandler();
    ~InputHandler() = default;
    
    void initialize(sf::RenderWindow* window, zappy::graphics::utils::Camera3D* camera);
    void setConfig(const InputConfig& config) { _config = config; }
    InputConfig getConfig() const { return _config; }
    
    void handleEvents();
    void handleEvent(const sf::Event& event);
    void updateContinuousInput();
    
    const InputState& getInputState() const { return _state; }
    void clearFrameFlags();
    
    bool isKeyPressed(sf::Keyboard::Key key) const;
    bool isKeyJustPressed(sf::Keyboard::Key key) const;
    bool shouldApplicationClose() const { return _state.should_close; }
    
    sf::Vector2f getCameraOffset() const;
    float getCameraRotation() const;
    float getCameraZoom() const;
    
    // Simple tile selection
    void selectTileByCoordinates(int x, int y);
    bool isValidTileCoordinate(int x, int y, int map_width, int map_height) const;
    
    // Player selection
    void selectPlayerById(int player_id);
    bool isValidPlayerId(int player_id, const std::map<int, zappy::game::entities::ZappyPlayer>& players) const;
    
    // === SIMPLE FPS MODE ===
    bool isFPSModeActive() const { return _state.fps_mode_active; }
    void setFPSMode(bool enabled);
    void toggleFPSMode();
    
    // === MÉTHODES POUR CONTRÔLES CREEPER ===
    void printCreeperControls() const;
    void printAllControls() const;
    
    // === MÉTHODES DANS InputHandlerSelection.cpp ===
    void handleSelectionWithPriority(const sf::Vector2i& screen_pos);
    bool testPlayerIntersection(const MathUtils::Vector3f& ray_start, 
                               const MathUtils::Vector3f& ray_dir, 
                               const sf::Vector2i& screen_pos);
    void testTileIntersection(const MathUtils::Vector3f& ray_start, 
                             const MathUtils::Vector3f& ray_dir, 
                             const sf::Vector2i& screen_pos);

private:
    void handleKeyboardEvent(const sf::Event& event);
    void handleMouseEvent(const sf::Event& event);
    void handleMouseButtonPressed(const sf::Event& event);
    void handleMouseButtonReleased(const sf::Event& event);
    void handleMouseMoved(const sf::Event& event);
    void handleMouseWheel(const sf::Event& event);
    void updateKeyboardCamera();
    void updateMouseCursors();
    void processActionKeys(sf::Keyboard::Key key);
    
    // === NOUVEAU: FPS handling ===
    void handleFPSMouseMovement(const sf::Event& event);
    void updateFPSMovement();
    
    float getDistanceBetweenPoints(const sf::Vector2i& p1, const sf::Vector2i& p2) const;
    
    InputConfig _config;
    InputState _state;
    InputState _previous_state;
    
    sf::RenderWindow* _window = nullptr;
    zappy::graphics::utils::Camera3D* _camera = nullptr;
    
    sf::Cursor _hand_cursor;
    sf::Cursor _arrow_cursor;
    
    bool _initialized = false;
    
    // Pour click detection
    sf::Clock _click_timer;
    
    // === NOUVEAU: Tracking des modificateurs ===
    bool _shift_pressed = false;
    bool _ctrl_pressed = false;
    bool _alt_pressed = false;
};
}
#endif /* !INPUTHANDLER_HPP */