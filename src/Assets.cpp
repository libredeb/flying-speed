#include "Assets.hpp"

#include "Config.hpp"
#include "Util.hpp"

#include <SDL_image.h>

#include <cmath>
#include <cstdio>

namespace flying {
namespace {

SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", path.c_str(), IMG_GetError());
    }
    return tex;
}

SDL_Texture* makeSky(SDL_Renderer* renderer) {
    constexpr int w = 4;
    constexpr int h = kLogicalH;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        return nullptr;
    }

    auto* pixels = static_cast<Uint32*>(surface->pixels);
    for (int y = 0; y < h; ++y) {
        const float t = static_cast<float>(y) / static_cast<float>(h - 1);
        const float t2 = t * t;
        const Uint8 r = static_cast<Uint8>(lerpf(12.f, 48.f, t));
        const Uint8 g = static_cast<Uint8>(lerpf(18.f, 72.f, t));
        const Uint8 b = static_cast<Uint8>(lerpf(42.f, 118.f, t2));
        const Uint32 color = SDL_MapRGBA(surface->format, r, g, b, 255);
        for (int x = 0; x < w; ++x) {
            pixels[y * w + x] = color;
        }
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (tex) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
    }
    return tex;
}

SDL_Texture* makeTrailDot(SDL_Renderer* renderer) {
    constexpr int size = 64;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        return nullptr;
    }

    const float c = (size - 1) * 0.5f;
    auto* pixels = static_cast<Uint32*>(surface->pixels);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const float dx = static_cast<float>(x) - c;
            const float dy = static_cast<float>(y) - c;
            const float d = std::sqrt(dx * dx + dy * dy) / (c * 0.98f);
            float a = clampf(1.f - d, 0.f, 1.f);
            a = a * a * (3.f - 2.f * a);
            const Uint8 alpha = static_cast<Uint8>(a * 255.f);
            pixels[y * size + x] = SDL_MapRGBA(surface->format, 255, 255, 255, alpha);
        }
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (tex) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
    }
    return tex;
}

}  // namespace

bool Assets::load(SDL_Renderer* renderer) {
    root = findAssetsRoot();

    auto p = [&](const char* name) { return root + "/" + name; };

    bird = loadTexture(renderer, p("bird.png"));
    clouds = loadTexture(renderer, p("clouds.png"));
    farBuildings = loadTexture(renderer, p("far_buildings.png"));
    nearBuildings = loadTexture(renderer, p("near_buildings.png"));
    trees = loadTexture(renderer, p("trees.png"));
    greenHead = loadTexture(renderer, p("green_head_pipe.png"));
    greenBody = loadTexture(renderer, p("green_body_pipe.png"));
    blueHead = loadTexture(renderer, p("blue_head_pipe.png"));
    blueBody = loadTexture(renderer, p("blue_body_pipe.png"));
    yellowHead = loadTexture(renderer, p("yellow_head_pipe.png"));
    yellowBody = loadTexture(renderer, p("yellow_body_pipe.png"));
    sky = makeSky(renderer);
    trailDot = makeTrailDot(renderer);

    auto nearest = [](SDL_Texture* tex) {
        if (tex) {
            SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
        }
    };
    nearest(greenHead);
    nearest(greenBody);
    nearest(blueHead);
    nearest(blueBody);
    nearest(yellowHead);
    nearest(yellowBody);

    fontTitle = TTF_OpenFont(p("Bird.ttf").c_str(), 112);
    fontUi = TTF_OpenFont(p("Bird.ttf").c_str(), 52);
    fontScore = TTF_OpenFont(p("Bird.ttf").c_str(), 128);
    if (!fontTitle || !fontUi || !fontScore) {
        std::fprintf(stderr, "Aviso: no se pudo abrir Bird.ttf (%s)\n", TTF_GetError());
    }

    return bird && clouds && farBuildings && nearBuildings && trees && greenHead && greenBody &&
           blueHead && blueBody && yellowHead && yellowBody && sky && trailDot;
}

void Assets::unload() {
    auto destroy = [](SDL_Texture*& t) {
        if (t) {
            SDL_DestroyTexture(t);
            t = nullptr;
        }
    };
    destroy(bird);
    destroy(clouds);
    destroy(farBuildings);
    destroy(nearBuildings);
    destroy(trees);
    destroy(greenHead);
    destroy(greenBody);
    destroy(blueHead);
    destroy(blueBody);
    destroy(yellowHead);
    destroy(yellowBody);
    destroy(trailDot);
    destroy(sky);

    auto closeFont = [](TTF_Font*& f) {
        if (f) {
            TTF_CloseFont(f);
            f = nullptr;
        }
    };
    closeFont(fontTitle);
    closeFont(fontUi);
    closeFont(fontScore);
}

}  // namespace flying
