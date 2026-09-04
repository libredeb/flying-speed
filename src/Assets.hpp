#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <functional>
#include <string>

namespace flying {

using ProgressFn = std::function<void(int current, int total)>;

struct Assets {
    SDL_Texture* bird = nullptr;
    SDL_Texture* clouds = nullptr;
    SDL_Texture* farBuildings = nullptr;
    SDL_Texture* nearBuildings = nullptr;
    SDL_Texture* trees = nullptr;
    SDL_Texture* greenHead = nullptr;
    SDL_Texture* greenBody = nullptr;
    SDL_Texture* blueHead = nullptr;
    SDL_Texture* blueBody = nullptr;
    SDL_Texture* yellowHead = nullptr;
    SDL_Texture* yellowBody = nullptr;
    SDL_Texture* trailDot = nullptr;
    SDL_Texture* sky = nullptr;

    TTF_Font* fontTitle = nullptr;
    TTF_Font* fontUi = nullptr;
    TTF_Font* fontScore = nullptr;

    std::string root;

    static constexpr int kLoadSteps = 16;

    bool load(SDL_Renderer* renderer, ProgressFn progress = nullptr);
    void unload();
};

}  // namespace flying
