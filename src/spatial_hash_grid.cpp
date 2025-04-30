#include "utils.h"
#include "spatial_hash_grid.h"

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

std::vector<Particle *> SpatialHashGrid::query(sf::Vector2f center, float radius) const
{
    std::vector<Particle *> result;
    const float radius_sq = radius * radius; // Precompute squared radius

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

void SpatialHashGrid::clear()
{
    grid_.clear();
}

void SpatialHashGrid::batch_insert(std::vector<Particle> &particles)
{
    for (auto &&particle : particles)
    {
        insert(&particle);
    }
}
