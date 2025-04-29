
#include "utils.h"
#include "spatial_hash_grid.h"


#include <cmath>

void SpatialHashGrid::insert(Particle *particle)
{
    size_t key = hash(particle->position);
    grid_[key].emplace_back(particle);
}

void SpatialHashGrid::remove(Particle *particle)
{
    size_t key = hash(particle->position);
    auto &cell = grid_[key];
    for (auto it = cell.begin(); it != cell.end(); it++)
    {
        if (*it == particle)
        {
            cell.erase(it);
            return;
        }
    }
    throw std::runtime_error(PARTICLE_NOT_FOUND_ERROR_MESSAGE);
}

inline std::vector<size_t> SpatialHashGrid::hash_cells(sf::Vector2f center, float radius) const
{
    std::vector<size_t> keys;

    size_t min_cell_x = (center.x - radius) > 0 ? static_cast<size_t>(center.x - radius) / cell_size_ : 0;
    size_t max_cell_x = (center.x + radius) > 0 ? static_cast<size_t>(center.x + radius) / cell_size_ : 0;
    size_t min_cell_y = (center.y - radius) > 0 ? static_cast<size_t>(center.y - radius) / cell_size_ : 0;
    size_t max_cell_y = (center.y - radius) > 0 ? static_cast<size_t>(center.y + radius) / cell_size_ : 0;

    for (size_t x = min_cell_x; x <= max_cell_x; ++x)
    {
        for (size_t y = min_cell_y; y <= max_cell_y; ++y)
        {
            keys.push_back(hash_cell(x, y));
        }
    }
    return keys;
}

std::vector<Particle *> SpatialHashGrid::query(sf::Vector2f center, float radius) const
{
    std::vector<Particle *> result;

    auto keys = hash_cells(center, radius);
    for (const auto &key : keys)
    {
        auto it = grid_.find(key);
        if (it == grid_.end())
            continue;

        for (const auto &particle : it->second)
        {
            float distance = utils::distance(center, particle->position);
            if (distance <= radius)
            {
                result.push_back(particle);
            }
        }
    }

    return result;
}

void SpatialHashGrid::clear()
{
    grid_.clear();
}

void SpatialHashGrid::batch_insert(std::vector<Particle>& particles) // TODO this could probably be optimized
{
    for (auto&& particle : particles)
    {
        insert(&particle);
    }
}
