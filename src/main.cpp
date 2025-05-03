#include <SFML/Graphics.hpp>

#include <iostream>

#include "fluid_sandbox.h"

constexpr char const *WINDOW_TITLE = "Fluid Simulation Sandbox";

constexpr unsigned int DEFAULT_WINDOW_WIDTH = 1200;
constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 700;

constexpr unsigned int FRAMERATE_LIMIT = 120;

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}), WINDOW_TITLE);
    window.setFramerateLimit(FRAMERATE_LIMIT);

    SimulationParameters params = {1.0f, {0, 0.4f}, 60.0f, 6.0f, 0.5f, 0.5f};

    FluidSandbox sandbox({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, params);

    sf::Clock clock;
    int counter = 1;
    auto window_position = window.getPosition();
    while (window.isOpen())
    {
        counter++;
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

        if (counter % FRAMERATE_LIMIT == 0)
        {
            std::cout << "FPS: " << FRAMERATE_LIMIT / clock.restart().asSeconds()
                      << ", Particle count: " << sandbox.particle_count() << std::endl;
            counter = 0;
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.add_particles(static_cast<sf::Vector2f>(mouse_position), 50.0f, 5);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.remove_particles(static_cast<sf::Vector2f>(mouse_position), 50.0f);
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

        sandbox.update();
        window.clear();
        window.draw(sandbox);
        window.display();
    }
    return 0;
}
