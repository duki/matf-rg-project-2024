#ifndef ROOM_CONTROLLER_HPP
#define ROOM_CONTROLLER_HPP

#include <LightSettings.hpp>
#include <engine/core/Controller.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/platform/PlatformEventObserver.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>
#include <memory>

class MainPlatformEventObserver final : public engine::platform::PlatformEventObserver {
public:
    explicit MainPlatformEventObserver(engine::graphics::Camera *camera);

    void on_mouse_move(engine::platform::MousePosition position) override;
    void on_scroll(engine::platform::MousePosition position) override;

private:
    engine::graphics::Camera *m_camera{nullptr};
};

class RoomController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "RoomController";
    }
    void trigger_movie_mode();
    void reset_mode();

    DirLightSettings m_dir_light{};
    PointLightSettings m_point_light{};

    // stanje tajmera za filmski mod
    bool m_movie_mode_triggered{false};
    float m_event_timer{0.0f};
    bool m_event_a_done{false};
    bool m_event_b_done{false};
    float m_tv_glow{0.0f};
    // kraj stanja tajmera za filmski mod

protected:
    void initialize() override;
    void poll_events() override;
    void update() override;
    void draw() override;
    void end_draw() override;


private:
    void draw_model(const engine::resources::Shader *shader, engine::resources::Model *model, const glm::mat4 &transform, const glm::vec3 &color);
};

#endif//ROOM_CONTROLLER_HPP
