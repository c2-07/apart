#pragma once
#include "raylib.h"
#include <string>

namespace obby {

/**
 * @brief RAII Resource holder managing the lifecycle and uniform state of a raylib Shader.
 * Demonstrates:
 * - RAII (Resource Acquisition Is Initialization)
 * - Automatic resource deallocation upon destruction (UnloadShader)
 * - Deleted copy constructor / copy assignment
 * - Move semantics (move constructor and move assignment)
 */
class ShaderHolder {
private:
    Shader m_shader;
    bool m_loaded;
    int m_lightDirLoc;
    int m_lightColorLoc;
    int m_ambientColorLoc;
    int m_viewPosLoc;
    int m_fogColorLoc;
    int m_fogDensityLoc;

public:
    ShaderHolder();
    ShaderHolder(const std::string& vsPath, const std::string& fsPath);
    ~ShaderHolder();

    // RAII semantics: Non-copyable, movable
    ShaderHolder(const ShaderHolder&) = delete;
    ShaderHolder& operator=(const ShaderHolder&) = delete;
    ShaderHolder(ShaderHolder&& other) noexcept;
    ShaderHolder& operator=(ShaderHolder&& other) noexcept;

    bool isLoaded() const { return m_loaded; }
    const Shader& getShader() const { return m_shader; }

    void applyLighting(Vector3 eyePos, int worldId) const;
    void begin() const;
    void end() const;
};

} // namespace obby

using ShaderHolder = obby::ShaderHolder;
