#include <SFML/Graphics.hpp>

#include <iostream>

#include "fluid_sandbox.h"

constexpr char const *WINDOW_TITLE = "Fluid Simulation Sandbox";

constexpr unsigned int DEFAULT_WINDOW_WIDTH = 800;
constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 500;

constexpr unsigned int FRAMERATE_LIMIT = 144;

constexpr float SIMULATION_SPEED = 5.0f;

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}), WINDOW_TITLE);
    window.setFramerateLimit(FRAMERATE_LIMIT);

    FluidSandbox sandbox({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT});

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
            std::cout << "FPS: " << FRAMERATE_LIMIT / clock.restart().asSeconds() << std::endl;
            counter = 0;
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && counter % 10 ==0)
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.add_particle({static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y)});
        }

        sandbox.update(1.0f / FRAMERATE_LIMIT * SIMULATION_SPEED);
        window.clear();
        window.draw(sandbox);
        window.display();
    }
    return 0;
}
