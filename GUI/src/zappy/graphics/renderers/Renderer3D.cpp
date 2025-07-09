/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Core 3D Renderer
*/

#include "zappy/graphics/renderers/Renderer3D.hpp"
#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <iostream>

zappy::graphics::renderers::Renderer3D::Renderer3D()
{
    //std::cout << "🎨 Renderer3D core created" << std::endl;
}

zappy::graphics::renderers::Renderer3D::~Renderer3D()
{
    shutdown();
}

bool zappy::graphics::renderers::Renderer3D::initialize()
{
    if (_initialized) return true;
    
    //std::cout << "🎨 Initializing Renderer3D core..." << std::endl;
    
    // Initialize OpenGL
    initOpenGL();
    
    // Setup all rendering systems
    setupSystems();
    
    // Apply configuration
    applyConfig();
    
    _initialized = true;
    //std::cout << "✅ Renderer3D core initialized successfully!" << std::endl;
    
    return true;
}

void zappy::graphics::renderers::Renderer3D::shutdown()
{
    if (!_initialized) return;
    
    //std::cout << "🔚 Shutting down Renderer3D..." << std::endl;
    _initialized = false;
}

void zappy::graphics::renderers::Renderer3D::initOpenGL()
{
    //std::cout << "🔧 Initializing OpenGL for 3D rendering..." << std::endl;
    
    // Initialize OpenGL utilities
    OpenGLUtils::initializeOpenGL();
    
    // Setup specific 3D rendering state
    OpenGLUtils::enableDepthTest();
    OpenGLUtils::enableTexturing();
    
    // Enable color material for dynamic colors
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    //std::cout << "✅ OpenGL initialized for 3D rendering!" << std::endl;
}

void zappy::graphics::renderers::Renderer3D::setupSystems()
{
    //std::cout << "⚙️ Setting up rendering systems..." << std::endl;
    
    // Initialize texture manager and load default textures
    _texture_manager.loadDefaultTextures();
    
    // Initialize terrain renderer
    _terrain_renderer.initialize(&_texture_manager);
    
    // Initialize entity renderer avec TextureManager pour les Creepers
    _entity_renderer.initialize(&_texture_manager);
    
    //std::cout << "✅ All rendering systems initialized!" << std::endl;
}

void zappy::graphics::renderers::Renderer3D::setConfig(const RenderConfig& config)
{
    _config = config;
    if (_initialized) {
        applyConfig();
    }
}

void zappy::graphics::renderers::Renderer3D::applyConfig()
{
    // Apply texture configuration
    if (_config.textures_enabled) {
        _texture_manager.enableTexturing();
    } else {
        _texture_manager.disableTexturing();
    }
    
    // Apply quality settings
    if (_config.wireframe_mode) {
        OpenGLUtils::setupWireframe();
    } else {
        OpenGLUtils::setupFilled();
    }
    
    if (_config.face_culling) {
        OpenGLUtils::enableFaceCulling();
    } else {
        OpenGLUtils::disableFaceCulling();
    }
}

void zappy::graphics::renderers::Renderer3D::renderTile(int x, int z, const zappy::game::world::ZappyTile& tile)
{
    if (!_initialized) return;
    
    _terrain_renderer.renderTile(x, z, tile);
}

void zappy::graphics::renderers::Renderer3D::renderPlayer(const zappy::game::entities::ZappyPlayer& player)
{
    if (!_initialized) return;
    
    _entity_renderer.renderPlayer(player);
}

void zappy::graphics::renderers::Renderer3D::renderTileMap(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    if (!_initialized) return;
    
    _terrain_renderer.renderTileMap(world_map);
}

void zappy::graphics::renderers::Renderer3D::renderPlayers(const std::map<int, zappy::game::entities::ZappyPlayer>& players)
{
    if (!_initialized) return;
    
    _entity_renderer.renderPlayers(players);
}

void zappy::graphics::renderers::Renderer3D::renderText(const sf::Font& font, const std::string& text, 
    float x, float y, const sf::Color& color)
{
    (void)font; (void)text; (void)x; (void)y; (void)color;
}

void zappy::graphics::renderers::Renderer3D::printRenderingStatistics() const
{
    if (!_initialized) return;
    
    //std::cout << "📊 Rendering Statistics:" << std::endl;
    //std::cout << "  Textures: " << (_config.textures_enabled ? "ON" : "OFF") << std::endl;
    //std::cout << "  Wireframe: " << (_config.wireframe_mode ? "ON" : "OFF") << std::endl;

    _texture_manager.printTextureInfo();
    
    //std::cout << "  Terrain - Tiles rendered: " << _terrain_renderer.getTilesRendered() << std::endl;
    //std::cout << "  Entities - Players rendered: " << _entity_renderer.getPlayersRendered() << std::endl;
}