#include <SFML/Graphics.hpp>

#include "fluid_sandbox.h"
#include "controls.h"

constexpr char const WINDOW_TITLE[] = "Fluid Simulation Sandbox";

constexpr unsigned int DEFAULT_WINDOW_WIDTH = 1500;
constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 800;
constexpr unsigned int SIDEBAR_WIDTH = 300;

constexpr size_t FRAME_RATE_LIMIT = 100;

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}), WINDOW_TITLE);
    window.setFramerateLimit(FRAME_RATE_LIMIT);

    FluidSandbox sandbox({(DEFAULT_WINDOW_WIDTH > SIDEBAR_WIDTH ? DEFAULT_WINDOW_WIDTH - SIDEBAR_WIDTH : 0), DEFAULT_WINDOW_HEIGHT});
    ControlsDisplay controls_display(sandbox, SIDEBAR_WIDTH);

    sf::Clock clock;
    auto window_position = window.getPosition();

    bool lock_pressed = false;

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
                sandbox.resize({(resized->size.x > SIDEBAR_WIDTH ? resized->size.x - SIDEBAR_WIDTH : 0), resized->size.y});
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.add_particles(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.remove_particles(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.add_object(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H))
        {
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.remove_object(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        {
            lock_pressed = true;
        }
        else if (lock_pressed)
        {
            lock_pressed = false;
            const auto mouse_position = sf::Mouse::getPosition(window);
            sandbox.toggle_lock_object(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        {
            sandbox.clear();
        }
        auto new_window_position = window.getPosition();
        if (window_position != new_window_position)
        {
            sandbox.push_everything(static_cast<sf::Vector2f>(window_position - new_window_position) / 10.0f);
            window_position = new_window_position;
        }

        float dt = clock.restart().asSeconds();

        controls_display.update(dt);
        sandbox.update(dt);

        window.clear();
        window.draw(sandbox);
        window.draw(controls_display);
        window.display();
    }
    return 0;
}
