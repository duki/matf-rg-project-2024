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

            auto floor_model = rc->model("floor");
            glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            draw_model(shader, floor_model, model_mat, glm::vec3(0.78f, 0.62f, 0.45f));
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
