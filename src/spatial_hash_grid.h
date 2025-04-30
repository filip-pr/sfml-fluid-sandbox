#ifndef SPATIAL_HASH_GRID_H
#define SPATIAL_HASH_GRID_H

#include <unordered_map>
#include <vector>
#include <generator>

#include "particle.h"

constexpr size_t HASH_PRIME = 1168639;

inline constexpr char PARTICLE_NOT_FOUND_ERROR_MESSAGE[] = "Particle not found in grid";

class SpatialHashGrid
{
public:
    SpatialHashGrid(size_t cell_size) : cell_size_(cell_size) {}

    void insert(Particle *particle);
    void batch_insert(std::vector<Particle>& particles);
    void remove(Particle *particle);
    void clear();

    std::vector<Particle *> query(sf::Vector2f center, float radius) const;

private:
    std::unordered_map<size_t, std::vector<Particle *>> grid_;
    size_t cell_size_;

    size_t hash(const sf::Vector2f &position) const;
    size_t hash_cell(size_t cell_x, size_t cell_y) const;
    std::generator<size_t> hash_cells(sf::Vector2f center, float radius) const;
};

inline size_t SpatialHashGrid::hash(const sf::Vector2f &position) const
{
    size_t cell_x = (position.x >= 0) ? static_cast<size_t>(position.x / cell_size_) : 0;
    size_t cell_y = (position.y >= 0) ? static_cast<size_t>(position.y / cell_size_) : 0;
    return hash_cell(cell_x, cell_y);
}

inline size_t SpatialHashGrid::hash_cell(size_t cell_x, size_t cell_y) const
{
    return cell_x + cell_y * HASH_PRIME;
}

#endif
