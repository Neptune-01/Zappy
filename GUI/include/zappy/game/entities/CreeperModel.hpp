/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Creeper model manager - Production version (always enabled)
*/

#ifndef CREEPERMODEL_HPP
#define CREEPERMODEL_HPP

#include <SFML/Graphics.hpp>
#include "zappy/graphics/utils/ModelLoader.hpp"
#include "zappy/graphics/utils/MathUtils.hpp"
#include "zappy/game/entities/Player.hpp"
#include "zappy/graphics/utils/TextureManager.hpp"


namespace zappy::game::entities {
class CreeperModel {
public:
    struct CreeperConfig {
        bool enabled = true;                    // Always enabled by default
        
        // Paramètres de rendu
        float scale = 0.5f;                     // Taille des creepers
        sf::Color default_color = sf::Color(0, 150, 0); // Vert Creeper de base
        bool team_colors = true;                // Variations selon les équipes
        
        // Paramètres de texture
        bool use_texture = true;                // Utiliser la texture
        std::string texture_path = "assets/textures/creeper.png"; // Chemin de la texture
        float texture_blend = 0.8f;            // Mélange texture/couleur (0.0 = couleur pure, 1.0 = texture pure)
        float team_color_intensity = 0.3f;     // Intensité de la couleur d'équipe (0.0 = pas de variation, 1.0 = forte variation)
        
        // Variations supplémentaires
        bool random_variations = false;        // Petites variations aléatoires (disabled for production)
        float variation_intensity = 0.1f;      // Intensité des variations (0.0 à 1.0)
    };

    // Structure pour les couleurs d'équipe avec variations
    struct TeamColorVariation {
        sf::Color base_color;
        sf::Color tint_color;      // Couleur de teinte
        float brightness_factor;   // Facteur de luminosité
        float saturation_factor;   // Facteur de saturation
    };

public:
    CreeperModel();
    ~CreeperModel();
    
    bool initialize(const std::string& model_path = "assets/models/Creeper.obj", 
                   zappy::graphics::utils::TextureManager* texture_manager = nullptr);
    void shutdown();
    
    // Main rendering method
    void renderPlayer(const zappy::game::entities::ZappyPlayer& player, float delta_time = 0.016f);
    
    // Contrôles de configuration
    void setEnabled(bool enabled) { _config.enabled = enabled; }
    void toggleEnabled() { _config.enabled = !_config.enabled; }
    void toggleTextures() { _config.use_texture = !_config.use_texture; }
    void toggleTeamColors() { _config.team_colors = !_config.team_colors; }
    void toggleRandomVariations() { _config.random_variations = !_config.random_variations; }
    
    // Setters pour les paramètres
    void setTextureBlend(float blend) { _config.texture_blend = std::max(0.0f, std::min(1.0f, blend)); }
    void setTeamColorIntensity(float intensity) { _config.team_color_intensity = std::max(0.0f, std::min(1.0f, intensity)); }
    void setVariationIntensity(float intensity) { _config.variation_intensity = std::max(0.0f, std::min(1.0f, intensity)); }
    
    // Status methods
    bool isEnabled() const { return _config.enabled && _model != nullptr; }
    bool isTextureEnabled() const { return _config.use_texture && _texture_loaded; }
    CreeperConfig getConfig() const { return _config; }
    
    void printStatus() const;

private:
    void renderCreeperModel(const MathUtils::Vector3f& position, 
                           const MathUtils::Vector3f& rotation,
                           const MathUtils::Vector3f& scale,
                           const sf::Color& color);
    
    MathUtils::Vector3f calculatePlayerPosition(const zappy::game::entities::ZappyPlayer& player) const;
    MathUtils::Vector3f calculateScale(const zappy::game::entities::ZappyPlayer& player) const;
    sf::Color getPlayerColor(const zappy::game::entities::ZappyPlayer& player) const;
    float getPlayerRotation(const zappy::game::entities::ZappyPlayer& player) const;
    
    // Méthodes pour la gestion des textures et couleurs
    bool loadCreeperTexture();
    sf::Color calculateTeamColorVariation(const zappy::game::entities::ZappyPlayer& player) const;
    sf::Color applyRandomVariation(const sf::Color& base_color, int player_id) const;
    sf::Color blendColors(const sf::Color& base, const sf::Color& tint, float intensity) const;
    TeamColorVariation getTeamVariation(const std::string& team) const;
    
    CreeperConfig _config;
    zappy::graphics::utils::Model* _model = nullptr;
    zappy::graphics::utils::TextureManager* _texture_manager = nullptr;
    bool _initialized = false;
    bool _texture_loaded = false;
    
    // Model path pour le rechargement
    std::string _model_path;
    
    // Cache des variations de couleur par équipe
    mutable std::map<std::string, TeamColorVariation> _team_variations_cache;
};
}

#endif /* !CREEPERMODEL_HPP */