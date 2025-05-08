#include <SFML/Graphics.hpp>

#include "fluid_sandbox.h"
#include "controls.h"

#include <algorithm>
#include <limits>

#include <iostream>

constexpr char const *WINDOW_TITLE = "Fluid Simulation Sandbox";

constexpr unsigned int DEFAULT_WINDOW_WIDTH = 1500;
constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 800;

constexpr size_t MAX_FRAME_RATE = 100;

sf::Keyboard::Key convert_key(char key)
{
    if (key >= 'A' && key <= 'Z')
    {
        return static_cast<sf::Keyboard::Key>(key - 'A' + static_cast<char>(sf::Keyboard::Key::A));
    }
    else if (key >= '0' && key <= '9')
    {
        return static_cast<sf::Keyboard::Key>(key - '0' + static_cast<char>(sf::Keyboard::Key::Num0));
    }
    return sf::Keyboard::Key::Unknown;
}

void adjust_param(
    float &param_value,
    char key,
    float step,
    bool increase,
    bool decrease,
    float min_val = std::numeric_limits<float>::min(),
    float max_val = std::numeric_limits<float>::max())
{
    if (sf::Keyboard::isKeyPressed(convert_key(key)))
    {
        if (increase)
        {
            param_value += step;
        }
        if (decrease)
        {
            param_value -= step;
        }
        param_value = std::max(min_val, param_value);
        param_value = std::min(max_val, param_value);
    }
}

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}), WINDOW_TITLE);
    window.setFramerateLimit(MAX_FRAME_RATE);

    FluidSandbox sandbox({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT});

    sf::Clock clock;
    auto window_position = window.getPosition();
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (const auto *resized = event->getIf<sf::Event::Resized>())
            {
                window.setView(sf::View(sf::FloatRect({0, 0}, static_cast<sf::Vector2f>(resized->size))));
                sandbox.resize({resized->size.x, resized->size.y});
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.add_particles(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.remove_particles(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        {
            sandbox.clear_particles();
        }
        auto new_window_position = window.getPosition();
        if (window_position != new_window_position)
        {
            sandbox.push_particles(static_cast<sf::Vector2f>(window_position - new_window_position) / 10.0f);
            window_position = new_window_position;
        }

        bool increase = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add);
        bool decrease = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract);

        float dt = clock.restart().asSeconds();

        adjust_param(sandbox.get_params().simulation_speed, SIMULATION_SPEED_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().gravity_x, GRAVITY_X_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().gravity_y, GRAVITY_Y_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().edge_bounciness, EDGE_BOUNCINESS_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().interaction_radius, INTERACTION_RADIUS_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().rest_density, REST_DENSITY_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().stiffness, STIFFNESS_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().near_stiffness, NEAR_STIFFNESS_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().linear_viscosity, LINEAR_VISCOSITY_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().quadratic_viscosity, QUADRATIC_VISCOSITY_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().plasticity, PLASTICITY_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().yield_ratio, YIELD_RATIO_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().spring_stiffness, SPRING_STIFFNESS_KEY, dt, increase, decrease);

        adjust_param(sandbox.get_params().control_radius, CONTROL_RADIUS_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().particle_spawn_rate, PARTICLE_SPAWN_RATE_KEY, dt, increase, decrease);

        adjust_param(sandbox.get_params().base_particle_size, BASE_PARTICLE_SIZE_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().particle_stress_size_multiplier, PARTICLE_STRESS_SIZE_MULTIPLIER_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().base_particle_color, BASE_PARTICLE_COLOR_KEY, dt, increase, decrease);
        adjust_param(sandbox.get_params().particle_stress_color_multiplier, PARTICLE_STRESS_COLOR_MULTIPLIER_KEY, dt, increase, decrease);

        sandbox.update(dt);
        window.clear();
        window.draw(sandbox);
        window.display();
    }
    return 0;
}
