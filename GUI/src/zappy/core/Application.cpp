/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Main application coordinator - Network-driven version avec FPS simple
*/

#include "zappy/core/Application.hpp"
#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <iostream>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iomanip>

zappy::core::Application::Application(std::shared_ptr<zappy::game::world::SharedGameState> shared_state)
    : _shared_state(shared_state)
{
    if (!_shared_state) {
        throw std::runtime_error("SharedGameState cannot be null");
    }
    
    std::cout << "🚀 Zappy 3D Application created with SharedGameState and simple FPS support" << std::endl;
}

bool zappy::core::Application::initialize()
{
    AppConfig default_config;
    return initialize(default_config);
}

bool zappy::core::Application::initialize(const AppConfig& config)
{
    if (_initialized) return true;
    
    std::cout << "🚀 Initializing Zappy 3D Application with simple FPS support..." << std::endl;
    
    _config = config;
    
    if (!initializeWindow()) {
        std::cerr << "❌ Failed to initialize window" << std::endl;
        return false;
    }
    
    if (!initializeComponents()) {
        std::cerr << "❌ Failed to initialize components" << std::endl;
        return false;
    }
    
    setupInitialState();
    
    _initialized = true;
    _running = true;
    
    std::cout << "✅ Zappy 3D Application initialized successfully with simple FPS support!" << std::endl;
    printWelcomeMessage();
    
    return true;
}

bool zappy::core::Application::initializeWindow()
{
    sf::ContextSettings settings(
        _config.depth_bits,
        _config.stencil_bits,
        _config.antialiasing_level,
        _config.major_version,
        _config.minor_version
    );
    
    _window.create(
        sf::VideoMode(_config.window_width, _config.window_height),
        _config.window_title,
        sf::Style::Default,
        settings
    );
    
    if (!_window.isOpen()) {
        return false;
    }
    
    _window.setFramerateLimit(_config.framerate_limit);
    _window.setActive(true);
    
    std::cout << "🖼️ Window created: " << _config.window_width << "x" << _config.window_height << std::endl;
    return true;
}

bool zappy::core::Application::initializeComponents()
{
    if (!_renderer.initialize()) {
        return false;
    }
    
    _input_handler.initialize(&_window, &_camera);
    _ui_renderer.initialize(&_window);
    
    return true;
}

void zappy::core::Application::setupInitialState()
{
    // Initialize empty selections
    _current_tile_selection.has_selection = false;
    _current_player_selection.has_selection = false;
    
    // Initialize camera in orbital mode
    _camera.setMode(zappy::graphics::utils::Camera3D::CameraMode::ORBITAL);
    
    std::cout << "🗺️ Application ready with orbital camera mode" << std::endl;
}

void zappy::core::Application::printWelcomeMessage()
{
    std::cout << "🎮 Controls:" << std::endl;
    std::cout << "  === ORBITAL MODE ===" << std::endl;
    std::cout << "  - Mouse drag: Move camera" << std::endl;
    std::cout << "  - Right click: Rotate camera" << std::endl;
    std::cout << "  - Mouse wheel: Zoom in/out" << std::endl;
    std::cout << "  - Arrow keys: Move camera" << std::endl;
    std::cout << "  - Q/E: Rotate camera" << std::endl;
    std::cout << "  - R: Reset rotation" << std::endl;
    std::cout << "  - Left Click: Select tiles and players" << std::endl;
    std::cout << "  === SIMPLE FPS MODE ===" << std::endl;
    std::cout << "  - SPACE: Toggle FPS view (select a player first!)" << std::endl;
    std::cout << "  - Static view: No movement controls in FPS mode" << std::endl;
}

void zappy::core::Application::run()
{
    if (!_initialized) {
        std::cerr << "❌ Application not initialized!" << std::endl;
        return;
    }
    
    std::cout << "🎮 Starting main loop with simple FPS support..." << std::endl;
    
    while (isRunning()) {
        float deltaTime = _fps_clock.restart().asSeconds();
        if (deltaTime > 0.001f) {
            _current_fps = 1.0f / deltaTime;
        }
        
        handleInput();
        update(deltaTime);
        render();
    }
    
    shutdown();
}

void zappy::core::Application::handleInput()
{
    _input_handler.handleEvents();
    processInputActions();
}

void zappy::core::Application::update(float deltaTime)
{
    (void)deltaTime; // Suppress unused parameter warning
    
    // Update camera based on current game state
    updateCameraFromGameState();
    
    // Process any new game events from the network
    processGameEvents();
    
    // Update entity renderer with delta time for animations
    auto* entity_renderer = _renderer.getEntityRenderer();
    if (entity_renderer) {
        entity_renderer->setDeltaTime(deltaTime);
    }
    
    // Update selections based on current game state
    updateSelectionsFromGameState();
    
    _input_handler.clearFrameFlags();
}

void zappy::core::Application::render()
{
    OpenGLUtils::clearBuffers();
    
    _camera.setupProjection(_window.getSize().x, _window.getSize().y);
    _camera.setupCamera();
    
    // Get current state from SharedGameState
    auto world_map = getWorldMapSafe();
    auto players_map = getPlayersSafe();
    
    // Only render if we have a valid map
    if (!world_map.empty() && !world_map[0].empty()) {
        // 1. Render the terrain with grid
        auto* terrain_renderer = _renderer.getTerrainRenderer();
        if (terrain_renderer) {
            _renderer.renderTileMap(world_map);
        }
        
        // 2. Render resources on tiles
        if (terrain_renderer) {
            for (int y = 0; y < static_cast<int>(world_map.size()); y++) {
                for (int x = 0; x < static_cast<int>(world_map[y].size()); x++) {
                    if (world_map[y][x].getTotalResources() > 0) {
                        terrain_renderer->renderResourceStacks(
                            static_cast<float>(x), 
                            0.0f,
                            static_cast<float>(y), 
                            world_map[y][x]
                        );
                    }
                }
            }
        }
        
        // 3. Render players (but not the FPS target player from inside view)
        if (_camera.isFPSMode()) {
            renderPlayersExceptFPSTarget(players_map);
        } else {
            _renderer.renderPlayers(players_map);
        }
    }
    
    // 4. UI with tile and player info
    _camera.resetMatrices();
    OpenGLUtils::disableDepthTest();

    auto stats = _ui_renderer.calculateGameStats(world_map, players_map, 
                                                _current_fps, _camera.getZoom(), 
                                                _camera.getRotation());
    
    bool has_tile_info = _current_tile_selection.has_selection;
    bool has_player_info = _current_player_selection.has_selection;
    
    if (has_tile_info && has_player_info) {
        _ui_renderer.renderUI(stats, _current_tile_selection, _current_player_selection);
    } else if (has_tile_info) {
        _ui_renderer.renderUI(stats, _current_tile_selection);
    } else {
        _ui_renderer.renderUI(stats);
    }
    
    OpenGLUtils::enableDepthTest();
    
    _window.display();
}

void zappy::core::Application::renderPlayersExceptFPSTarget(const std::map<int, zappy::game::entities::ZappyPlayer>& players)
{
    int fps_target = _camera.getFPSTarget();
    
    for (const auto& pair : players) {
        // Don't render the player we're viewing from in FPS mode
        if (pair.first != fps_target) {
            _renderer.renderPlayer(pair.second);
        }
    }
}

void zappy::core::Application::shutdown()
{
    std::cout << "🔚 Shutting down application..." << std::endl;
    
    _renderer.shutdown();
    _window.close();
    _running = false;
    
    std::cout << "✅ Application shutdown complete" << std::endl;
}

// === GAME STATE SYNCHRONIZATION ===

void zappy::core::Application::updateCameraFromGameState()
{
    int map_width = _shared_state->getMapWidth();
    int map_height = _shared_state->getMapHeight();
    
    if (map_width > 0 && map_height > 0) {
        _camera.setMapSize(map_width, map_height);
    }
}

void zappy::core::Application::updateSelectionsFromGameState()
{
    auto world_map = getWorldMapSafe();
    auto players_map = getPlayersSafe();
    
    // Update tile selection if it exists
    if (_current_tile_selection.has_selection && !world_map.empty()) {
        _current_tile_selection = _ui_renderer.createTileInfo(
            _current_tile_selection.x, _current_tile_selection.y, world_map, players_map);
    }
    
    // Update player selection if it exists
    if (_current_player_selection.has_selection) {
        _current_player_selection = _ui_renderer.createPlayerInfo(
            _current_player_selection.player_id, players_map);
    }
}

void zappy::core::Application::processGameEvents()
{
    auto events = _shared_state->getAndClearEvents();
    
    for (const auto& event : events) {
        switch (event.type) {
            case zappy::game::world::GameEvent::PLAYER_MOVED:
                std::cout << "👤 Player " << event.player_id << " moved to (" 
                         << event.x << ", " << event.y << ")" << std::endl;
                
                // If we're in FPS mode and this is our target player, update camera position
                if (_camera.isFPSMode() && _camera.getFPSTarget() == event.player_id) {
                    auto players_map = getPlayersSafe();
                    auto player_it = players_map.find(event.player_id);
                    if (player_it != players_map.end()) {
                        _camera.setFPSFromPlayer(player_it->second);
                    }
                }
                break;
                
            case zappy::game::world::GameEvent::PLAYER_LEVELED:
                std::cout << "📈 Player " << event.player_id << " reached level " 
                         << event.data.at("level") << std::endl;
                break;
                
            case zappy::game::world::GameEvent::RESOURCE_CHANGED:
                if (event.x >= 0 && event.y >= 0) {
                    //std::cout << "💎 Resources changed at (" << event.x << ", " << event.y << ")" << std::endl;
                }
                break;
                
            case zappy::game::world::GameEvent::INCANTATION_STARTED:
                std::cout << "🔮 Incantation started at (" << event.x << ", " << event.y << ")" << std::endl;
                break;
                
            case zappy::game::world::GameEvent::INCANTATION_ENDED:
                std::cout << "✨ Incantation ended: " << event.message << std::endl;
                break;
                
            case zappy::game::world::GameEvent::BROADCAST_RECEIVED:
                if (event.player_id >= 0) {
                    std::cout << "📢 Player " << event.player_id << " broadcasts: " << event.message << std::endl;
                } else {
                    std::cout << "📢 Server message: " << event.message << std::endl;
                }
                break;
                
            default:
                break;
        }
    }
}

// === HELPER METHODS FOR THREAD-SAFE ACCESS ===

std::vector<std::vector<zappy::game::world::ZappyTile>> zappy::core::Application::getWorldMapSafe() const
{
    return _shared_state->getWorldMap();
}

std::map<int, zappy::game::entities::ZappyPlayer> zappy::core::Application::getPlayersSafe() const
{
    std::map<int, zappy::game::entities::ZappyPlayer> players_map;
    auto players_vector = _shared_state->getPlayers();
    
    for (const auto& player : players_vector) {
        players_map[player.id] = player;
    }
    
    return players_map;
}

// === SELECTION METHODS ===

void zappy::core::Application::processInputActions()
{
    const auto& input_state = _input_handler.getInputState();
    
    if (input_state.should_close) {
        _running = false;
    }
    
    // FPS mode toggle
    if (input_state.toggle_fps_mode) {
        handleFPSModeToggle();
    }
    
    // Selection logic
    if (input_state.player_clicked || input_state.tile_clicked) {
        handlePrioritySelection(input_state);
    }
}

void zappy::core::Application::handleFPSModeToggle()
{
    if (!_camera.isFPSMode()) {
        // Trying to enter FPS mode
        if (_current_player_selection.has_selection) {
            // We have a selected player, enter FPS mode
            auto players_map = getPlayersSafe();
            auto player_it = players_map.find(_current_player_selection.player_id);
            
            if (player_it != players_map.end()) {
                _camera.setMode(zappy::graphics::utils::Camera3D::CameraMode::FPS);
                _camera.setFPSFromPlayer(player_it->second);
                _input_handler.setFPSMode(true);
                
                //std::cout << "🎯 Entered simple FPS mode for player " << player_it->second.id 
                //         << " (" << player_it->second.team << ")" << std::endl;
            } else {
                std::cout << "❌ Selected player not found, cannot enter FPS mode" << std::endl;
            }
        } else {
            std::cout << "❌ No player selected! Select a player first, then press SPACE for FPS view" << std::endl;
        }
    } else {
        // Exit FPS mode
        _camera.setMode(zappy::graphics::utils::Camera3D::CameraMode::ORBITAL);
        _input_handler.setFPSMode(false);
        
        std::cout << "👁️ Exited FPS mode, returned to orbital view" << std::endl;
    }
}

void zappy::core::Application::handlePrioritySelection(const zappy::input::InputHandler::InputState& input_state)
{
    bool player_was_selected = false;
    
    // PRIORITY 1: Test player selection first
    if (input_state.player_clicked && input_state.player_selection.has_selection) {
        sf::Vector2i click_pos = input_state.player_selection.click_screen_pos;
        player_was_selected = trySelectPlayerAtPosition(click_pos);
        
        if (player_was_selected) {
            return; // Player selected, stop here
        }
    }
    
    // PRIORITY 2: Test tiles only if no player was selected
    if (input_state.tile_clicked && input_state.tile_selection.has_selection && !player_was_selected) {
        int tile_x = input_state.tile_selection.tile_x;
        int tile_y = input_state.tile_selection.tile_y;
        
        // Get current map dimensions
        auto world_map = getWorldMapSafe();
        int map_width = world_map.empty() ? 0 : static_cast<int>(world_map[0].size());
        int map_height = static_cast<int>(world_map.size());
        
        // Validate coordinates
        if (_input_handler.isValidTileCoordinate(tile_x, tile_y, map_width, map_height)) {
            handleTileSelection(tile_x, tile_y);
        } else {
            _current_tile_selection.has_selection = false;
        }
    }
}

bool zappy::core::Application::trySelectPlayerAtPosition(const sf::Vector2i& screen_pos)
{    
    auto players_map = getPlayersSafe();
    if (players_map.empty()) {
        return false;
    }
    
    // Recreate the ray (same logic as in InputHandler)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    _camera.setupProjection(_window.getSize().x, _window.getSize().y);
    _camera.setupCamera();
    
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];
    
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    float opengl_y = viewport[3] - screen_pos.y;
    
    GLdouble world_x1, world_y1, world_z1;
    GLdouble world_x2, world_y2, world_z2;
    
    int success1 = gluUnProject(screen_pos.x, opengl_y, 0.0,
                               modelview, projection, viewport,
                               &world_x1, &world_y1, &world_z1);
    
    int success2 = gluUnProject(screen_pos.x, opengl_y, 1.0,
                               modelview, projection, viewport,
                               &world_x2, &world_y2, &world_z2);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    if (success1 != GL_TRUE || success2 != GL_TRUE) {
        return false;
    }
    
    MathUtils::Vector3f ray_start(world_x1, world_y1, world_z1);
    MathUtils::Vector3f ray_end(world_x2, world_y2, world_z2);
    MathUtils::Vector3f ray_dir = (ray_end - ray_start).normalized();
    
    // Intersect ray with player plane (y = 1.5)
    float player_plane_y = 1.5f;
    
    if (ray_dir.y > -0.001f) {
        return false;
    }
    
    float t = (player_plane_y - ray_start.y) / ray_dir.y;
    if (t < 0) {
        return false;
    }
    
    MathUtils::Vector3f intersection = ray_start + ray_dir * t;
    
    // Find closest player to intersection point
    int closest_player_id = -1;
    float closest_distance = 0.8f + 1.0f; // player_selection_radius + margin
    const float PLAYER_SELECTION_RADIUS = 0.8f;
    
    for (const auto& player_pair : players_map) {
        const zappy::game::entities::ZappyPlayer& player = player_pair.second;
        float distance = calculateDistanceToPlayer(intersection, player);
        
        if (distance < PLAYER_SELECTION_RADIUS && distance < closest_distance) {
            closest_player_id = player.id;
            closest_distance = distance;
        }
    }
    
    if (closest_player_id != -1) {
        // Select the found player
        _current_player_selection = _ui_renderer.createPlayerInfo(closest_player_id, players_map);
        
        if (_current_player_selection.has_selection) {
            //const auto& player = _current_player_selection.player_data;
            //std::cout << "👤 Player " << player.id << " (" << player.team << ") selected" << std::endl;
            std::cout << "   💡 Press SPACE to enter FPS mode for this player!" << std::endl;
        }
        
        return true; // Player successfully selected
    } else {
        return false; // No player found
    }
}

void zappy::core::Application::handleTileSelection(int x, int y)
{
    // Clear player selection since we're selecting a tile
    _current_player_selection.has_selection = false;
    
    // Create tile info
    auto world_map = getWorldMapSafe();
    auto players_map = getPlayersSafe();
    _current_tile_selection = _ui_renderer.createTileInfo(x, y, world_map, players_map);
    
    if (_current_tile_selection.has_selection) {
        //std::cout << "🎯 Tile (" << x << ", " << y << ") selected" << std::endl;
        
        // Log tile contents for console feedback
        const auto& tile = _current_tile_selection.tile_data;
        if (tile.getTotalResources() > 0) {
            std::cout << "  💎 Resources: " << tile.getTotalResources() << " total" << std::endl;
        }
        
        if (!_current_tile_selection.players_on_tile.empty()) {
            std::cout << "  👥 Players: " << _current_tile_selection.players_on_tile.size() << " on this tile" << std::endl;
        }
    }
}

float zappy::core::Application::calculateDistanceToPlayer(const MathUtils::Vector3f& click_world_pos, const zappy::game::entities::ZappyPlayer& player) const
{
    // Players are positioned at their tile coordinates + some Y offset
    MathUtils::Vector3f player_pos(
        static_cast<float>(player.x), 
        1.5f, // Y position of players (roughly their center height)
        static_cast<float>(player.y)
    );
    
    // Calculate distance in X-Z plane (ignore Y difference)
    float dx = click_world_pos.x - player_pos.x;
    float dz = click_world_pos.z - player_pos.z;
    
    return std::sqrt(dx * dx + dz * dz);
}

void zappy::core::Application::updateSelectionsAfterStateChange()
{
    updateSelectionsFromGameState();
}

zappy::graphics::renderers::UIRenderer::TileInfo zappy::core::Application::getCurrentTileInfo() const
{
    return _current_tile_selection;
}

zappy::graphics::renderers::UIRenderer::PlayerInfo zappy::core::Application::getCurrentPlayerInfo() const
{
    return _current_player_selection;
}