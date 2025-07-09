/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** OpenGL utility
*/

#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <iostream>

namespace OpenGLUtils {

void initializeOpenGL()
{
    //std::cout << "🔧 Initializing OpenGL..." << std::endl;
    
    setupBasicState();
    setupCleanRendering();
    setupSmoothShading();
    
    //std::cout << "✅ OpenGL initialized successfully!" << std::endl;
}

void setupBasicState()
{
    // Enable depth testing
    enableDepthTest();
    glDepthFunc(GL_LEQUAL);
    
    // Disable face culling to avoid artifacts
    disableFaceCulling();
    
    // Enable automatic normal normalization
    glEnable(GL_NORMALIZE);
    
    // Better depth buffer precision
    glClearDepth(1.0);
    
    // Sky blue clear color
    setClearColor(0.4f, 0.6f, 0.9f, 1.0f);
}

void setupCleanRendering()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
}

void setupSmoothShading()
{
    glShadeModel(GL_SMOOTH);
}

void setupWireframe()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void setupFilled()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void printOpenGLInfo()
{
    //std::cout << "📊 OpenGL Information:" << std::endl;
    //std::cout << "  Vendor: " << glGetString(GL_VENDOR) << std::endl;
    //std::cout << "  Renderer: " << glGetString(GL_RENDERER) << std::endl;
    //std::cout << "  Version: " << glGetString(GL_VERSION) << std::endl;
}

// State Management
void enableDepthTest() { glEnable(GL_DEPTH_TEST); }
void disableDepthTest() { glDisable(GL_DEPTH_TEST); }
void enableTexturing() { glEnable(GL_TEXTURE_2D); }
void disableTexturing() { glDisable(GL_TEXTURE_2D); }
void enableBlending() { glEnable(GL_BLEND); }
void disableBlending() { glDisable(GL_BLEND); }
void enableFaceCulling() { glEnable(GL_CULL_FACE); }
void disableFaceCulling() { glDisable(GL_CULL_FACE); }

// Matrix Operations
void setProjectionMode() { glMatrixMode(GL_PROJECTION); }
void setModelViewMode() { glMatrixMode(GL_MODELVIEW); }
void pushMatrix() { glPushMatrix(); }
void popMatrix() { glPopMatrix(); }
void loadIdentity() { glLoadIdentity(); }

void resetMatrices()
{
    setProjectionMode();
    loadIdentity();
    setModelViewMode();
    loadIdentity();
}

// Color and Clearing
void setColor(const sf::Color& color)
{
    setColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void setColor(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
}

void setClearColor(const sf::Color& color)
{
    setClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void setClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void clearBuffers()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void clearColorBuffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void clearDepthBuffer()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

// Texture Utilities
GLuint createTexture(const sf::Image& image)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    
    if (texture_id == 0) return 0;
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 image.getSize().x, image.getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                 image.getPixelsPtr());
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_id;
}

void bindTexture(GLuint texture_id)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

void unbindTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void deleteTexture(GLuint texture_id)
{
    glDeleteTextures(1, &texture_id);
}

// Basic Geometry
void renderQuad(float size)
{
    float half = size * 0.5f;
    glBegin(GL_QUADS);
    setTexCoord(0, 0); setVertex(-half, -half, 0);
    setTexCoord(1, 0); setVertex( half, -half, 0);
    setTexCoord(1, 1); setVertex( half,  half, 0);
    setTexCoord(0, 1); setVertex(-half,  half, 0);
    glEnd();
}

void renderCube(float size)
{
    float half = size * 0.5f;
    
    // Top face
    setNormal(0, 1, 0);
    glBegin(GL_QUADS);
    setVertex(-half, half, -half);
    setVertex( half, half, -half);
    setVertex( half, half,  half);
    setVertex(-half, half,  half);
    glEnd();
}

void setNormal(float x, float y, float z) { glNormal3f(x, y, z); }
void setTexCoord(float u, float v) { glTexCoord2f(u, v); }
void setVertex(float x, float y, float z) { glVertex3f(x, y, z); }

// Debug and Error Checking
void checkError(const std::string& operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "❌ OpenGL Error";
        if (!operation.empty()) {
            std::cerr << " in " << operation;
        }
        std::cerr << ": " << error << std::endl;
    }
}

bool hasError()
{
    return glGetError() != GL_NO_ERROR;
}

}