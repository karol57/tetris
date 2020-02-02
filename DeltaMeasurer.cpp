#include "DeltaMeasurer.hpp"
#include <stdexcept>
#include <SDL2/SDL.h>

bool DeltaMeasurer::g_initialized = false;
Uint64 DeltaMeasurer::g_performanceFrequency;

void DeltaMeasurer::g_init()
{
    if (SDL_WasInit(SDL_INIT_TIMER) != SDL_INIT_TIMER)
        throw std::runtime_error("SDL_TIMER not initialized.");
    g_performanceFrequency = SDL_GetPerformanceFrequency();
    g_initialized = true;
}

DeltaMeasurer::DeltaMeasurer()
    : m_delta{ 0 }
    , m_last{ SDL_GetPerformanceCounter() }
{
    if (!g_initialized)
        g_init();
}

void DeltaMeasurer::tick() noexcept
{
    const Uint64 current = SDL_GetPerformanceCounter();
    m_delta = current - m_last;
    m_last = current;
}
