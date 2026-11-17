#pragma once
#include <vector>
#include <cstddef>
#include <algorithm>

namespace obby {

/**
 * @brief Generic object pool class template for high-frequency game objects.
 * Demonstrates:
 * - C++ template programming (template <typename T>)
 * - Dynamic capacity growth with pre-allocation
 * - Reusable slot management avoiding runtime heap allocations
 * - STL container encapsulation (std::vector)
 */
template <typename T>
class ObjectPool {
private:
    std::vector<T> m_pool;
    size_t m_activeCount;

public:
    explicit ObjectPool(size_t initialCapacity = 256)
        : m_pool(initialCapacity), m_activeCount(0) {}

    ~ObjectPool() = default;

    // Non-copyable, movable
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&& other) noexcept = default;
    ObjectPool& operator=(ObjectPool&& other) noexcept = default;

    T* acquire() {
        if (m_activeCount >= m_pool.size()) {
            m_pool.resize(m_pool.empty() ? 64 : m_pool.size() * 2);
        }
        return &m_pool[m_activeCount++];
    }

    void release(size_t index) {
        if (index < m_activeCount) {
            // Swap with last active element for O(1) removal
            m_activeCount--;
            std::swap(m_pool[index], m_pool[m_activeCount]);
        }
    }

    void clear() {
        m_activeCount = 0;
    }

    size_t activeCount() const { return m_activeCount; }
    size_t capacity() const { return m_pool.size(); }
    bool empty() const { return m_activeCount == 0; }

    T& operator[](size_t index) { return m_pool[index]; }
    const T& operator[](size_t index) const { return m_pool[index]; }

    auto begin() { return m_pool.begin(); }
    auto end() { return m_pool.begin() + m_activeCount; }
    auto begin() const { return m_pool.cbegin(); }
    auto end() const { return m_pool.cbegin() + m_activeCount; }
};

} // namespace obby

template <typename T>
using ObjectPool = obby::ObjectPool<T>;
