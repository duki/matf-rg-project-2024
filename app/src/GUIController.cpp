#include <GUIController.hpp>
#include <RoomController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <imgui.h>

void GUIController::poll_events() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    // f1 toggle za prikaz/sakrivanje gui-a preko ugradjenog enable mehanizma
    if (platform->key(engine::platform::KeyId::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        set_enable(!is_enabled());
    }
}

void GUIController::draw() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto room = engine::core::Controller::get<RoomController>();

    // fps calc
    float dt = platform->dt();
    float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;
    float frame_time_ms = dt * 1000.0f;

    graphics->begin_gui();
    ImGui::Begin("devtools", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("perf");
    ImGui::Separator();
    ImGui::Text("fps: %.1f FPS", fps);
    ImGui::Text("frametime: %.2f ms", frame_time_ms);

    ImGui::Separator();

    ImGui::Text("info");
    ImGui::Separator();

    ImGui::Text("[x y z]: (%.2f, %.2f, %.2f)",
                camera->Position.x, camera->Position.y, camera->Position.z);
    ImGui::Text("[yaw pitch]: (%.1f/%.1f)",
                camera->Yaw, camera->Pitch);
    ImGui::Separator();
    ImGui::Text("controls:");
    ImGui::Text("press f1 to hide");
    ImGui::Text("press f2 to capture/release mouse");
    ImGui::Text("mouse captured: %s",
                !platform->is_cursor_enabled() ? "yes" : "no");

    ImGui::SliderFloat("movespeed", &camera->MovementSpeed, 1.0f, 15.0f);
    ImGui::Separator();

    // kontrole za sunce
    if (ImGui::CollapsingHeader("directional light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("enable", &room->m_dir_light.enabled);
        ImGui::SliderFloat3("heading", &room->m_dir_light.direction.x, -0.42f, 1.0f);
        ImGui::ColorEdit3("directional light color", &room->m_dir_light.color.x);
    }

    // kontrole za lampu
    if (ImGui::CollapsingHeader("point light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("enable", &room->m_point_light.enabled);
        ImGui::SliderFloat3("position", &room->m_point_light.position.x, -2.0f, 2.0f);
        ImGui::ColorEdit3("point light color", &room->m_point_light.color.x);
    }

    if (ImGui::CollapsingHeader("movie mode", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("[F3] to start");
        if (ImGui::Button("[F3]", ImVec2(150, 28))) {
            room->trigger_movie_mode();
        }
        ImGui::SameLine();
        if (ImGui::Button("reset mode", ImVec2(80, 28))) {
            room->reset_mode();
        }

        ImGui::Separator();
        ImGui::Text("status: %s", room->m_movie_mode_triggered ? "now playing: \"Chernobyl\"" : "await");
        if (room->m_movie_mode_triggered) {
            ImGui::ProgressBar(glm::clamp(room->m_event_timer / 3.5f, 0.0f, 1.0f));
            ImGui::Text("timer: %.2f s", room->m_event_timer);
            ImGui::Text("event A (1.5s - lights turn off): %s", room->m_event_a_done ? "done" : "await");
            ImGui::Text("event B (3.5s - tv glow on): %s", room->m_event_b_done ? "done" : "await");
        }
    }
    ImGui::End();
    graphics->end_gui();
}
