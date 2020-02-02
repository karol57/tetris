#include <cassert>
#include <ctime>
#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include "DeltaMeasurer.hpp"

struct Color { int r, g, b, a; };

constexpr int blockSize = 16;
constexpr int width = 16;
constexpr int height = 32;
constexpr int rWidth = width * (blockSize + 1) - 1;
constexpr int rHeight = height * (blockSize + 1) - 1;
Color colors[] = {
    { 128, 0, 128, SDL_ALPHA_OPAQUE },  // purple
    { 255, 0, 0, SDL_ALPHA_OPAQUE },    // red
    { 255, 127, 0, SDL_ALPHA_OPAQUE },  // orange
    { 0, 255, 0, SDL_ALPHA_OPAQUE },    // green
    { 0, 255, 255, SDL_ALPHA_OPAQUE },  // cyan
    { 0, 0, 255, SDL_ALPHA_OPAQUE },    // blue
    { 255, 255, 255, SDL_ALPHA_OPAQUE } // white
};
uint8_t map[height][width];

static
void drawBackground(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 0x1F, 0x1F, 0x1F, SDL_ALPHA_OPAQUE);
    for (int x = 0; x < (width - 1); ++x)
    {
        const int rx = (x+1) * (blockSize+1) - 1;
        SDL_RenderDrawLine(renderer, rx, 0u, rx, rHeight);
    }
    for (int y = 0; y < (height - 1); ++y)
    {
        const int ry = (y+1) * (blockSize+1) - 1;
        SDL_RenderDrawLine(renderer, 0u, ry, rWidth, ry);
    }
}

static
void drawMap(SDL_Renderer* renderer)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const uint8_t cell = map[y][x];
            if (cell != 0xFF)
            {
                const Color& c = colors[cell];
                SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
                const SDL_Rect rc{x * (blockSize+1), y * (blockSize+1), blockSize, blockSize};
                SDL_RenderFillRect(renderer, &rc);
            }
        }
    }
}

size_t checkMap()
{
    size_t cleared = 0;
    for (int y = 0; y < height; ++y)
    {
        if (std::all_of(std::begin(map[y]), std::end(map[y]),
            [](auto&& cell) { return cell != 0xFF; }))
        {
            ++cleared;
            if (y != 0)
                std::copy_backward(&map[0][0], &map[y][0], &map[y+1][0]);
        }
    }

    std::fill_n(&map[0][0], cleared * width, 0xFFu);
    return cleared;
}

class Block
{
public:
    Block(size_t color, int width, int height, std::vector<bool>&& fields)
        : m_fields{std::move(fields)}
        , m_width{width}, m_height{height}
        , m_x{::width / 2 - m_width / 2}, m_y{0}
        , m_color{ color }
    {
        assert(m_width * m_height == m_fields.size());
    }

    void show()
    {
        for (size_t y = 0; y < m_height; ++y) for (size_t x = 0; x < m_width; ++x)
            if (m_fields[y * m_width + x])
                map[m_y + y][m_x + x] = m_color;
    }

    void hide()
    {
        for (size_t y = 0; y < m_height; ++y) for (size_t x = 0; x < m_width; ++x)
            if (m_fields[y * m_width + x])
                map[m_y + y][m_x + x] = 0xFF;
    }


    void moveLeft()
    {
        hide();
        --m_x;
        if (isColliding())
            ++m_x;
        show();
    }
    void moveRight()
    {
        hide();
        ++m_x;
        if (isColliding())
            --m_x;
        show();
    }
    bool moveDown()
    {
        hide();
        ++m_y;

        if (isColliding())
        {
            --m_y;
            show();
            return false;
        }

        show();
        return true;
    }

    void rotateLeft()
    {
        hide();

        const std::vector<bool> old = m_fields;
        for (int y = 0; y < m_height; y++)
            for (int x = 0; x < m_width; x++)
                m_fields[(m_width - x - 1) * m_height + y] = old[y * m_width + x];
        std::swap(m_width, m_height);

        if (isColliding())
        {
            std::swap(m_width, m_height);
            m_fields = old;
        }
        show();
    }

    void rotateRight()
    {
        hide();

        const std::vector<bool> old = m_fields;
        // abc -> gda
        // def -> heb
        // ghi -> ifc
        for (int y = 0; y < m_height; y++)
            for (int x = 0; x < m_width; x++)
                m_fields[x * m_height + (m_height - y - 1)] = old[y * m_width + x];
        std::swap(m_width, m_height);

        if (isColliding())
        {
            std::swap(m_width, m_height);
            m_fields = old;
        }
        show();
    }

    bool isColliding()
    {
        if(m_x < 0 || m_y < 0 || m_x + m_width > width || m_y + m_height > height)
            return true;

        for (size_t y = 0; y < m_height; ++y)
            for (size_t x = 0; x < m_width; ++x)
                if (m_fields[y * m_width + x] && map[m_y + y][m_x + x] != 0xFF)
                    return true;
        return false;
    }
private:
    std::vector<bool> m_fields;
    int m_width;
    int m_height;

    int m_x;
    int m_y;
    size_t m_color;
};

const Block blocks[] = {
    { 0, 3, 2, { 0, 1, 1,
                 1, 1, 0 } },

    { 1, 3, 2, { 1, 1, 0,
                 0, 1, 1 } },

    { 2, 3, 2, { 0, 1, 0,
                 1, 1, 1 } },

    { 3, 2, 2, { 1, 1,
                 1, 1 } },

    { 4, 4, 1, { 1, 1, 1, 1 } },

    { 5, 4, 2, { 1, 0, 0, 0,
                 1, 1, 1, 1 } },

    { 6, 4, 2, { 0, 0, 0, 1,
                 1, 1, 1, 1 } },
};

static const Block& getRandomBlock()
{
    return blocks[rand() % std::size(blocks)];
}

int SDLMAIN_DECLSPEC main(int argc, char *argv[])
{
    if (const int ec = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER); ec < 0)
    {
        std::cerr << "SDL_Init (error " << ec << "): " << SDL_GetError() << std::endl;
        return ec;
    }

    srand(time(nullptr));
    std::fill_n(&map[0][0], width * height, 0xFF);

    SDL_Window * window;
    SDL_Renderer * renderer;
    if (SDL_CreateWindowAndRenderer(rWidth, rHeight, SDL_WINDOW_OPENGL, &window, &renderer) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    DeltaMeasurer dm;
    const double blockSpeed = 0.25;
    double blockTimer = 0.0;
    Block block = getRandomBlock();
    block.show();
    for(;;dm.tick())
    {
        const double delta = dm.delta();
        char buffer[256];
        snprintf(buffer, 256, "FPS: %8.2f ", dm.fps());
        SDL_SetWindowTitle(window, buffer);

        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                break;

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_ESCAPE: goto end;

                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        block.moveLeft();
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        block.moveRight();
                        break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:
                        block.moveDown();
                        break;

                    case SDL_SCANCODE_E:
                    case SDL_SCANCODE_PAGEDOWN:
                        block.rotateRight();
                        break;
                    case SDL_SCANCODE_Q:
                    case SDL_SCANCODE_PAGEUP:
                        block.rotateLeft();
                        break;
                }
            }
        }

        for (blockTimer += delta; blockTimer > blockSpeed; blockTimer -= blockSpeed)
        {
            if (!block.moveDown())
            {
                checkMap();
                block = getRandomBlock();
                block.show();
                blockTimer = 0.0;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        drawBackground(renderer);
        drawMap(renderer);
        SDL_RenderPresent(renderer);
    }
    end:

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
