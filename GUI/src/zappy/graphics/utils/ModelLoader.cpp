/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** OBJ Model loader
*/

#include "zappy/graphics/utils/ModelLoader.hpp"
#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Static member definition
std::map<std::string, zappy::graphics::utils::Model*> zappy::graphics::utils::ModelLoader::_loaded_models;

bool zappy::graphics::utils::Model::loadFromFile(const std::string& filepath) {
    //std::cout << "🐸 Loading Creeper model: " << filepath << std::endl;
    
    if (!parseOBJ(filepath)) {
        std::cerr << "❌ Failed to parse OBJ file: " << filepath << std::endl;
        return false;
    }
    
    // Process all meshes
    for (auto& mesh : _meshes) {
        processMesh(mesh);
    }
    
    calculateBoundingBox();
    _loaded = true;
    
    //std::cout << "✅ Creeper model loaded successfully!" << std::endl;
    //std::cout << "   Meshes: " << _meshes.size() << std::endl;
    //std::cout << "   Vertices: " << _obj_vertices.size() << std::endl;
    //std::cout << "   Normals: " << _obj_normals.size() << std::endl;
    //std::cout << "   TexCoords: " << _obj_texcoords.size() << std::endl;
    //std::cout << "   Faces: ";
    size_t total_faces = 0;
    for (const auto& mesh : _meshes) {
        total_faces += mesh.faces.size();
    }
    //std::cout << total_faces << std::endl;
    
    return true;
}

bool zappy::graphics::utils::Model::parseOBJ(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "❌ Cannot open OBJ file: " << filepath << std::endl;
        return false;
    }
    
    // Clear previous data
    _obj_vertices.clear();
    _obj_normals.clear();
    _obj_texcoords.clear();
    _meshes.clear();
    
    // Create default mesh
    Mesh current_mesh;
    current_mesh.name = "CreeperMesh";
    current_mesh.material.name = "CreeperMaterial";
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // Vertex position
            float x, y, z;
            if (iss >> x >> y >> z) {
                _obj_vertices.emplace_back(x, y, z);
            }
        }
        else if (prefix == "vn") {
            // Vertex normal
            float x, y, z;
            if (iss >> x >> y >> z) {
                _obj_normals.emplace_back(x, y, z);
            }
        }
        else if (prefix == "vt") {
            // Texture coordinate
            float u, v;
            if (iss >> u >> v) {
                _obj_texcoords.emplace_back(u, v);
            }
        }
        else if (prefix == "f") {
            // Face definition
            Face face;
            std::string vertex_data;
            
            while (iss >> vertex_data) {
                // Parse vertex/texture/normal indices (format: v/vt/vn or v//vn or v)
                std::vector<std::string> indices;
                std::stringstream ss(vertex_data);
                std::string item;
                
                while (std::getline(ss, item, '/')) {
                    indices.push_back(item);
                }
                
                if (!indices.empty() && !indices[0].empty()) {
                    int vertex_idx = std::stoi(indices[0]) - 1; // OBJ indices are 1-based
                    if (vertex_idx >= 0 && vertex_idx < static_cast<int>(_obj_vertices.size())) {
                        face.vertex_indices.push_back(vertex_idx);
                    }
                }
                
                // Texture coordinates (optional)
                if (indices.size() > 1 && !indices[1].empty()) {
                    int texcoord_idx = std::stoi(indices[1]) - 1;
                    if (texcoord_idx >= 0 && texcoord_idx < static_cast<int>(_obj_texcoords.size())) {
                        face.texcoord_indices.push_back(texcoord_idx);
                    }
                }
                
                // Normal indices (optional)
                if (indices.size() > 2 && !indices[2].empty()) {
                    int normal_idx = std::stoi(indices[2]) - 1;
                    if (normal_idx >= 0 && normal_idx < static_cast<int>(_obj_normals.size())) {
                        face.normal_indices.push_back(normal_idx);
                    }
                }
            }
            
            // Only add faces with valid vertices
            if (face.vertex_indices.size() >= 3) {
                current_mesh.faces.push_back(face);
            }
        }
        else if (prefix == "o" || prefix == "g") {
            // Object or group - start new mesh if current one has data
            if (!current_mesh.faces.empty()) {
                _meshes.push_back(current_mesh);
                current_mesh = Mesh();
                current_mesh.name = "CreeperMesh_" + std::to_string(_meshes.size());
            }
        }
    }
    
    // Add final mesh if it has data
    if (!current_mesh.faces.empty()) {
        _meshes.push_back(current_mesh);
    }
    
    // If no meshes were created, create a default one
    if (_meshes.empty()) {
        _meshes.push_back(current_mesh);
    }
    
    file.close();
    
    //std::cout << "📊 OBJ parsing complete:" << std::endl;
    //std::cout << "   Raw vertices: " << _obj_vertices.size() << std::endl;
    //std::cout << "   Raw normals: " << _obj_normals.size() << std::endl;
    //std::cout << "   Raw texcoords: " << _obj_texcoords.size() << std::endl;
    //std::cout << "   Meshes created: " << _meshes.size() << std::endl;
    
    return !_obj_vertices.empty();
}

void zappy::graphics::utils::Model::processMesh(Mesh& mesh) {
    // Convert faces to triangulated vertices
    mesh.vertices.clear();
    
    for (const auto& face : mesh.faces) {
        if (face.vertex_indices.size() < 3) continue;
        
        for (size_t i = 2; i < face.vertex_indices.size(); ++i) {
            // Triangle: indices 0, i-1, i
            for (int tri_idx : {0, static_cast<int>(i-1), static_cast<int>(i)}) {
                if (tri_idx < 0 || tri_idx >= static_cast<int>(face.vertex_indices.size())) continue;
                
                Vertex vertex;
                
                // Position
                int pos_idx = face.vertex_indices[tri_idx];
                if (pos_idx >= 0 && pos_idx < static_cast<int>(_obj_vertices.size())) {
                    vertex.position = _obj_vertices[pos_idx];
                }
                
                // Normal
                if (tri_idx < static_cast<int>(face.normal_indices.size())) {
                    int norm_idx = face.normal_indices[tri_idx];
                    if (norm_idx >= 0 && norm_idx < static_cast<int>(_obj_normals.size())) {
                        vertex.normal = _obj_normals[norm_idx];
                    }
                } else {
                    if (mesh.vertices.size() >= 2) {
                        MathUtils::Vector3f v1 = mesh.vertices[mesh.vertices.size()-1].position - mesh.vertices[mesh.vertices.size()-2].position;
                        MathUtils::Vector3f v2 = vertex.position - mesh.vertices[mesh.vertices.size()-2].position;
                        vertex.normal = v1.cross(v2).normalized();
                    } else {
                        vertex.normal = MathUtils::Vector3f(0, 1, 0);
                    }
                }
                
                // Texture coordinates
                if (tri_idx < static_cast<int>(face.texcoord_indices.size())) {
                    int tex_idx = face.texcoord_indices[tri_idx];
                    if (tex_idx >= 0 && tex_idx < static_cast<int>(_obj_texcoords.size())) {
                        vertex.texCoords = _obj_texcoords[tex_idx];
                    }
                } else {
                    // Coordonnées par défaut si pas spécifiées
                    vertex.texCoords = sf::Vector2f(0.0f, 0.0f);
                }
                
                mesh.vertices.push_back(vertex);
            }
        }
    }
    
    //std::cout << "📦 Mesh '" << mesh.name << "' processed: " << mesh.vertices.size() << " vertices" << std::endl;
}

void zappy::graphics::utils::Model::render(const MathUtils::Vector3f& position, 
    const MathUtils::Vector3f& rotation,
    const MathUtils::Vector3f& scale) const {
    if (!_loaded || _meshes.empty()) return;
    
    glPushMatrix();
    
    // Apply transformations
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.y, 0, 1, 0); // Y rotation for facing direction
    glRotatef(rotation.x, 1, 0, 0); // X rotation
    glRotatef(rotation.z, 0, 0, 1); // Z rotation
    glScalef(scale.x, scale.y, scale.z);
    
    // Set color if override is specified
    if (_override_color.a > 0) {
        OpenGLUtils::setColor(_override_color);
    }
    
    // Set wireframe mode
    if (_wireframe_mode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Render all meshes
    for (const auto& mesh : _meshes) {
        mesh.render();
    }
    
    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glPopMatrix();
}

void Mesh::render() const {
    if (vertices.empty()) return;
    
    bool has_texcoords = false;
    for (const auto& vertex : vertices) {
        if (vertex.texCoords.x != 0.0f || vertex.texCoords.y != 0.0f) {
            has_texcoords = true;
            break;
        }
    }
    
    glBegin(GL_TRIANGLES);
    
    for (const auto& vertex : vertices) {
        glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
        
        if (has_texcoords) {
            glTexCoord2f(vertex.texCoords.x, vertex.texCoords.y);
        }
        
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
    
    glEnd();
}

void zappy::graphics::utils::Model::calculateBoundingBox() {
    if (_obj_vertices.empty()) {
        _bbox_min = _bbox_max = MathUtils::Vector3f(0, 0, 0);
        return;
    }
    
    _bbox_min = _bbox_max = _obj_vertices[0];
    
    for (const auto& vertex : _obj_vertices) {
        _bbox_min.x = std::min(_bbox_min.x, vertex.x);
        _bbox_min.y = std::min(_bbox_min.y, vertex.y);
        _bbox_min.z = std::min(_bbox_min.z, vertex.z);
        
        _bbox_max.x = std::max(_bbox_max.x, vertex.x);
        _bbox_max.y = std::max(_bbox_max.y, vertex.y);
        _bbox_max.z = std::max(_bbox_max.z, vertex.z);
    }
    
    //std::cout << "📏 Bounding box: (" << _bbox_min.x << "," << _bbox_min.y << "," << _bbox_min.z 
        //<< ") to (" << _bbox_max.x << "," << _bbox_max.y << "," << _bbox_max.z << ")" << std::endl;
}

void zappy::graphics::utils::Model::setColor(const sf::Color& color) {
    _override_color = color;
}

const std::vector<Mesh>& zappy::graphics::utils::Model::getMeshes() const {
    return _meshes;
}

// ModelLoader static methods
zappy::graphics::utils::Model* zappy::graphics::utils::ModelLoader::loadModel(const std::string& filepath) {
    auto it = _loaded_models.find(filepath);
    if (it != _loaded_models.end()) {
        return it->second;
    }
    
    zappy::graphics::utils::Model* model = new zappy::graphics::utils::Model();
    if (model->loadFromFile(filepath)) {
        _loaded_models[filepath] = model;
        //std::cout << "📚 Model cached: " << filepath << std::endl;
        return model;
    }
    
    delete model;
    return nullptr;
}

void zappy::graphics::utils::ModelLoader::unloadModel(const std::string& filepath) {
    auto it = _loaded_models.find(filepath);
    if (it != _loaded_models.end()) {
        delete it->second;
        _loaded_models.erase(it);
        //std::cout << "🗑️ Model unloaded: " << filepath << std::endl;
    }
}

void zappy::graphics::utils::ModelLoader::unloadAllModels() {
    for (auto& pair : _loaded_models) {
        delete pair.second;
    }
    _loaded_models.clear();
    //std::cout << "🗑️ All models unloaded" << std::endl;
}

zappy::graphics::utils::Model* zappy::graphics::utils::ModelLoader::getModel(const std::string& filepath) {
    auto it = _loaded_models.find(filepath);
    return (it != _loaded_models.end()) ? it->second : nullptr;
}

bool zappy::graphics::utils::ModelLoader::isModelLoaded(const std::string& filepath) {
    return _loaded_models.find(filepath) != _loaded_models.end();
}

void zappy::graphics::utils::ModelLoader::printLoadedModels() {
    //std::cout << "\n📚 === LOADED MODELS ===" << std::endl;
    //for (const auto& pair : _loaded_models) {
        //std::cout << "  " << pair.first << std::endl;
    //}
    //std::cout << "Total: " << _loaded_models.size() << " models" << std::endl;
}