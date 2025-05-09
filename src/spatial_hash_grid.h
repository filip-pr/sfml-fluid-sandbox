#ifndef SPATIAL_HASH_GRID_H
#define SPATIAL_HASH_GRID_H

#include <unordered_map>
#include <vector>

#include "utils.h"

template <typename T>
class SpatialHashGrid
{
public:
    void update(std::vector<T> &objects, size_t cell_size);

    std::vector<T *> query(sf::Vector2f center, float radius) const;

private:
    std::unordered_map<size_t, std::vector<T *>> grid_;
    size_t cell_size_ = 1;
    size_t max_cell_size_ = 0;

    void insert(std::vector<T> &objects);
    void clear();

    size_t hash_position(sf::Vector2f position) const;
    size_t hash_cell(size_t cell_x, size_t cell_y) const;
};

template <typename T>
inline size_t SpatialHashGrid<T>::hash_position(const sf::Vector2f position) const
{
    size_t cell_x = (position.x >= 0) ? static_cast<size_t>(position.x / cell_size_) : 0;
    size_t cell_y = (position.y >= 0) ? static_cast<size_t>(position.y / cell_size_) : 0;
    return hash_cell(cell_x, cell_y);
}

template <typename T>
inline size_t SpatialHashGrid<T>::hash_cell(size_t cell_x, size_t cell_y) const
{
    return cell_x + cell_y * utils::HASH_PRIME;
}

template <typename T>
inline void SpatialHashGrid<T>::clear()
{
    grid_.clear();
}

template <typename T>
inline void SpatialHashGrid<T>::insert(std::vector<T> &objects)
{
    size_t new_max_cell_size = 0;
    for (auto &&object : objects)
    {
        size_t key = hash_position(object.position);
        auto &cell = grid_[key];
        if (cell.empty())
        {
            cell.reserve(max_cell_size_);
        }
        cell.push_back(&object);
    }
    for (auto &&cell : grid_)
    {
        new_max_cell_size = std::max(new_max_cell_size, cell.second.size());
    }
    max_cell_size_ = new_max_cell_size;
}

template <typename T>
inline std::vector<T *> SpatialHashGrid<T>::query(sf::Vector2f center, float radius) const
{
    if (cell_size_ == 0)
    {
        return std::vector<T *>();
    }

    const float radius_sq = radius * radius;

    size_t min_cell_x = (center.x - radius) > 0 ? static_cast<size_t>(center.x - radius) / cell_size_ : 0;
    size_t max_cell_x = (center.x + radius) > 0 ? static_cast<size_t>(center.x + radius) / cell_size_ : 0;
    size_t min_cell_y = (center.y - radius) > 0 ? static_cast<size_t>(center.y - radius) / cell_size_ : 0;
    size_t max_cell_y = (center.y + radius) > 0 ? static_cast<size_t>(center.y + radius) / cell_size_ : 0;

    std::vector<T *> result;

    result.reserve((max_cell_x - min_cell_x + 1) * (max_cell_y - min_cell_y + 1) * max_cell_size_);

    for (size_t x = min_cell_x; x <= max_cell_x; ++x)
    {
        for (size_t y = min_cell_y; y <= max_cell_y; ++y)
        {
            size_t key = hash_cell(x, y);

            auto it = grid_.find(key);
            if (it == grid_.end())
                continue;

            for (const auto &object : it->second)
            {
                if (utils::distance_sq(center, object->position) <= radius_sq)
                {
                    result.push_back(object);
                }
            }
        }
    }
    return result;
}

template <typename T>
inline void SpatialHashGrid<T>::update(std::vector<T> &objects, size_t cell_size)
{
    clear();
    cell_size_ = cell_size;
    insert(objects);
}

#endif
