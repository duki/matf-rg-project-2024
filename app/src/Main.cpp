#include <GUIController.hpp>
#include <RoomController.hpp>
#include <engine/core/Engine.hpp>
#include <memory>

class MainApp final : public engine::core::App {
protected:
    void app_setup() override {
        auto room_controller = register_controller<RoomController>();
        auto gui_controller = register_controller<GUIController>();
        room_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        gui_controller->after(room_controller);
    }
};

int main(int argc, char **argv) {
    return std::make_unique<MainApp>()->run(argc, argv);
}
