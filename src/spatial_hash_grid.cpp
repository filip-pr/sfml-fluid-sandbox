
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
