/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Creeper model implementation - Production version (always enabled)
*/

#include "zappy/game/entities/CreeperModel.hpp"
#include "zappy/graphics/utils/ModelLoader.hpp"
#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

zappy::game::entities::CreeperModel::CreeperModel() {
    //std::cout << "🐸 CreeperModel created" << std::endl;
    
    // Configure for production: always enabled with textures
    _config.enabled = true;
    _config.use_texture = true;
    _config.team_colors = true;
    _config.random_variations = false; // Keep consistent for production
}

zappy::game::entities::CreeperModel::~CreeperModel() {
    shutdown();
}

bool zappy::game::entities::CreeperModel::initialize(const std::string& model_path, zappy::graphics::utils::TextureManager* texture_manager) {
    if (_initialized) return true;
    
    //std::cout << "🐸 Initializing CreeperModel..." << std::endl;
    
    _model_path = model_path;
    _texture_manager = texture_manager;
    
    // Load 3D model
    _model = zappy::graphics::utils::ModelLoader::loadModel(model_path);
    
    if (!_model) {
        std::cerr << "❌ Failed to load Creeper model: " << model_path << std::endl;
        std::cerr << "💡 Make sure " << model_path << " is in your working directory" << std::endl;
        return false;
    }
    
    // Load texture if TextureManager is available
    if (_texture_manager && _config.use_texture) {
        _texture_loaded = loadCreeperTexture();
        if (_texture_loaded) {
            //std::cout << "✅ Creeper texture loaded successfully!" << std::endl;
        } else {
            //std::cout << "⚠️ Could not load Creeper texture, using colors only" << std::endl;
        }
    }
    
    _initialized = true;
    //std::cout << "✅ CreeperModel initialized successfully!" << std::endl;
    
    return true;
}

void zappy::game::entities::CreeperModel::shutdown() {
    if (!_initialized) return;
    
    //std::cout << "🔚 Shutting down CreeperModel..." << std::endl;
    _model = nullptr; // ModelLoader manages lifetime
    _texture_loaded = false;
    _team_variations_cache.clear();
    _initialized = false;
}

bool zappy::game::entities::CreeperModel::loadCreeperTexture() {
    if (!_texture_manager) return false;
    
    // Try to load Creeper texture
    std::vector<std::string> texture_paths = {
        _config.texture_path,
        "creeper.png",
        "./creeper.png",
        "textures/creeper.png",
        "assets/creeper.png",
        "assets/textures/creeper.png"
    };
    
    for (const auto& path : texture_paths) {
        if (_texture_manager->loadCustomTexture("creeper", path)) {
            //std::cout << "✅ Loaded Creeper texture: " << path << std::endl;
            return true;
        }
    }
    
    //std::cout << "⚠️ Could not find Creeper texture in any of the searched paths" << std::endl;
    return false;
}

void zappy::game::entities::CreeperModel::renderPlayer(const zappy::game::entities::ZappyPlayer& player, float delta_time) {
    (void)delta_time; // No animations in this version
    
    if (!isEnabled()) return;
    
    // Calculate rendering parameters
    MathUtils::Vector3f position = calculatePlayerPosition(player);
    MathUtils::Vector3f scale = calculateScale(player);
    sf::Color color = getPlayerColor(player);
    float rotation_y = getPlayerRotation(player);
    
    // Create rotation vector (Y rotation for facing direction)
    MathUtils::Vector3f rotation(0, rotation_y, 0);
    
    // Render the main Creeper model
    renderCreeperModel(position, rotation, scale, color);
}

void zappy::game::entities::CreeperModel::renderCreeperModel(const MathUtils::Vector3f& position, 
                                     const MathUtils::Vector3f& rotation,
                                     const MathUtils::Vector3f& scale,
                                     const sf::Color& color) {
    if (!_model) return;
    
    glPushMatrix();
    
    // Apply transformations
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.y, 0, 1, 0); // Y rotation for facing direction
    glRotatef(rotation.x, 1, 0, 0); // X rotation
    glRotatef(rotation.z, 0, 0, 1); // Z rotation
    glScalef(scale.x, scale.y, scale.z);
    
    // Setup texture and color
    if (isTextureEnabled()) {
        // Texture + color mode
        OpenGLUtils::enableTexturing();
        _texture_manager->bindCustomTexture("creeper");
        
        // Color modulates the texture
        OpenGLUtils::setColor(color);
        
        // Render model with texture
        _model->setWireframeMode(false);
        for (const auto& mesh : _model->getMeshes()) {
            mesh.render();
        }
        
        _texture_manager->unbindTexture();
    } else {
        // Color only mode
        OpenGLUtils::disableTexturing();
        OpenGLUtils::setColor(color);
        
        // Render model in solid color
        _model->setWireframeMode(false);
        for (const auto& mesh : _model->getMeshes()) {
            mesh.render();
        }
    }
    
    glPopMatrix();
}

MathUtils::Vector3f zappy::game::entities::CreeperModel::calculatePlayerPosition(const zappy::game::entities::ZappyPlayer& player) const {
    return MathUtils::Vector3f(
        static_cast<float>(player.x),
        0.5f + 1.0f, // Tile surface + half Creeper height
        static_cast<float>(player.y)
    );
}

MathUtils::Vector3f zappy::game::entities::CreeperModel::calculateScale(const ZappyPlayer& player) const {
    float base_scale = _config.scale;
    
    // Scale based on player level for visual distinction (simple formula)
    float level_scale = 1.0f + (player.level - 1) * 0.1f; // 10% bigger per level
    base_scale *= level_scale;
    
    return MathUtils::Vector3f(base_scale, base_scale, base_scale);
}

sf::Color zappy::game::entities::CreeperModel::getPlayerColor(const zappy::game::entities::ZappyPlayer& player) const {
    sf::Color final_color;
    
    if (_config.team_colors && player.color.a > 0) {
        // Apply team variation
        final_color = calculateTeamColorVariation(player);
        
        // Blend with base Creeper color if requested
        if (_config.texture_blend < 1.0f) {
            final_color = blendColors(final_color, _config.default_color, 1.0f - _config.texture_blend);
        }
    } else {
        final_color = _config.default_color;
    }
    
    // Apply random variations if enabled
    if (_config.random_variations) {
        final_color = applyRandomVariation(final_color, player.id);
    }
    
    return final_color;
}

sf::Color zappy::game::entities::CreeperModel::calculateTeamColorVariation(const zappy::game::entities::ZappyPlayer& player) const {
    TeamColorVariation variation = getTeamVariation(player.team);
    
    // Start with team base color
    sf::Color result = variation.base_color;
    
    // Apply team tint
    result = blendColors(result, variation.tint_color, _config.team_color_intensity);
    
    // Apply brightness and saturation factors
    result.r = static_cast<sf::Uint8>(std::min(255.0f, result.r * variation.brightness_factor));
    result.g = static_cast<sf::Uint8>(std::min(255.0f, result.g * variation.brightness_factor));
    result.b = static_cast<sf::Uint8>(std::min(255.0f, result.b * variation.brightness_factor));
    
    return result;
}

zappy::game::entities::CreeperModel::TeamColorVariation zappy::game::entities::CreeperModel::getTeamVariation(const std::string& team) const {
    // Check cache
    auto cache_it = _team_variations_cache.find(team);
    if (cache_it != _team_variations_cache.end()) {
        return cache_it->second;
    }
    
    // Create new variation based on team name
    TeamColorVariation variation;
    
    // Predefined variations by team
    if (team == "RedTeam" || team == "Red") {
        variation.base_color = sf::Color(200, 50, 50);
        variation.tint_color = sf::Color(255, 100, 100);
        variation.brightness_factor = 1.1f;
        variation.saturation_factor = 1.2f;
    } else if (team == "BlueTeam" || team == "Blue") {
        variation.base_color = sf::Color(50, 50, 200);
        variation.tint_color = sf::Color(100, 100, 255);
        variation.brightness_factor = 1.0f;
        variation.saturation_factor = 1.1f;
    } else if (team == "GreenTeam" || team == "Green") {
        variation.base_color = sf::Color(50, 200, 50);
        variation.tint_color = sf::Color(100, 255, 100);
        variation.brightness_factor = 1.05f;
        variation.saturation_factor = 1.0f;
    } else if (team == "YellowTeam" || team == "Yellow") {
        variation.base_color = sf::Color(200, 200, 50);
        variation.tint_color = sf::Color(255, 255, 100);
        variation.brightness_factor = 1.2f;
        variation.saturation_factor = 1.1f;
    } else if (team == "PurpleTeam" || team == "Purple") {
        variation.base_color = sf::Color(150, 50, 150);
        variation.tint_color = sf::Color(200, 100, 200);
        variation.brightness_factor = 0.9f;
        variation.saturation_factor = 1.3f;
    } else if (team == "CyanTeam" || team == "Cyan") {
        variation.base_color = sf::Color(50, 150, 150);
        variation.tint_color = sf::Color(100, 200, 200);
        variation.brightness_factor = 1.1f;
        variation.saturation_factor = 1.0f;
    } else {
        // Generic variation based on hash
        size_t hash = 0;
        for (char c : team) {
            hash = hash * 31 + c;
        }
        
        float hue = (hash % 360) / 360.0f;
        float sat = 0.7f + (hash % 30) / 100.0f;
        float val = 0.8f + (hash % 20) / 100.0f;
        
        // Simplified HSV to RGB conversion
        float r, g, b;
        int i = static_cast<int>(hue * 6);
        float f = hue * 6 - i;
        float p = val * (1 - sat);
        float q = val * (1 - f * sat);
        float t = val * (1 - (1 - f) * sat);
        
        switch (i % 6) {
            case 0: r = val; g = t; b = p; break;
            case 1: r = q; g = val; b = p; break;
            case 2: r = p; g = val; b = t; break;
            case 3: r = p; g = q; b = val; break;
            case 4: r = t; g = p; b = val; break;
            case 5: r = val; g = p; b = q; break;
            default: r = g = b = 0;
        }
        
        variation.base_color = sf::Color(
            static_cast<sf::Uint8>(r * 255),
            static_cast<sf::Uint8>(g * 255),
            static_cast<sf::Uint8>(b * 255)
        );
        variation.tint_color = sf::Color(
            static_cast<sf::Uint8>(std::min(255.0f, r * 255 * 1.3f)),
            static_cast<sf::Uint8>(std::min(255.0f, g * 255 * 1.3f)),
            static_cast<sf::Uint8>(std::min(255.0f, b * 255 * 1.3f))
        );
        variation.brightness_factor = 1.0f;
        variation.saturation_factor = 1.0f;
    }
    
    // Cache the variation
    _team_variations_cache[team] = variation;
    
    return variation;
}

sf::Color zappy::game::entities::CreeperModel::applyRandomVariation(const sf::Color& base_color, int player_id) const {
    if (_config.variation_intensity <= 0.0f) return base_color;
    
    // Use player ID as seed for consistent variations
    std::mt19937 rng(static_cast<unsigned int>(player_id + 12345));
    std::uniform_real_distribution<float> dist(-_config.variation_intensity, _config.variation_intensity);
    
    float r_variation = dist(rng);
    float g_variation = dist(rng);
    float b_variation = dist(rng);
    
    sf::Color result;
    result.r = static_cast<sf::Uint8>(std::max(0.0f, std::min(255.0f, base_color.r * (1.0f + r_variation))));
    result.g = static_cast<sf::Uint8>(std::max(0.0f, std::min(255.0f, base_color.g * (1.0f + g_variation))));
    result.b = static_cast<sf::Uint8>(std::max(0.0f, std::min(255.0f, base_color.b * (1.0f + b_variation))));
    result.a = base_color.a;
    
    return result;
}

sf::Color zappy::game::entities::CreeperModel::blendColors(const sf::Color& base, const sf::Color& tint, float intensity) const {
    intensity = std::max(0.0f, std::min(1.0f, intensity));
    
    sf::Color result;
    result.r = static_cast<sf::Uint8>(base.r * (1.0f - intensity) + tint.r * intensity);
    result.g = static_cast<sf::Uint8>(base.g * (1.0f - intensity) + tint.g * intensity);
    result.b = static_cast<sf::Uint8>(base.b * (1.0f - intensity) + tint.b * intensity);
    result.a = static_cast<sf::Uint8>(base.a * (1.0f - intensity) + tint.a * intensity);
    
    return result;
}

float zappy::game::entities::CreeperModel::getPlayerRotation(const zappy::game::entities::ZappyPlayer& player) const {
    // Convert player orientation to rotation angle
    switch (player.orientation) {
        case 1: return 0.0f;    // North
        case 2: return 90.0f;   // East
        case 3: return 180.0f;  // South
        case 4: return 270.0f;  // West
        default: return 0.0f;
    }
}