#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <glm/gtc/matrix_transform.hpp>
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


            glm::mat4 projection = graphics->projection_matrix();
            glm::mat4 view = camera->view_matrix();
            shader->set_mat4("projection", projection);
            shader->set_mat4("view", view);

            // crtanje poda
            auto floor_model = rc->model("floor");
            for (int x = -2; x <= 2; x++) {
                for (int z = -2; z <= 2; z++) {
                    glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(x * 1.0f + 0.0f, 0.0f, z * 1.0f + 0.0f));
                    draw_model(shader, floor_model, model_mat, glm::vec3(0.28f, 0.62f, 0.45f));
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


        platform->swap_buffers();
    }

private:
    void draw_model(const engine::resources::Shader *shader, engine::resources::Model *model, const glm::mat4 &transform, const glm::vec3 &color) {
        if (model == nullptr) {
            return;
        }

        shader->set_mat4("model", transform);
        shader->set_vec3("objectColor", color);
        model->draw(shader);
    }

    bool m_mouse_captured{true};
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
