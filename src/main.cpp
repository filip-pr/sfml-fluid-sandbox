#include <SFML/Graphics.hpp>

#include <optional>

#include "fluid_sandbox.h"
#include "controls.h"

constexpr char const WINDOW_TITLE[] = "Fluid Simulation Sandbox";

constexpr unsigned int DEFAULT_WINDOW_WIDTH = 1500;
constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 900;
constexpr unsigned int SIDEBAR_WIDTH = 300;

constexpr size_t FRAME_RATE_LIMIT = 100;

constexpr float WINDOW_MOVE_STRENGTH = 0.1f;

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}), WINDOW_TITLE);
    window.setFramerateLimit(FRAME_RATE_LIMIT);

    FluidSandbox sandbox({(DEFAULT_WINDOW_WIDTH > SIDEBAR_WIDTH ? DEFAULT_WINDOW_WIDTH - SIDEBAR_WIDTH : 0), DEFAULT_WINDOW_HEIGHT});
    ControlsDisplay controls_display(sandbox, SIDEBAR_WIDTH);

    sf::Clock clock;
    auto window_position = window.getPosition();

    bool lock_pressed = false;
    std::optional<Object *> grabbed_object = std::nullopt;
    sf::Vector2i grab_offset;
    bool grabbed_object_locked = false;

    while (window.isOpen())
    {
        // Window events handling
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

        // Keyboard and mouse input handling
        const auto mouse_position = sf::Mouse::getPosition(window);
        auto new_window_position = window.getPosition();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            sandbox.add_particles(static_cast<sf::Vector2f>(mouse_position));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            sandbox.remove_particles(static_cast<sf::Vector2f>(mouse_position));
        }
        if (!grabbed_object.has_value()) // Don't allow adding/removing objects while dragging an object (it could invalidate the pointer)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
            {
                sandbox.add_object(static_cast<sf::Vector2f>(mouse_position));
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H))
            {
                sandbox.remove_object(static_cast<sf::Vector2f>(mouse_position));
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)) // Only lock / unlock objects on key release
            {
                lock_pressed = true;
            }
            else if (lock_pressed)
            {
                lock_pressed = false;
                sandbox.toggle_lock_object(static_cast<sf::Vector2f>(mouse_position));
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
            {
                sandbox.clear();
            }
        }

        if (window_position != new_window_position)
        {
            sandbox.push_everything(static_cast<sf::Vector2f>(window_position - new_window_position) * WINDOW_MOVE_STRENGTH);
            window_position = new_window_position;
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            if (grabbed_object == std::nullopt)
            {
                grabbed_object = sandbox.try_grab_object(static_cast<sf::Vector2f>(mouse_position));
                if (grabbed_object.has_value())
                {
                    if (!grabbed_object.value()->is_locked)
                    {
                        grabbed_object.value()->toggle_lock();
                        grabbed_object_locked = true;
                    }
                    else
                    {
                        grabbed_object_locked = false;
                    }
                    grab_offset = static_cast<sf::Vector2i>(mouse_position) - static_cast<sf::Vector2i>(grabbed_object.value()->position);
                }
            }
            else
            {
                if (grabbed_object.value() != nullptr)
                {
                    grabbed_object.value()->position = static_cast<sf::Vector2f>(mouse_position) - static_cast<sf::Vector2f>(grab_offset);
                }
            }
        }
        else if (grabbed_object.has_value())
        {
            if (grabbed_object_locked)
                grabbed_object.value()->toggle_lock();
            grabbed_object = std::nullopt;
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
