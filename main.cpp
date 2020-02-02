#include <ctime>
#include <iostream>
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

int SDLMAIN_DECLSPEC main(int argc, char *argv[])
{
    if (const int ec = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER); ec < 0)
    {
        std::cerr << "SDL_Init (error " << ec << "): " << SDL_GetError() << std::endl;
        return ec;
    }

    srand(time(nullptr));
    std::fill_n(&map[0][0], width * height, 0xFF);
    for (auto& row : map) for (auto& cell : row)
        if (rand() < (RAND_MAX / 2))
            cell = rand() % std::size(colors);        

    SDL_Window * window;
    SDL_Renderer * renderer;
    if (SDL_CreateWindowAndRenderer(rWidth, rHeight, 0, &window, &renderer) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    for(;;)
    {
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            return EXIT_SUCCESS;
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        drawBackground(renderer);
        drawMap(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
