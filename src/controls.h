#ifndef CONTROLS_H
#define CONTROLS_H

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "fluid_sandbox.h"

constexpr char const FONT_PATH_FROM_BUILD[] = "../../assets/Roboto-Regular.ttf";
constexpr char const FONT_PATH_FROM_SOURCE[] = "../assets/Roboto-Regular.ttf";
constexpr char const FONT_PATH_FROM_PROJECT[] = "./assets/Roboto-Regular.ttf";
constexpr int FONT_SIZE = 15;

constexpr float LINE_SPACING = 1.3f;
constexpr float TEXT_X_OFFSET = 10.0f;
constexpr float TEXT_Y_OFFSET = 10.0f;

/**
 * @brief Represents a simulation parameter that can be changed.
 */
struct Param
{
public:
    std::string name;
    char key;
    float default_value;
    float &value;
    float step_size;
    float min_value = std::numeric_limits<float>::lowest();
    float max_value = std::numeric_limits<float>::max();

    /**
     * @brief Converts a character key to an sf::Keyboard::Key.
     * @param key The character representing the key.
     * @return The corresponding sf::Keyboard::Key.
     */
    sf::Keyboard::Key convert_key(char key);

    /**
     * @brief Updates the parameter's value based on keyboard input.
     * Handles incrementing, decrementing, reseting, and clamping to min/max values.
     * @param dt Time step (used to normalize the increase/decrease based on time).
     */
    void update(float dt);
};

/**
 * @brief Manages and displays the simulation controls and parameter information.
 */
class ControlsDisplay : public sf::Drawable
{
public:
    /**
     * @brief Constructs the ControlsDisplay.
     * @param sandbox Reference to the FluidSandbox to access and modify its parameters.
     * @param width The width of the display area.
     */
    ControlsDisplay(FluidSandbox &sandbox, unsigned int width);

    /**
     * @brief Updates the state of all parameters.
     * @param dt Time step.
     */
    void update(float dt);

    /**
     * @brief Draws the controls and parameter information to the render target.
     * @param target The render target.
     * @param states Current render states.
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    FluidSandbox &sandbox_;
    sf::Font font_;
    unsigned int width_;
    float dt_ = 0.0f;
    std::vector<Param> params_;

    /**
     * @brief Helper function to draw a line of text.
     * @param text The string to draw.
     * @param style The style of the text (e.g., sf::Text::Bold).
     * @param target The render target.
     * @param text_template A pre-configured sf::Text object to use as a template.
     * @param y_offset Current Y offset for drawing, updated by this function.
     */
    void draw_text(const std::string &text, sf::Text::Style style, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const;

    /**
     * @brief Helper function to draw an informational line (name and value).
     * @param text The descriptive name of the info.
     * @param value The numerical value to display.
     * @param target The render target.
     * @param text_template A pre-configured sf::Text object.
     * @param y_offset Current Y offset for drawing, updated by this function.
     */
    void draw_info(const std::string &text, float value, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const;
    /**
     * @brief Helper function to draw information for a Param struct.
     * @param param The Param object to display.
     * @param target The render target.
     * @param text_template A pre-configured sf::Text object.
     * @param y_offset Current Y offset for drawing, updated by this function.
     */
    void draw_info(const Param &param, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const;
};

#endif
