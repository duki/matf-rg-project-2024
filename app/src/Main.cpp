#include <LightSettings.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

class MainPlatformEventObserver final : public engine::platform::PlatformEventObserver {
public:
    explicit MainPlatformEventObserver(engine::graphics::Camera *camera, bool *mouse_captured)
        : m_camera(camera)
        , m_mouse_captured(mouse_captured) {}


    void on_mouse_move(engine::platform::MousePosition position) override {
        if (*m_mouse_captured) {
            m_camera->rotate_camera(position.dx, position.dy);
        }
    }
    void on_scroll(engine::platform::MousePosition position) override {
        m_camera->zoom(position.scroll);
    }

private:
    engine::graphics::Camera *m_camera{nullptr};
    bool *m_mouse_captured{nullptr};
};

class RoomController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "RoomController";
    }

protected:
    void initialize() override {


        engine::graphics::OpenGL::enable_depth_testing();

        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();


        // settuj cameru na pocetku scene
        auto camera = graphics->camera();
        camera->Position = glm::vec3(0.0f, 1.8f, 2.8f);
        camera->Yaw = -90.0f;
        camera->Pitch = -12.0f;


        // settuj kradju cursora na pocetku scene i registruj observer
        platform->set_enable_cursor(!m_mouse_captured);
        auto observer = std::make_unique<MainPlatformEventObserver>(camera, &m_mouse_captured);
        platform->register_platform_event_observer(std::move(observer));

        spdlog::info("RoomController initialized.");
    }


    void poll_events() override {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

        // f2 toggle za kradju kursora
        if (platform->key(engine::platform::KeyId::KEY_F2).state() == engine::platform::Key::State::JustPressed) {
            m_mouse_captured = !m_mouse_captured;
            platform->set_enable_cursor(!m_mouse_captured);
        }

        // f1 toggle za prikaz gui-a
        if (platform->key(engine::platform::KeyId::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
            m_draw_gui = !m_draw_gui;
        }
    }

    void update() override {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto camera = graphics->camera();
        float dt = platform->dt();


        if (m_mouse_captured) {
            // basic kbd tasteri se koriste za kontrole pomeranja kamere
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
        }
    }

    void draw() override {
        auto rc = engine::core::Controller::get<engine::resources::ResourcesController>();
        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        auto camera = graphics->camera();


        engine::graphics::OpenGL::clear_buffers();


        auto shader = rc->shader("starter");
        if (shader != nullptr) {
            shader->use();
            // globalne matrice
            glm::mat4 projection = graphics->projection_matrix();
            glm::mat4 view = camera->view_matrix();
            shader->set_mat4("projection", projection);
            shader->set_mat4("view", view);
            shader->set_vec3("viewPos", camera->Position);

            // default boja je 0.0f
            shader->set_vec3("dirLightDir", m_dir_light.direction);
            shader->set_vec3("dirLightColor", m_dir_light.enabled ? m_dir_light.color : glm::vec3(0.0f));

            shader->set_vec3("pointLightPos", m_point_light.position);
            shader->set_vec3("pointLightColor", m_point_light.enabled ? m_point_light.color : glm::vec3(0.0f));

            // crtanje poda
            auto floor_model = rc->model("floor");
            for (int x = -2; x <= 2; x++) {
                for (int z = -2; z <= 2; z++) {
                    glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(x * 1.0f + 0.0f, 0.0f, z * 1.0f + 0.0f));
                    draw_model(shader, floor_model, model_mat, glm::vec3(0.741f, 0.580f, 0.463f));
                }
            }
            // crtanje zida
            auto wall_model = rc->model("wall");
            auto wall_window_model = rc->model("wall_window");

            for (int x = -2; x <= 2; x++) {
                glm::mat4 wall_mat = glm::translate(glm::mat4(1.0f), glm::vec3(x * 1.0f, 0.0f, -2.05f));
                if (x == 0) {
                    // prozor na punom zidu
                    draw_model(shader, wall_window_model, wall_mat, glm::vec3(0.92f, 0.90f, 0.86f));
                } else {
                    // pun zid
                    draw_model(shader, wall_model, wall_mat, glm::vec3(0.92f, 0.90f, 0.86f));
                }
            }
            // levi bocni zid
            for (int z = -2; z <= 2; z++) {
                glm::mat4 wall_left = glm::translate(glm::mat4(1.0f), glm::vec3(-3.05f, 0.0f, z * 1.0f));
                wall_left = glm::rotate(wall_left, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                draw_model(shader, wall_model, wall_left, glm::vec3(0.85f, 0.88f, 0.90f));
            }
            // desni bocni zid
            for (int z = -2; z <= 2; z++) {
                glm::mat4 wall_right = glm::translate(glm::mat4(1.0f), glm::vec3(2.05f, 0.0f, +1.0f + z * 1.0f));
                wall_right = glm::rotate(wall_right, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                draw_model(shader, wall_model, wall_right, glm::vec3(0.85f, 0.88f, 0.90f));
            }
            // zadnja strana zida
            for (int x = -2; x <= 2; x++) {
                glm::mat4 wall_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f + x * 1.0f, 0.0f, 3.05f));
                wall_back = glm::rotate(wall_back, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                draw_model(shader, wall_model, wall_back, glm::vec3(0.85f, 0.88f, 0.90f));
            }
        }


        // centralni tepih
        auto rug_model = rc->model("rug");
        glm::mat4 rug_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.055f, -0.5f));
        rug_mat = glm::scale(rug_mat, glm::vec3(1.3f, 1.0f, 1.3f));
        draw_model(shader, rug_model, rug_mat, glm::vec3(0.82f, 0.32f, 0.25f));

        // stocic za kafu (na sredini sobe)
        auto coffee_table = rc->model("coffee_table");
        glm::mat4 table_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.6f, 0.06f, -0.13f));
        draw_model(shader, coffee_table, table_mat, glm::vec3(0.55f, 0.36f, 0.22f));

        // tv komoda
        auto tv_cabinet = rc->model("tv_cabinet");
        glm::mat4 cabinet_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.9f, 0.0f, -1.8f));
        cabinet_mat = glm::rotate(cabinet_mat, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        draw_model(shader, tv_cabinet, cabinet_mat, glm::vec3(0.25f, 0.22f, 0.20f));

        // ekran tv-a
        auto tv_model = rc->model("tv");
        glm::mat4 tv_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.52f, 0.32f, -1.9f));
        tv_mat = glm::rotate(tv_mat, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        draw_model(shader, tv_model, tv_mat, glm::vec3(0.12f, 0.12f, 0.12f));


        // veliki kauc naspram tv-a
        auto sofa_model = rc->model("sofa");
        glm::mat4 sofa_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, .9f));
        draw_model(shader, sofa_model, sofa_mat, glm::vec3(0.25f, 0.52f, 0.7f));

        // ugaona fotelja
        auto chair_model = rc->model("lounge_chair");
        glm::mat4 chair_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.0f, 0.4f));
        chair_mat = glm::rotate(chair_mat, glm::radians(27.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        draw_model(shader, chair_model, chair_mat, glm::vec3(0.85f, 0.45f, 0.35f));

        // medved
        auto bear_model = rc->model("bear");
        glm::mat4 bear_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-1.25f, 0.87f, -1.81f));
        bear_mat = glm::scale(bear_mat, glm::vec3(0.7f));
        bear_mat = glm::rotate(bear_mat, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        draw_model(shader, bear_model, bear_mat, glm::vec3(0.65f, 0.42f, 0.25f));

        // radni sto
        auto desk_model = rc->model("desk");
        glm::mat4 desk_mat = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, 0.0f, -1.0f));
        desk_mat = glm::rotate(desk_mat, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        draw_model(shader, desk_model, desk_mat, glm::vec3(0.58f, 0.38f, 0.24f));

        // podna lampa + izvor tackastog svetla
        auto lamp_model = rc->model("floor_lamp");
        glm::mat4 lamp_mat = glm::translate(glm::mat4(1.0f), glm::vec3(m_point_light.position.x, 0.07f, m_point_light.position.z));
        glm::vec3 lamp_color = m_point_light.enabled ? glm::vec3(0.9f, 0.8f, 0.4f) : glm::vec3(0.3f, 0.3f, 0.3f);
        draw_model(shader, lamp_model, lamp_mat, lamp_color);


        // gui
        if (m_draw_gui) {
            draw_gui();
        }
        platform->swap_buffers();
    }

private:
    DirLightSettings m_dir_light{};
    PointLightSettings m_point_light{};

    void draw_gui() {
        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto camera = graphics->camera();
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

        float dt = platform->dt();
        float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;
        float frame_time_ms = dt * 1000.0f;

        graphics->begin_gui();


        ImGui::Begin("devtools", &m_draw_gui);

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
                    m_mouse_captured ? "yes" : "no");

        ImGui::SliderFloat("movespeed", &camera->MovementSpeed, 1.0f, 15.0f);
        ImGui::Separator();
        // Kontrole za Sunce (Directional Light)
        if (ImGui::CollapsingHeader("Directional Light (Sun)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Sun Enabled", &m_dir_light.enabled);
            ImGui::SliderFloat3("Sun Direction", &m_dir_light.direction.x, -1.0f, 1.0f);
            ImGui::ColorEdit3("Sun Color", &m_dir_light.color.x);
        }

        // Kontrole za Sobnu Lampu (Point Light)
        if (ImGui::CollapsingHeader("Point Light (Floor Lamp)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Lamp Enabled", &m_point_light.enabled);
            ImGui::SliderFloat3("Lamp Position", &m_point_light.position.x, -2.0f, 2.0f);
            ImGui::ColorEdit3("Lamp Color", &m_point_light.color.x);
        }
        ImGui::End();
        graphics->end_gui();
    }

    void draw_model(const engine::resources::Shader *shader, engine::resources::Model *model, const glm::mat4 &transform, const glm::vec3 &color) {
        if (model == nullptr) {
            return;
        }

        shader->set_mat4("model", transform);
        shader->set_vec3("objectColor", color);
        model->draw(shader);
    }

    bool m_mouse_captured{true};
    bool m_draw_gui{true};
    bool m_spotlight_attached_to_camera{true};
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
