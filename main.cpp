#include <cassert>
#include <ctime>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

struct Color { int r, g, b, a; };

constexpr int blockSize = 16;
constexpr int width = 16;
constexpr int height = 32;
constexpr int rWidth = width * (blockSize + 1) - 1;
constexpr int rHeight = height * (blockSize + 1) - 1;
Color colors[] = {
    { 128, 0, 128, SDL_ALPHA_OPAQUE },
    { 255, 0, 0, SDL_ALPHA_OPAQUE },
    { 257, 127, 0, SDL_ALPHA_OPAQUE },
    { 0, 255, 0, SDL_ALPHA_OPAQUE },
    { 0, 255, 255, SDL_ALPHA_OPAQUE },
    { 0, 0, 255, SDL_ALPHA_OPAQUE },
    { 255, 255, 255, SDL_ALPHA_OPAQUE }
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

class Block
{
public:
    Block(size_t width, size_t height, std::vector<bool>&& fields)
        : m_fields{std::move(fields)}
        , m_width{width}, m_height{height}
        , m_x{::width / 2 - m_width / 2}, m_y{5u}
    {
        assert(m_width * m_height == m_fields.size());
    }

    void genColor() { m_color = rand() % std::size(colors); }
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
        if (m_x)
        {
            hide();
            --m_x;
            show();
        }
    }
    void moveRight()
    {
        if(m_x + m_width < width)
        {
            hide();
            ++m_x;
            show();
        }
    }
private:
    std::vector<bool> m_fields;
    size_t m_width;
    size_t m_height;

    size_t m_x;
    size_t m_y;
    size_t m_color;
};

const Block sblock { 3, 2, { 0, 1, 1,
                             1, 1, 0 } };

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
    if (SDL_CreateWindowAndRenderer(rWidth, rHeight, 0, &window, &renderer) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    Block block = sblock;
    block.genColor();
    block.show();
    for(;;)
    {
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
                    case SDL_SCANCODE_LEFT: block.moveLeft(); break;
                    case SDL_SCANCODE_RIGHT: block.moveRight(); break;
                }
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
