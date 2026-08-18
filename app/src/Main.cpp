#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <spdlog/spdlog.h>

class RoomController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "RoomController";
    }

protected:
    void initialize() override {
        engine::graphics::OpenGL::enable_depth_testing();

        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

        // settuj cameru na pocetku scene
        auto camera = graphics->camera();
        camera->Position = glm::vec3(0.0f, 1.8f, 2.8f);
        camera->Yaw = -90.0f;
        camera->Pitch = -12.0f;

        spdlog::info("RoomController initialized.");
    }

    void update() override {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto camera = graphics->camera();
        float dt = platform->dt();


        // basic opcije za pomeranje u svih 6 smerova kao u elite dangerous
        if (platform->key(engine::platform::KeyId::KEY_W).is_down()) {
            camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_S).is_down()) {
            camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_A).is_down()) {
            camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_D).is_down()) {
            camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_E).is_down()) {
            camera->move_camera(engine::graphics::Camera::Movement::UP, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
            camera->move_camera(engine::graphics::Camera::Movement::DOWN, dt);
        }
    }

    void draw() override {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        engine::graphics::OpenGL::clear_buffers();
        platform->swap_buffers();
    }
};

class MainApp final : public engine::core::App {
protected:
    void app_setup() override {
        auto room_controller = register_controller<RoomController>();
        room_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    }
};

int main(int argc, char **argv) {
    return std::make_unique<MainApp>()->run(argc, argv);
}
