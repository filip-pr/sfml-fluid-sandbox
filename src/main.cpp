#include <SFML/Graphics.hpp>

#include <iostream>

#include "fluid_sandbox.h"

constexpr char const *WINDOW_TITLE = "Fluid Simulation Sandbox";

constexpr unsigned int DEFAULT_WINDOW_WIDTH = 1200;
constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 700;

constexpr unsigned int FRAMERATE_LIMIT = 60;


int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}), WINDOW_TITLE);
    //window.setFramerateLimit(FRAMERATE_LIMIT);

    FluidSandbox sandbox({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, 1);

    sf::Clock clock;
    int counter = 1;
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
                window.setView(sf::View(sf::FloatRect({0, 0}, {static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)})));
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
            for (int i = 0; i < 50; i++)
            {
                float velocity_x = (rand() % 100) / 10.0f - 5.0f;
                float velocity_y = (rand() % 100) / 10.0f - 5.0f;
                float x_offset = (rand() % 100) / 100.0f - 5.0f;
                float y_offset = (rand() % 100) / 100.0f - 5.0f;
                sandbox.add_particle({static_cast<float>(mouse_position.x) + x_offset, static_cast<float>(mouse_position.y) + y_offset}, {velocity_x, velocity_y});
            }
        }

        auto mouse_position = sf::Mouse::getPosition(window);

        sandbox.update();
        window.clear();
        window.draw(sandbox);
        window.display();
    }
    return 0;
}
