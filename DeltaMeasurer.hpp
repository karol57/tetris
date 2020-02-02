#pragma once
#include <cstdint>
#include <limits>

class DeltaMeasurer
{
public:
    DeltaMeasurer();

    double delta() const noexcept
    {
        return static_cast<double>(m_delta) / static_cast<double>(g_performanceFrequency);
    }
    double fps() const noexcept
    {
        if (m_delta == 0)
            return std::numeric_limits<double>::quiet_NaN();
        return g_performanceFrequency / m_delta;
    }
    void tick() noexcept;
private:
    uint64_t m_delta;
    uint64_t m_last;

    static void g_init();
    static bool g_initialized;
    static uint64_t g_performanceFrequency;
};