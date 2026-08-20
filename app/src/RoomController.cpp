#include <RoomController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

MainPlatformEventObserver::MainPlatformEventObserver(engine::graphics::Camera *camera)
    : m_camera(camera) {
}

void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    if (!platform->is_cursor_enabled()) {
        m_camera->rotate_camera(position.dx, position.dy);
    }
}

void MainPlatformEventObserver::on_scroll(engine::platform::MousePosition position) {
    m_camera->zoom(position.scroll);
}

void RoomController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    // settuj cameru na pocetku scene
    auto camera = graphics->camera();
    camera->Position = glm::vec3(0.0f, 1.8f, 2.8f);
    camera->Yaw = -90.0f;
    camera->Pitch = -12.0f;

    // settuj stanje kradje cursora na pocetku scene i registruj observer
    platform->set_enable_cursor(false);
    auto observer = std::make_unique<MainPlatformEventObserver>(camera);
    platform->register_platform_event_observer(std::move(observer));

    spdlog::info("RoomController initialized.");
}

void RoomController::poll_events() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    // f2 toggle za kradju kursora
    if (platform->key(engine::platform::KeyId::KEY_F2).state() == engine::platform::Key::State::JustPressed) {
        platform->set_enable_cursor(!platform->is_cursor_enabled());
    }


    // f3 toggle za filmski mod
    if (platform->key(engine::platform::KeyId::KEY_F3).state() == engine::platform::Key::State::JustPressed) {
        trigger_movie_mode();
    }
}

void RoomController::update() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    float dt = platform->dt();

    if (!platform->is_cursor_enabled()) {
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

    if (m_movie_mode_triggered) {
        m_event_timer += dt;

        // A - ugasi tackasto svetlo, promeni direkcionog svetla boju
        if (m_event_timer >= 1.5f && !m_event_a_done) {
            m_event_a_done = true;
            m_point_light.enabled = false;
            m_dir_light.color = glm::vec3(0.08f, 0.08f, 0.15f);
            spdlog::info("\x1b[35mlamp off\x1b[0m");
        }

        // B - upali point svetlo, namesti boju da izgleda kao da je emituje tv
        if (m_event_timer >= 3.5f && !m_event_b_done) {
            m_event_b_done = true;
            m_tv_glow = 1.0f;

            m_point_light.position = glm::vec3(0.0f, 0.9f, -1.6f);
            m_point_light.color = glm::vec3(0.6f, 0.85f, 1.2f);
            m_point_light.enabled = true;
            spdlog::info("\x1b[36mtv on\x1b[0m");
        }
    }
}

void RoomController::draw() {
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
    glm::vec3 tv_color = glm::mix(glm::vec3(0.12f, 0.12f, 0.12f), glm::vec3(0.85f, 0.95f, 1.0f), m_tv_glow);
    draw_model(shader, tv_model, tv_mat, tv_color);

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
}
void RoomController::end_draw() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    platform->swap_buffers();
}

void RoomController::draw_model(const engine::resources::Shader *shader, engine::resources::Model *model, const glm::mat4 &transform, const glm::vec3 &color) {
    if (model == nullptr) {
        return;
    }

    shader->set_mat4("model", transform);
    shader->set_vec3("objectColor", color);
    model->draw(shader);
}

void RoomController::trigger_movie_mode() {
    m_movie_mode_triggered = true;
    m_event_timer = 0.0f;
    m_event_a_done = false;
    m_event_b_done = false;
    spdlog::info("start movie mode");
}

void RoomController::reset_mode() {
    m_movie_mode_triggered = false;
    m_event_timer = 0.0f;
    m_event_a_done = false;
    m_event_b_done = false;
    m_tv_glow = 0.0f;

    m_point_light.position = glm::vec3(1.4f, 1.4f, 0.6f);
    m_point_light.color = glm::vec3(0.70f, 0.55f, 0.25f);
    m_point_light.enabled = true;

    m_dir_light.color = glm::vec3(0.45f, 0.40f, 0.35f);
    spdlog::info("reset mode");
}
