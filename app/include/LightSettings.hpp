#ifndef LIGHT_SETTINGS_HPP
#define LIGHT_SETTINGS_HPP

#include <glm/glm.hpp>

struct DirLightSettings {
    glm::vec3 direction{-0.5f, -1.0f, -0.6f};
    glm::vec3 color{0.45f, 0.40f, 0.35f};
    bool enabled{true};
};

struct PointLightSettings {
    glm::vec3 position{1.4f, 1.4f, 0.6f};
    glm::vec3 color{0.70f, 0.55f, 0.25f};
    bool enabled{true};
};

#endif//LIGHT_SETTINGS_HPP
