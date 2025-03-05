

#include "fluid_sandbox.h"

void FluidSandbox::update(float dt)
{
    for (auto &&particle : particles_)
    {
        particle.velocity.y += GRAVITY * dt;
        particle.update(dt);
        if (particle.position.x - PARTICLE_RADIUS < 0)
        {
            particle.position.x = PARTICLE_RADIUS;
            particle.velocity.x = -particle.velocity.x;
        }
        if (particle.position.x + PARTICLE_RADIUS > size_.x)
        {
            particle.position.x = size_.x - PARTICLE_RADIUS;
            particle.velocity.x = -particle.velocity.x;
        }
        if (particle.position.y - PARTICLE_RADIUS < 0)
        {
            particle.position.y = PARTICLE_RADIUS;
            particle.velocity.y = -particle.velocity.y;
        }
        if (particle.position.y + PARTICLE_RADIUS > size_.y)
        {
            particle.position.y = size_.y - PARTICLE_RADIUS;
            particle.velocity.y = -particle.velocity.y;
        }

    }
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::CircleShape circle(PARTICLE_RADIUS);
    for (auto &&particle : particles_)
    {
        circle.setPosition(particle.position);
        target.draw(circle, states);
    }
}
