#ifndef SPATIAL_HASH_GRID_H
#define SPATIAL_HASH_GRID_H

#include <unordered_map>
#include <vector>

#include "utils.h"

/**
 * @brief A spatial hash grid for efficient neighbor searching.
 * @tparam T The type of objects to be stored in the grid (e.g., Particle, Object).
 * Type T must have a public `sf::Vector2f position` member.
 */
template <typename T>
class SpatialHashGrid
{
public:
    /**
     * @brief Updates the grid with a new set of objects and cell size.
     * Clears the existing grid and re-inserts all objects.
     * @param objects A vector of objects to populate the grid with.
     * @param cell_size The desired size for each grid cell. Should typically be
     * related to the interaction radius of the objects.
     */
    void update(std::vector<T> &objects, size_t cell_size);

    /**
     * @brief Queries the grid for objects within a given radius of a center point.
     * @param center The center point of the query circle.
     * @param radius The radius of the query circle.
     * @return A vector of pointers to objects found within the query radius.
     */
    std::vector<T *> query(sf::Vector2f center, float radius) const;

private:
    std::unordered_map<size_t, std::vector<T *>> grid_;
    size_t cell_size_ = 1;
    size_t max_cell_size_ = 0;

    /**
     * @brief Inserts objects into the grid.
     * @param objects The vector of objects to insert.
     */
    void insert(std::vector<T> &objects);

    /**
     * @brief Clears all objects from the grid.
     */
    void clear();

    /**
     * @brief Computes the hash key for a given position.
     * @param position The position to hash.
     * @return The hash key corresponding to the cell containing the position.
     */
    size_t hash_position(sf::Vector2f position) const;

    /**
     * @brief Computes the hash key for given cell coordinates.
     * @param cell_x The x-coordinate of the cell.
     * @param cell_y The y-coordinate of the cell.
     * @return The hash key for the cell.
     */
    size_t hash_cell(size_t cell_x, size_t cell_y) const;
};

template <typename T>
inline size_t SpatialHashGrid<T>::hash_position(const sf::Vector2f position) const
{
    // Ensure non-negative cell coordinates before casting
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
            cell.reserve(max_cell_size_*1.5f);
        }
        cell.push_back(&object);
    }
    for (auto &&cell_pair : grid_)
    {
        new_max_cell_size = std::max(new_max_cell_size, cell_pair.second.size());
    }
    max_cell_size_ = new_max_cell_size;
}

template <typename T>
inline std::vector<T *> SpatialHashGrid<T>::query(sf::Vector2f center, float radius) const
{
    if (cell_size_ == 0) // Avoid zero division
    {
        return std::vector<T *>();
    }

    const float radius_sq = radius * radius; // Use squared distance for efficiency

    size_t min_cell_x = (center.x - radius) > 0 ? static_cast<size_t>((center.x - radius) / cell_size_) : 0;
    size_t max_cell_x = (center.x + radius) > 0 ? static_cast<size_t>((center.x + radius) / cell_size_) : 0;
    size_t min_cell_y = (center.y - radius) > 0 ? static_cast<size_t>((center.y - radius) / cell_size_) : 0;
    size_t max_cell_y = (center.y + radius) > 0 ? static_cast<size_t>((center.y + radius) / cell_size_) : 0;

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

            for (const auto &object_ptr : it->second)
            {
                if (utils::distance_sq(center, object_ptr->position) <= radius_sq)
                {
                    result.push_back(object_ptr);
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
