
#ifndef SPATIAL_HASH_GRID_H
#define SPATIAL_HASH_GRID_H

#include <unordered_map>
#include <deque>

#include "particle.h"

constexpr size_t HASH_PRIME = 1168639;

inline constexpr char PARTICLE_NOT_FOUND_ERROR_MESSAGE[] = "Particle not found in grid";

class SpatialHashGrid
{
public:
    SpatialHashGrid(size_t cell_size) : cell_size_(cell_size) {}

    void insert(Particle *particle);
    void remove(Particle *particle);

private:
    std::unordered_map<size_t, std::deque<Particle*>> grid_;
    size_t cell_size_;

    size_t hash(sf::Vector2f position) const;
};

inline size_t SpatialHashGrid::hash(sf::Vector2f position) const
{
    return static_cast<size_t>(position.x) / cell_size_ + static_cast<size_t>(position.y) / cell_size_ * HASH_PRIME;
}

#endif
