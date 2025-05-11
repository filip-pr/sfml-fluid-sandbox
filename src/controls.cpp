#include <SFML/Graphics.hpp>

#include <string>
#include <sstream>
#include <iomanip>
#include <math.h>

#include "controls.h"
#include "fluid_sandbox.h"

sf::Keyboard::Key Param::convert_key(char key)
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

void Param::update(float dt)
{
    if (sf::Keyboard::isKeyPressed(convert_key(key)))
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal))
        {
            value += step_size * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Hyphen))
        {
            value -= step_size * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Backspace))
        {
            value = default_value;
        }
        value = std::max(min_value, value);
        value = std::min(max_value, value);
    }
}

ControlsDisplay::ControlsDisplay(FluidSandbox &sandbox, unsigned int width) : sandbox_(sandbox), width_(width)
{
    std::string font_path; // To allow for running from different directories
    if (std::filesystem::exists(FONT_PATH_FROM_BUILD))
    {
        font_path = FONT_PATH_FROM_BUILD;
    }
    else if (std::filesystem::exists(FONT_PATH_FROM_SOURCE))
    {
        font_path = FONT_PATH_FROM_SOURCE;
    }
    else if (std::filesystem::exists(FONT_PATH_FROM_PROJECT))
    {
        font_path = FONT_PATH_FROM_PROJECT;
    }
    else
    {
        throw std::runtime_error("Font file not found");
    }
    if (!font_.openFromFile(font_path))
    {
        throw std::runtime_error("Failed to load font");
    }
    params_.emplace_back(Param{"Sim Speed", '1', SIMULATION_SPEED_DEFAULT, sandbox_.params().simulation_speed, 50.0f, 0.01f, 100.0f});
    params_.emplace_back(Param{"Gravity X", '2', GRAVITY_X_DEFAULT, sandbox_.params().gravity_x, 0.5f});
    params_.emplace_back(Param{"Gravity Y", '3', GRAVITY_Y_DEFAULT, sandbox_.params().gravity_y, 0.5f});
    params_.emplace_back(Param{"Edge Bounciness", '4', EDGE_BOUNCINESS_DEFAULT, sandbox_.params().edge_bounciness, 0.5f, 0.0f, 1.0f});
    params_.emplace_back(Param{"Interaction Radius", '5', INTERACTION_RADIUS_DEFAULT, sandbox_.params().interaction_radius, 20.0f, 0.0f});
    params_.emplace_back(Param{"Rest Density", '6', REST_DENSITY_DEFAULT, sandbox_.params().rest_density, 5.0f, 0.0f, 10.0f});
    params_.emplace_back(Param{"Stiffness", '7', STIFFNESS_DEFAULT, sandbox_.params().stiffness, 0.5f, 0.0f});
    params_.emplace_back(Param{"Near Stiffness", '8', NEAR_STIFFNESS_DEFAULT, sandbox_.params().near_stiffness, 0.5f, 0.0f});
    params_.emplace_back(Param{"Linear Viscosity", '9', LINEAR_VISCOSITY_DEFAULT, sandbox_.params().linear_viscosity, 0.5f, 0.0f});
    params_.emplace_back(Param{"Quad Viscosity", '0', QUADRATIC_VISCOSITY_DEFAULT, sandbox_.params().quadratic_viscosity, 0.5f, 0.0f});
    params_.emplace_back(Param{"Plasticity", 'Q', PLASTICITY_DEFAULT, sandbox_.params().plasticity, 0.5f, 0.2f, 1.0f});
    params_.emplace_back(Param{"Yield Ratio", 'W', YIELD_RATIO_DEFAULT, sandbox_.params().yield_ratio, 0.2f, 0.0f, 1.0f});
    params_.emplace_back(Param{"Spring Stiffness", 'E', SPRING_STIFFNESS_DEFAULT, sandbox_.params().spring_stiffness, 0.5f, 0.0f, 1.0f});
    params_.emplace_back(Param{"Control Radius", 'R', CONTROL_RADIUS_DEFAULT, sandbox_.params().control_radius, 50.0f, 0.01f});
    params_.emplace_back(Param{"Spawn Rate", 'T', PARTICLE_SPAWN_RATE_DEFAULT, sandbox_.params().particle_spawn_rate, 5.0f, 0.01f});
    params_.emplace_back(Param{"Object Radius", 'Y', OBJECT_RADIUS_DEFAULT, sandbox_.params().object_radius, 50.0f, 0.01f});
    params_.emplace_back(Param{"Object Mass", 'U', OBJECT_MASS_DEFAULT, sandbox_.params().object_mass, 50.0f, 0.01f});
    params_.emplace_back(Param{"Base Size", 'I', BASE_PARTICLE_SIZE_DEFAULT, sandbox_.params().base_particle_size, 5.0f, 0.0f});
    params_.emplace_back(Param{"Stress Size Mult", 'O', PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT, sandbox_.params().particle_stress_size_multiplier, 5.0f, 0.0f});
    params_.emplace_back(Param{"Base Color", 'P', BASE_PARTICLE_COLOR_DEFAULT, sandbox_.params().base_particle_color, 50.0f, 0.0f});
    params_.emplace_back(Param{"Stress Color Mult", 'A', PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT, sandbox_.params().particle_stress_color_multiplier, 50.0f, 0.0f});
}

void ControlsDisplay::update(float dt)
{
    dt_ = dt;
    for (auto &param : params_)
    {
        param.update(dt);
    }
}

void ControlsDisplay::draw_text(const std::string &text, sf::Text::Style style, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const
{
    text_template.setString(text);
    text_template.setPosition({sandbox_.size().x + TEXT_X_OFFSET, std::round(y_offset)});
    text_template.setStyle(style);
    target.draw(text_template);
    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;
}

void ControlsDisplay::draw_info(const std::string &text, float value, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const
{
    std::stringstream ss;
    text_template.setStyle(sf::Text::Regular);
    ss << text << ": " << std::fixed << std::setprecision(2) << value;
    text_template.setString(ss.str());
    text_template.setPosition({sandbox_.size().x + TEXT_X_OFFSET, std::round(y_offset)});
    target.draw(text_template);
    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;
}

void ControlsDisplay::draw_info(const Param &param, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const
{
    std::stringstream ss;
    text_template.setStyle(sf::Text::Regular);
    ss << param.name << " (key: " << param.key << ")"
       << ": " << std::fixed << std::setprecision(2) << param.value;
    text_template.setString(ss.str());
    text_template.setPosition({sandbox_.size().x + TEXT_X_OFFSET, std::round(y_offset)});
    target.draw(text_template);
    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;
}

void ControlsDisplay::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::RectangleShape sidebar_bg({static_cast<float>(width_), static_cast<float>(target.getSize().y)});
    sidebar_bg.setPosition({static_cast<float>(sandbox_.size().x), 0.0f});
    sidebar_bg.setFillColor(sf::Color(192, 192, 192));
    target.draw(sidebar_bg);

    sf::Text text_template(font_);
    text_template.setCharacterSize(FONT_SIZE);
    text_template.setFillColor(sf::Color::Black);

    float y_offset = TEXT_Y_OFFSET;

    draw_text("Runtime Stats", sf::Text::Bold, target, text_template, y_offset);
    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;

    draw_info("Particles", static_cast<float>(sandbox_.particle_count()), target, text_template, y_offset);
    draw_info("Objects", static_cast<float>(sandbox_.object_count()), target, text_template, y_offset);
    draw_info("Frame Rate", 1 / dt_, target, text_template, y_offset);

    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;
    draw_text("Controls", sf::Text::Bold, target, text_template, y_offset);
    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;

    draw_text("<key> & '+' or '-' to Adjust Param", sf::Text::Regular, target, text_template, y_offset);
    draw_text("<key> & 'backspace' to Reset Param", sf::Text::Regular, target, text_template, y_offset);
    draw_text("LMB to Grab and Move Objects", sf::Text::Regular, target, text_template, y_offset);
    draw_text("D - Spawn Particles", sf::Text::Regular, target, text_template, y_offset);
    draw_text("F - Delete Particles", sf::Text::Regular, target, text_template, y_offset);
    draw_text("G - Spawn an Object", sf::Text::Regular, target, text_template, y_offset);
    draw_text("H - Delete an Object", sf::Text::Regular, target, text_template, y_offset);
    draw_text("J - Lock/Unlock an Object", sf::Text::Regular, target, text_template, y_offset);
    draw_text("Space - Clear Particles and Objects", sf::Text::Regular, target, text_template, y_offset);

    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;
    draw_text("Simulation Params", sf::Text::Bold, target, text_template, y_offset);
    y_offset += static_cast<float>(FONT_SIZE) * LINE_SPACING;

    for (const auto &param : params_)
    {
        draw_info(param, target, text_template, y_offset);
    }
}
