#include "fluid_sandbox.h"
#include "utils.h"
#include "controls.h"

#include <algorithm>
#include <cmath>

void FluidSandbox::clear_particles()
{
    particles_.clear();
}

void FluidSandbox::add_particles(sf::Vector2f position)
{
    size_t num_new_particles = static_cast<size_t>(params_.particle_spawn_rate * dt_);
    if (num_new_particles == 0)
    {
        num_new_particles = static_cast<float>(rand()) / RAND_MAX < params_.particle_spawn_rate * dt_ ? 1 : 0;
    }
    particles_.reserve(particles_.size() + num_new_particles);
    for (size_t i = 0; i < num_new_particles; ++i)
    {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float distance = static_cast<float>(rand()) / RAND_MAX * params_.control_radius;
        sf::Vector2f offset = {std::cos(angle) * distance, std::sin(angle) * distance};
        particles_.emplace_back(position + offset);
    }
}

void FluidSandbox::remove_particles(sf::Vector2f position)
{
    float radius_sq = params_.control_radius * params_.control_radius;
    auto it = std::remove_if(particles_.begin(), particles_.end(),
                             [position, radius_sq](const Particle &particle)
                             {
                                 return utils::distance_sq(particle.position, position) < radius_sq;
                             });
    particles_.erase(it, particles_.end());
}

void FluidSandbox::push_particles(sf::Vector2f velocity)
{
    for (auto &&particle : particles_)
    {
        particle.velocity += velocity;
    }
}

void FluidSandbox::update(float dt)
{
    dt_ = std::min(dt * params_.simulation_speed, 1.0f); // to prevent instability
    frame_rate_ = 1.0f / dt;
    move_particles();
    update_neighbors();
    adjust_apply_strings();
    do_double_density_relaxation();
    resolve_collisions();
    recalculate_velocity();
    apply_gravity();
    apply_viscosity();
    reverse_calculation_order_ = !reverse_calculation_order_;
}

void FluidSandbox::move_particles()
{
    for (auto &&particle : particles_)
    {
        particle.update(dt_);
    }
    grid_.update(particles_, params_.interaction_radius);
}

void FluidSandbox::update_neighbors()
{
    if (particles_.size() != particle_neighbors_.size())
    {
        particle_neighbors_.resize(particles_.size());
    }
    size_t particle_id = 0;
    for (auto &&particle : particles_)
    {
        particle_neighbors_[particle_id] = grid_.query(particle.position, params_.interaction_radius);
        ++particle_id;
    }
}

void FluidSandbox::adjust_apply_strings()
{
    if (params_.spring_stiffness == 0.0f || params_.plasticity == 0.0f)
        return;

    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_plasticity = params_.plasticity * dt_;
    const float dt_sq_spring_stiffness_half = params_.spring_stiffness * dt_ * dt_ * 0.5f;

    size_t num_particles = particles_.size();

    for (size_t i = 0; i < num_particles; ++i)
    {
        size_t particle_id = reverse_calculation_order_ ? num_particles - i - 1 : i;
        auto &particle = particles_[particle_id];

        auto &neighbors = particle_neighbors_[particle_id];

        std::unordered_map<size_t, float> new_springs;
        new_springs.reserve(neighbors.size());

        for (auto &&neighbor : neighbors)
        {
            if (neighbor <= &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }
            float distance = std::sqrt(distance_sq);
            float spring_length;

            auto it = particle.springs.find(neighbor->id);
            if (it != particle.springs.end())
            {
                spring_length = it->second;
            }
            else
            {
                spring_length = params_.interaction_radius;
            }
            float tolerable_deformation = spring_length * params_.yield_ratio;
            if (distance > spring_length + tolerable_deformation)
            {
                spring_length += dt_plasticity * (distance - spring_length - tolerable_deformation);
            }
            else if (distance < spring_length - tolerable_deformation)
            {
                spring_length -= dt_plasticity * (spring_length - distance - tolerable_deformation);
            }
            if (spring_length > params_.interaction_radius)
            {
                continue;
            }
            new_springs.emplace(neighbor->id, spring_length);

            float displacement_magnitude = dt_sq_spring_stiffness_half * (1 - spring_length * inv_interaction_radius) * (spring_length - distance) / distance;

            sf::Vector2f displacement = (neighbor->position - particle.position) * displacement_magnitude;

            particle.position -= displacement;
            neighbor->position += displacement;
        }
        std::swap(particle.springs, new_springs);
    }
}

void FluidSandbox::do_double_density_relaxation()
{
    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_sq_half = 0.5f * dt_ * dt_;

    size_t num_particles = particles_.size();

    for (size_t i = 0; i < num_particles; ++i)
    {
        size_t particle_id = reverse_calculation_order_ ? num_particles - i - 1 : i;
        auto &particle = particles_[particle_id];
        float density = 0.0f;
        float near_density = 0.0f;

        auto &neighbors = particle_neighbors_[particle_id];

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance * inv_interaction_radius;

            float one_minus_ratio = 1.0f - distance_ratio;
            float one_minus_ratio_sq = one_minus_ratio * one_minus_ratio;

            density += one_minus_ratio_sq;
            near_density += one_minus_ratio_sq * one_minus_ratio;
        }

        float pressure = params_.stiffness * (density - params_.rest_density);
        float near_pressure = params_.near_stiffness * near_density;

        particle.stress = STRESS_SMOOTHING * particle.stress + (1 - STRESS_SMOOTHING) * near_pressure;

        sf::Vector2f total_displacement = {0.0f, 0.0f};

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            sf::Vector2f position_diff = neighbor->position - particle.position;
            float distance_sq = position_diff.lengthSquared();

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance * inv_interaction_radius;
            float one_minus_ratio = 1.0f - distance_ratio;

            float displacement_magnitude = dt_sq_half * (pressure * one_minus_ratio + near_pressure * (one_minus_ratio * one_minus_ratio)) / distance;

            sf::Vector2f displacement = position_diff * displacement_magnitude;

            neighbor->position += displacement;
            total_displacement -= displacement;
        }
        particle.position += total_displacement;
    }
}

void FluidSandbox::resolve_collisions()
{
    const float min_x = 0;
    const float max_x = static_cast<float>(size_.x);
    const float min_y = 0;
    const float max_y = static_cast<float>(size_.y);

    for (auto &&particle : particles_)
    {
        if (particle.position.x < min_x)
        {
            particle.position.x = min_x;
            particle.velocity.x *= -params_.edge_bounciness;
        }
        else if (particle.position.x > max_x)
        {
            particle.position.x = max_x;
            particle.velocity.x *= -params_.edge_bounciness;
        }

        if (particle.position.y < min_y)
        {
            particle.position.y = min_y;
            particle.velocity.y *= -params_.edge_bounciness;
        }
        else if (particle.position.y > max_y)
        {
            particle.position.y = max_y;
            particle.velocity.y *= -params_.edge_bounciness;
        }
        if (std::isnan(particle.position.x))
        {
            particle.position.x = 0;
        }
        if (std::isnan(particle.position.y))
        {
            particle.position.y = 0;
        }
    }
}

void FluidSandbox::recalculate_velocity()
{
    const float inv_dt = 1.0f / dt_;
    for (auto &&particle : particles_)
    {
        particle.velocity = (particle.position - particle.prev_position) * inv_dt;
    }
}

void FluidSandbox::apply_gravity()
{
    for (auto &&particle : particles_)
    {
        particle.velocity.x += params_.gravity_x * dt_;
        particle.velocity.y += params_.gravity_y * dt_;
    }
}

void FluidSandbox::apply_viscosity()
{
    if (params_.linear_viscosity == 0.0f && params_.quadratic_viscosity == 0.0f)
        return;

    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_half = 0.5f * dt_;

    size_t num_particles = particles_.size();

    for (size_t i = 0; i < num_particles; ++i)
    {
        size_t particle_id = reverse_calculation_order_ ? num_particles - i - 1 : i;
        auto &particle = particles_[particle_id];

        auto &neighbors = particle_neighbors_[particle_id];

        for (auto &&neighbor : neighbors)
        {
            if (neighbor <= &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            sf::Vector2f position_diff = (neighbor->position - particle.position);
            float non_normal_inward_velocity = utils::dot_product((particle.velocity - neighbor->velocity), position_diff);

            if (non_normal_inward_velocity > 0.0f)
            {
                float distance = std::sqrt(distance_sq);
                float inward_velocity = std::min(non_normal_inward_velocity / distance, 1.0f);

                float impulse_magnitude = dt_half * (1 - distance * inv_interaction_radius) * inward_velocity * (params_.linear_viscosity + params_.quadratic_viscosity * inward_velocity) / distance;

                sf::Vector2f impulse = position_diff * impulse_magnitude;

                particle.velocity -= impulse;
                neighbor->velocity += impulse;
            }
        }
    }
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        auto &&particle = particles_[i];
        float particle_size = std::max(params_.base_particle_size + particle.stress * params_.particle_stress_size_multiplier, 1.0f);
        int pressure_color = std::clamp(static_cast<int>(params_.base_particle_color - particle.stress * params_.particle_stress_color_multiplier), 0, 255);
        sf::Color particle_color = sf::Color(pressure_color, pressure_color, 255);

        particle_vertices[i * 6].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 1].position = particle.position + sf::Vector2f({particle_size, -particle_size});
        particle_vertices[i * 6 + 2].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 3].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 4].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 5].position = particle.position + sf::Vector2f(-particle_size, particle_size);
        for (size_t j = 0; j < 6; j++)
        {
            particle_vertices[i * 6 + j].color = particle_color;
        }
    }
    auto sidebar = sf::RectangleShape({static_cast<float>(SIDEBAR_SIZE), static_cast<float>(size_.y)});
    sidebar.setPosition({static_cast<float>(size_.x), 0.0f});

    sidebar.setFillColor(sf::Color(192, 192, 192));

    states.blendMode = sf::BlendMax;
    target.draw(particle_vertices, states);
    states.blendMode = sf::BlendAlpha;
    target.draw(sidebar, states);

    sf::Text text(font_);
    text.setCharacterSize(FONT_SIZE);
    text.setFillColor(sf::Color::Black);

    float current_y = 10.0f;
    const float x_pos = static_cast<float>(size_.x) + 10.0f;
    const float line_height_multiplier = 1.3f;
    const float line_spacing = static_cast<float>(FONT_SIZE) * line_height_multiplier;
    const float section_spacing = line_spacing * 0.5f;

    auto draw_text_line = [&](const std::string &str_content, bool is_underlined = false)
    {
        text.setString(str_content);
        text.setPosition({x_pos, current_y});
        if (is_underlined)
        {
            text.setStyle(sf::Text::Underlined);
        }
        else
        {
            text.setStyle(sf::Text::Regular);
        }
        target.draw(text, states);
        current_y += line_spacing;
    };

    auto draw_info_line = [&](const std::string &name, float value)
    {
        std::stringstream ss;
        ss << name << ": " << std::round(value * 100) / 100.0f;
        draw_text_line(ss.str());
    };

    auto draw_param_info_line = [&](const std::string &name, char key, float default_value, float value)
    {
        std::stringstream ss;
        ss << name << " (key: " << key << ", default: " << std::round(default_value * 100) / 100.0f << ")"
           << ": " << std::round(value * 100) / 100.0f;
        draw_text_line(ss.str());
    };

    draw_text_line("Runtime Stats", true);
    current_y += section_spacing;

    draw_info_line("Particles", particles_.size());
    draw_info_line("Frame Rate", frame_rate_);

    current_y += section_spacing;
    draw_text_line("Controls", true);
    current_y += section_spacing;

    draw_text_line("<key> & '+' or '-' to Adjust Param");
    draw_text_line("S - Spawn Particles");
    draw_text_line("D - Delete Particles");
    draw_text_line("Space - Clear Particles");

    current_y += section_spacing;
    draw_text_line("Simulation Params", true);
    current_y += section_spacing;

    draw_param_info_line("Sim Speed", SIMULATION_SPEED_KEY, SIMULATION_SPEED_DEFAULT, params_.simulation_speed);
    draw_param_info_line("Gravity X", GRAVITY_X_KEY, GRAVITY_X_DEFAULT, params_.gravity_x);
    draw_param_info_line("Gravity Y", GRAVITY_Y_KEY, GRAVITY_Y_DEFAULT, params_.gravity_y);
    draw_param_info_line("Edge Bounciness", EDGE_BOUNCINESS_KEY, EDGE_BOUNCINESS_DEFAULT, params_.edge_bounciness);
    draw_param_info_line("Interaction Radius", INTERACTION_RADIUS_KEY, INTERACTION_RADIUS_DEFAULT, params_.interaction_radius);
    draw_param_info_line("Rest Density", REST_DENSITY_KEY, REST_DENSITY_DEFAULT, params_.rest_density);
    draw_param_info_line("Stiffness", STIFFNESS_KEY, STIFFNESS_DEFAULT, params_.stiffness);
    draw_param_info_line("Near Stiffness", NEAR_STIFFNESS_KEY, NEAR_STIFFNESS_DEFAULT, params_.near_stiffness);
    draw_param_info_line("Linear Viscosity", LINEAR_VISCOSITY_KEY, LINEAR_VISCOSITY_DEFAULT, params_.linear_viscosity);
    draw_param_info_line("Quad Viscosity", QUADRATIC_VISCOSITY_KEY, QUADRATIC_VISCOSITY_DEFAULT, params_.quadratic_viscosity);
    draw_param_info_line("Plasticity", PLASTICITY_KEY, PLASTICITY_DEFAULT, params_.plasticity);
    draw_param_info_line("Yield Ratio", YIELD_RATIO_KEY, YIELD_RATIO_DEFAULT, params_.yield_ratio);
    draw_param_info_line("Spring Stiffness", SPRING_STIFFNESS_KEY, SPRING_STIFFNESS_DEFAULT, params_.spring_stiffness);

    current_y += section_spacing;
    draw_text_line("Controls Params", true);
    current_y += section_spacing;

    draw_param_info_line("Control Radius", CONTROL_RADIUS_KEY, CONTROL_RADIUS_DEFAULT, params_.control_radius);
    draw_param_info_line("Spawn Rate", PARTICLE_SPAWN_RATE_KEY, PARTICLE_SPAWN_RATE_DEFAULT, params_.particle_spawn_rate);

    current_y += section_spacing;
    draw_text_line("Visuals Params", true);
    current_y += section_spacing;

    draw_param_info_line("Base Size", BASE_PARTICLE_SIZE_KEY, BASE_PARTICLE_SIZE_DEFAULT, params_.base_particle_size);
    draw_param_info_line("Stress Size Mult", PARTICLE_STRESS_SIZE_MULTIPLIER_KEY, PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT, params_.particle_stress_size_multiplier);
    draw_param_info_line("Base Color", BASE_PARTICLE_COLOR_KEY, BASE_PARTICLE_COLOR_DEFAULT, params_.base_particle_color);
    draw_param_info_line("Stress Color Mult", PARTICLE_STRESS_COLOR_MULTIPLIER_KEY, PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT, params_.particle_stress_color_multiplier);
}
