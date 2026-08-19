#include <RoomController.hpp>
#include <engine/core/Engine.hpp>
#include <memory>

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
