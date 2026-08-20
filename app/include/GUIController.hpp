#ifndef GUI_CONTROLLER_HPP
#define GUI_CONTROLLER_HPP

#include <engine/core/Controller.hpp>

class GUIController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "GUIController";
    }

protected:
    void poll_events() override;
    void draw() override;
};

#endif//GUI_CONTROLLER_HPP
