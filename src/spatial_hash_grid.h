#ifndef SPATIAL_HASH_GRID_H
#define SPATIAL_HASH_GRID_H

#include <unordered_map>
#include <vector>
#include <generator>

#include "utils.h"
#include "particle.h"

constexpr size_t HASH_PRIME = 1168639;

class SpatialHashGrid
{
public:
    void update(std::vector<Particle> &particles, size_t cell_size);

    std::vector<Particle *> query(sf::Vector2f center, float radius) const;

private:
    std::unordered_map<size_t, std::vector<Particle *>> grid_;
    size_t cell_size_ = 1;

    void insert(std::vector<Particle> &particles);
    void clear();

    size_t hash(sf::Vector2f position) const;
    size_t hash_cell(size_t cell_x, size_t cell_y) const;
    std::generator<size_t> hash_cells(sf::Vector2f center, float radius) const;
};

inline size_t SpatialHashGrid::hash(const sf::Vector2f position) const
{
    size_t cell_x = (position.x >= 0) ? static_cast<size_t>(position.x / cell_size_) : 0;
    size_t cell_y = (position.y >= 0) ? static_cast<size_t>(position.y / cell_size_) : 0;
    return hash_cell(cell_x, cell_y);
}

inline size_t SpatialHashGrid::hash_cell(size_t cell_x, size_t cell_y) const
{
    return cell_x + cell_y * HASH_PRIME;
}

inline std::generator<size_t> SpatialHashGrid::hash_cells(sf::Vector2f center, float radius) const
{
    size_t min_cell_x = (center.x - radius) > 0 ? static_cast<size_t>(center.x - radius) / cell_size_ : 0;
    size_t max_cell_x = (center.x + radius) > 0 ? static_cast<size_t>(center.x + radius) / cell_size_ : 0;
    size_t min_cell_y = (center.y - radius) > 0 ? static_cast<size_t>(center.y - radius) / cell_size_ : 0;
    size_t max_cell_y = (center.y + radius) > 0 ? static_cast<size_t>(center.y + radius) / cell_size_ : 0;

    for (size_t x = min_cell_x; x <= max_cell_x; ++x)
    {
        for (size_t y = min_cell_y; y <= max_cell_y; ++y)
        {
            co_yield hash_cell(x, y);
        }
    }
}

inline void SpatialHashGrid::clear()
{
    grid_.clear();
}

inline void SpatialHashGrid::insert(std::vector<Particle> &particles)
{
    for (auto &&particle : particles)
    {
        size_t key = hash(particle.position);
        grid_[key].emplace_back(&particle);
    }
}

inline std::vector<Particle *> SpatialHashGrid::query(sf::Vector2f center, float radius) const
{
    std::vector<Particle *> result;
    const float radius_sq = radius * radius;

    auto keys = hash_cells(center, radius);
    for (const auto &key : keys)
    {
        auto it = grid_.find(key);
        if (it == grid_.end())
            continue;

        for (const auto &particle : it->second)
        {
            if (utils::distance_sq(center, particle->position) <= radius_sq)
            {
                result.push_back(particle);
            }
        }
    }

    return result;
}

inline void SpatialHashGrid::update(std::vector<Particle> &particles, size_t cell_size)
{
    clear();
    cell_size_ = cell_size;
    insert(particles);
}

#endif
