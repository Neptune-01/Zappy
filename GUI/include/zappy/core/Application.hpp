/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Main application coordinator avec support FPS
*/

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <memory>
#include "zappy/graphics/utils/Camera3D.hpp"
#include "zappy/graphics/renderers/Renderer3D.hpp"
#include "zappy/input/InputHandler.hpp"
#include "zappy/graphics/renderers/UIRenderer.hpp"
#include "zappy/game/entities/Player.hpp"
#include "zappy/game/world/Tile.hpp"
#include "zappy/game/world/SharedGameState.hpp"

namespace zappy::core {

class Application {
public:
    struct AppConfig {
        unsigned int window_width = 1400;
        unsigned int window_height = 900;
        std::string window_title = "Zappy Visualizer 3D";
        unsigned int framerate_limit = 60;
        
        unsigned int depth_bits = 24;
        unsigned int stencil_bits = 8;
        unsigned int antialiasing_level = 0;
        unsigned int major_version = 3;
        unsigned int minor_version = 3;
    };

public:
    Application(std::shared_ptr<zappy::game::world::SharedGameState> shared_state);
    ~Application() = default;
    
    // Core lifecycle methods
    bool initialize();
    bool initialize(const AppConfig& config);
    void run();
    void shutdown();
    
    // Configuration
    void setConfig(const AppConfig& config) { _config = config; }
    AppConfig getConfig() const { return _config; }
    
    // State queries
    bool isRunning() const { return _running && _window.isOpen(); }
    sf::RenderWindow& getWindow() { return _window; }
    std::shared_ptr<zappy::game::world::SharedGameState> getSharedState() { return _shared_state; }

    // Selection info accessors
    zappy::graphics::renderers::UIRenderer::TileInfo getCurrentTileInfo() const;
    zappy::graphics::renderers::UIRenderer::PlayerInfo getCurrentPlayerInfo() const;

private:
    // Initialization methods
    bool initializeWindow();
    bool initializeComponents();
    void setupInitialState();
    
    // Main loop methods
    void update(float deltaTime);
    void render();
    void handleInput();
    
    // === SIMPLE FPS mode methods ===
    void handleFPSModeToggle();
    void renderPlayersExceptFPSTarget(const std::map<int, zappy::game::entities::ZappyPlayer>& players);
    
    // Selection handling
    void processInputActions();
    void handlePrioritySelection(const zappy::input::InputHandler::InputState& input_state);
    bool trySelectPlayerAtPosition(const sf::Vector2i& screen_pos);
    void handleTileSelection(int x, int y);
    float calculateDistanceToPlayer(const MathUtils::Vector3f& click_world_pos, const zappy::game::entities::ZappyPlayer& player) const;
    
    // Game state synchronization
    void updateCameraFromGameState();
    void updateSelectionsFromGameState();
    void processGameEvents();
    
    // Helper methods
    void printWelcomeMessage();
    void updateSelectionsAfterStateChange();
    
    // Thread-safe accessors to SharedGameState
    std::vector<std::vector<zappy::game::world::ZappyTile>> getWorldMapSafe() const;
    std::map<int, zappy::game::entities::ZappyPlayer> getPlayersSafe() const;
    
    // Configuration and state
    AppConfig _config;
    bool _running = false;
    bool _initialized = false;
    float _current_fps = 60.0f;
    
    // Graphics components
    sf::RenderWindow _window;
    sf::Clock _fps_clock;
    zappy::graphics::utils::Camera3D _camera;
    zappy::graphics::renderers::Renderer3D _renderer;
    zappy::input::InputHandler _input_handler;
    zappy::graphics::renderers::UIRenderer _ui_renderer;
    
    // Shared game state (managed by network thread)
    std::shared_ptr<zappy::game::world::SharedGameState> _shared_state;
    
    // Selection states
    zappy::graphics::renderers::UIRenderer::TileInfo _current_tile_selection;
    zappy::graphics::renderers::UIRenderer::PlayerInfo _current_player_selection;
};

}

#endif /* !APPLICATION_HPP */