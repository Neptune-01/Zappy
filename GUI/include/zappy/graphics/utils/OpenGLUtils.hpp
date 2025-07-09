/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** OpenGL utility functions and state management (No Lighting)
*/

#pragma once

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <string>

namespace OpenGLUtils {
    
    void initializeOpenGL();
    void setupBasicState();
    void printOpenGLInfo();
    
    void enableDepthTest();
    void disableDepthTest();
    void enableTexturing();
    void disableTexturing();
    void enableBlending();
    void disableBlending();
    void enableFaceCulling();
    void disableFaceCulling();
    
    void setProjectionMode();
    void setModelViewMode();
    void pushMatrix();
    void popMatrix();
    void resetMatrices();
    void loadIdentity();
    
    void setColor(const sf::Color& color);
    void setColor(float r, float g, float b, float a = 1.0f);
    void setClearColor(const sf::Color& color);
    void setClearColor(float r, float g, float b, float a = 1.0f);
    void clearBuffers();
    void clearColorBuffer();
    void clearDepthBuffer();
    
    void setupCleanRendering();
    void setupSmoothShading();
    void setupWireframe();
    void setupFilled();
    
    GLuint createTexture(const sf::Image& image);
    void bindTexture(GLuint texture_id);
    void unbindTexture();
    void deleteTexture(GLuint texture_id);
    
    void renderQuad(float size = 1.0f);
    void renderCube(float size = 1.0f);
    void setNormal(float x, float y, float z);
    void setTexCoord(float u, float v);
    void setVertex(float x, float y, float z);
    
    void checkError(const std::string& operation = "");
    bool hasError();
    void enableDebug();
    void disableDebug();
    
    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
}