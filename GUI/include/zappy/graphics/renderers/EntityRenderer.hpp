/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Entity rendering system avec support texture Creeper avancé
*/

#ifndef ENTITYRENDERER_HPP
#define ENTITYRENDERER_HPP

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include "zappy/game/entities/Player.hpp"
#include "zappy/graphics/utils/MathUtils.hpp"
#include "zappy/game/entities/CreeperModel.hpp"
#include "zappy/graphics/utils/TextureManager.hpp"


namespace zappy::graphics::renderers {

class EntityRenderer {
public:
    struct EntityConfig {
        float player_size = 0.6f;
        float player_height = 1.5f;
        float player_y_offset = 1.01f;
        
        float indicator_size = 0.2f;
        float indicator_height = 0.4f;
        float indicator_distance = 0.3f;
        sf::Color indicator_color = sf::Color(255, 255, 100);
        
        bool smooth_movement = false;
        bool level_scaling = false;
        float brightness_factor = 1.0f;
    };

public:
    EntityRenderer();
    ~EntityRenderer();
    
    void initialize(zappy::graphics::utils::TextureManager* texture_manager = nullptr);
    void setConfig(const EntityConfig& config) { _config = config; }
    EntityConfig getConfig() const { return _config; }
    
    // Core rendering methods
    void renderPlayer(const zappy::game::entities::ZappyPlayer& player);
    void renderDirectionIndicator(const zappy::game::entities::ZappyPlayer& player);
    
    void renderPlayers(const std::map<int, zappy::game::entities::ZappyPlayer>& players);
    void renderPlayersRange(const std::map<int, zappy::game::entities::ZappyPlayer>& players, 
                           int start_id, int end_id);
    
    // Traditional cube rendering methods
    void renderCube(float x, float y, float z, float size, float height, 
                   const sf::Color& color);
    void renderColoredCube(const MathUtils::Vector3f& position, 
                          const MathUtils::Vector3f& size,
                          const sf::Color& color);
    
    // Contrôles Creeper de base
    void toggleCreeperModel();
    
    // Nouveaux contrôles Creeper avancés
    void toggleCreeperTextures();
    void toggleTeamColors();
    void toggleRandomVariations();
    
    // Ajustements fins
    void adjustTextureBlend(float delta);        // Augmente/diminue le mélange texture/couleur
    void adjustTeamColorIntensity(float delta);  // Augmente/diminue l'intensité des couleurs d'équipe
    void adjustVariationIntensity(float delta);  // Augmente/diminue les variations aléatoires
    
    // Status and info methods
    void printCreeperStatus() const;
    bool isCreeperEnabled() const;
    bool isCreeperTextureEnabled() const;
    void setDeltaTime(float delta_time);
    
    // Statistics
    size_t getPlayersRendered() const;
    void resetStatistics();

private:
    void renderPlayerTraditional(const zappy::game::entities::ZappyPlayer& player);
    void renderPlayerGeometry(float x, float y, float z, 
        float size, float height, 
        const sf::Color& color);
    void renderIndicatorGeometry(float x, float y, float z,
        float size, float height,
        const sf::Color& color);
    MathUtils::Vector3f getPlayerPosition(const zappy::game::entities::ZappyPlayer& player) const;
    MathUtils::Vector3f getIndicatorPosition(const zappy::game::entities::ZappyPlayer& player) const;
    void applyBrightness(sf::Color& color) const;
    
    EntityConfig _config;
    
    bool _initialized = false;
    size_t _players_rendered = 0;
    
    // Support Creeper avec texture
    zappy::game::entities::CreeperModel* _creeper_model = nullptr;
    float _last_delta_time = 0.016f; // ~60 FPS default
};
}
#endif /* !ENTITYRENDERER_HPP */