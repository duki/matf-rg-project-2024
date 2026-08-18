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
        spdlog::info("RoomController initialized.");
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
