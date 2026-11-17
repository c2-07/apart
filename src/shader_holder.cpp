#include "shader_holder.h"
#include "constants.h"
#include "render_utils.h"
#include <utility>
#include <iostream>

namespace obby {

ShaderHolder::ShaderHolder()
    : m_shader(Shader{ 0, nullptr }),
      m_loaded(false),
      m_lightDirLoc(-1),
      m_lightColorLoc(-1),
      m_ambientColorLoc(-1),
      m_viewPosLoc(-1),
      m_fogColorLoc(-1),
      m_fogDensityLoc(-1) {}

ShaderHolder::ShaderHolder(const std::string& vsPath, const std::string& fsPath)
    : ShaderHolder() {
    if (IsWindowReady()) {
        m_shader = LoadShader(vsPath.c_str(), fsPath.c_str());
        if (m_shader.id > 0) {
            m_loaded = true;

            m_lightDirLoc = GetShaderLocation(m_shader, "lightDir");
            m_lightColorLoc = GetShaderLocation(m_shader, "lightColor");
            m_ambientColorLoc = GetShaderLocation(m_shader, "ambientColor");
            m_viewPosLoc = GetShaderLocation(m_shader, "viewPos");
            m_fogColorLoc = GetShaderLocation(m_shader, "fogColor");
            m_fogDensityLoc = GetShaderLocation(m_shader, "fogDensity");

            float lDir[3] = { Constants::LIGHT_DIR_X, Constants::LIGHT_DIR_Y, Constants::LIGHT_DIR_Z };
            float lCol[4] = { Constants::SUN_LIGHT_INTENSITY, Constants::SUN_LIGHT_INTENSITY, Constants::SUN_LIGHT_INTENSITY, 1.0f };
            float ambCol[4] = { Constants::AMBIENT_LIGHT_INTENSITY, Constants::AMBIENT_LIGHT_INTENSITY, Constants::AMBIENT_LIGHT_INTENSITY, 1.0f };
            float fogDens = Constants::DEFAULT_FOG_DENSITY;

            SetShaderValue(m_shader, m_lightDirLoc, lDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(m_shader, m_lightColorLoc, lCol, SHADER_UNIFORM_VEC4);
            SetShaderValue(m_shader, m_ambientColorLoc, ambCol, SHADER_UNIFORM_VEC4);
            SetShaderValue(m_shader, m_fogDensityLoc, &fogDens, SHADER_UNIFORM_FLOAT);
        }
    }
}

ShaderHolder::~ShaderHolder() {
    if (m_loaded && m_shader.id > 0) {
        UnloadShader(m_shader);
        m_loaded = false;
        m_shader.id = 0;
    }
}

ShaderHolder::ShaderHolder(ShaderHolder&& other) noexcept
    : m_shader(other.m_shader),
      m_loaded(other.m_loaded),
      m_lightDirLoc(other.m_lightDirLoc),
      m_lightColorLoc(other.m_lightColorLoc),
      m_ambientColorLoc(other.m_ambientColorLoc),
      m_viewPosLoc(other.m_viewPosLoc),
      m_fogColorLoc(other.m_fogColorLoc),
      m_fogDensityLoc(other.m_fogDensityLoc) {
    other.m_loaded = false;
    other.m_shader.id = 0;
}

ShaderHolder& ShaderHolder::operator=(ShaderHolder&& other) noexcept {
    if (this != &other) {
        if (m_loaded && m_shader.id > 0) {
            UnloadShader(m_shader);
        }
        m_shader = other.m_shader;
        m_loaded = other.m_loaded;
        m_lightDirLoc = other.m_lightDirLoc;
        m_lightColorLoc = other.m_lightColorLoc;
        m_ambientColorLoc = other.m_ambientColorLoc;
        m_viewPosLoc = other.m_viewPosLoc;
        m_fogColorLoc = other.m_fogColorLoc;
        m_fogDensityLoc = other.m_fogDensityLoc;

        other.m_loaded = false;
        other.m_shader.id = 0;
    }
    return *this;
}

void ShaderHolder::applyLighting(Vector3 eyePos, int worldId) const {
    if (!m_loaded) return;
    WorldPalette pal = GetWorldPalette(worldId);

    float lCol[4] = {
        pal.lightColor.r / 255.0f,
        pal.lightColor.g / 255.0f,
        pal.lightColor.b / 255.0f,
        1.0f
    };
    float aCol[4] = {
        pal.ambientColor.r / 255.0f,
        pal.ambientColor.g / 255.0f,
        pal.ambientColor.b / 255.0f,
        1.0f
    };
    float camPos[3] = { eyePos.x, eyePos.y, eyePos.z };
    float horizonCol[4] = {
        pal.fogColor.r / 255.0f,
        pal.fogColor.g / 255.0f,
        pal.fogColor.b / 255.0f,
        1.0f
    };

    SetShaderValue(m_shader, m_lightColorLoc, lCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(m_shader, m_ambientColorLoc, aCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(m_shader, m_viewPosLoc, camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(m_shader, m_fogColorLoc, horizonCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(m_shader, m_fogDensityLoc, &pal.fogDensity, SHADER_UNIFORM_FLOAT);

    begin();
}

void ShaderHolder::begin() const {
    if (m_loaded) {
        BeginShaderMode(m_shader);
    }
}

void ShaderHolder::end() const {
    if (m_loaded) {
        EndShaderMode();
    }
}

} // namespace obby
