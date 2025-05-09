#ifndef CONTROLS_H
#define CONTROLS_H

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "fluid_sandbox.h"

constexpr char const FONT_PATH[] = "../../assets/Roboto-Regular.ttf";
constexpr int FONT_SIZE = 15;

constexpr float LINE_SPACING = 1.3f;
constexpr float TEXT_X_OFFSET = 10.0f;
constexpr float TEXT_Y_OFFSET = 10.0f;

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

    sf::Keyboard::Key convert_key(char key);
    void update(float dt);
};

class ControlsDisplay : public sf::Drawable
{
public:
    ControlsDisplay(FluidSandbox &sandbox, unsigned int width);

    void update(float dt);
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    FluidSandbox &sandbox_;
    sf::Font font_;
    unsigned int width_;
    float dt_ = 0.0f;
    std::vector<Param> params_;

    void draw_text(const std::string &text, sf::Text::Style style, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const;
    void draw_info(const std::string &text, float value, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const;
    void draw_info(const Param& param, sf::RenderTarget &target, sf::Text &text_template, float &y_offset) const;
};

#endif
