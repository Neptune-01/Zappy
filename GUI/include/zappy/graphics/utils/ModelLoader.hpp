/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** OBJ Model loader for 3D models like Creeper (OpenGL Legacy compatible)
*/

#ifndef MODELOADER_HPP
#define MODELOADER_HPP

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "zappy/graphics/utils/MathUtils.hpp"

struct Vertex {
    MathUtils::Vector3f position;
    MathUtils::Vector3f normal;
    sf::Vector2f texCoords;
    
    Vertex() = default;
    Vertex(const MathUtils::Vector3f& pos, const MathUtils::Vector3f& norm, const sf::Vector2f& tex)
        : position(pos), normal(norm), texCoords(tex) {}
};

struct Face {
    std::vector<unsigned int> vertex_indices;
    std::vector<unsigned int> normal_indices;
    std::vector<unsigned int> texcoord_indices;
};

struct Material {
    std::string name;
    sf::Color diffuse_color = sf::Color::White;
    sf::Color specular_color = sf::Color::White;
    float shininess = 32.0f;
    std::string texture_path;
};

struct Mesh {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    Material material;
    
    void render() const;
};

namespace zappy::graphics::utils {
class Model {
public:
    Model() = default;
    ~Model() = default;
    
    bool loadFromFile(const std::string& filepath);
    void render(const MathUtils::Vector3f& position, 
               const MathUtils::Vector3f& rotation = MathUtils::Vector3f(0, 0, 0),
               const MathUtils::Vector3f& scale = MathUtils::Vector3f(1, 1, 1)) const;
    
    void setColor(const sf::Color& color);
    void setWireframeMode(bool wireframe) { _wireframe_mode = wireframe; }
    bool isWireframeMode() const { return _wireframe_mode; }
    
    bool isLoaded() const { return _loaded; }
    size_t getMeshCount() const { return _meshes.size(); }
    
    MathUtils::Vector3f getBoundingBoxMin() const { return _bbox_min; }
    MathUtils::Vector3f getBoundingBoxMax() const { return _bbox_max; }
    
    // AJOUT: Accès aux meshes pour le rendu avancé
    const std::vector<Mesh>& getMeshes() const;

private:
    bool parseOBJ(const std::string& filepath);
    void calculateBoundingBox();
    void processMesh(Mesh& mesh);  // ← CETTE DÉCLARATION EST IMPORTANTE
    
    std::vector<Mesh> _meshes;
    std::map<std::string, Material> _materials;
    
    // Raw data from OBJ file
    std::vector<MathUtils::Vector3f> _obj_vertices;
    std::vector<MathUtils::Vector3f> _obj_normals;
    std::vector<sf::Vector2f> _obj_texcoords;
    
    MathUtils::Vector3f _bbox_min, _bbox_max;
    sf::Color _override_color = sf::Color::Transparent;
    bool _wireframe_mode = false;
    bool _loaded = false;
};

class ModelLoader {
public:
    static Model* loadModel(const std::string& filepath);
    static void unloadModel(const std::string& filepath);
    static void unloadAllModels();
    
    static Model* getModel(const std::string& filepath);
    static bool isModelLoaded(const std::string& filepath);
    
    static void printLoadedModels();

private:
    static std::map<std::string, Model*> _loaded_models;
};
}
#endif /* !MODELOADER_HPP */