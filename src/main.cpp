#include "Config.hpp"
#include "Game.hpp"
#include "Input.hpp"
#include "Util.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace {

void printHelp() {
    std::printf(
        "Flying Speed\n"
        "  --windowed    Ventana 720x720 (por defecto: pantalla completa)\n"
        "  --help        Esta ayuda\n"
        "Controles: ESPACIO / flecha arriba / W para volar, P pausa, ESC salir\n");
}

SDL_Window* createWindow(bool windowed) {
    Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN;
    if (!windowed) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    return SDL_CreateWindow("Flying Speed", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            flying::kLogicalW, flying::kLogicalH, flags);
}

}  // namespace

int main(int argc, char** argv) {
    bool windowed = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            printHelp();
            return 0;
        }
        if (std::strcmp(argv[i], "--windowed") == 0) {
            windowed = true;
        }
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#if SDL_VERSION_ATLEAST(2, 0, 16)
    SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, "letterbox");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 22)
    const std::string controllerDb = flying::findAssetsRoot() + "/gamecontrollerdb.txt";
    SDL_SetHint(SDL_HINT_GAMECONTROLLERCONFIG_FILE, controllerDb.c_str());
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    flying::seedRng();

    SDL_Window* window = createWindow(windowed);
    if (!window && !windowed) {
        std::fprintf(stderr, "Pantalla completa no disponible (%s), usando ventana.\n",
                     SDL_GetError());
        window = createWindow(true);
        windowed = true;
    }
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface* icon = IMG_Load((flying::findAssetsRoot() + "/bird.png").c_str());
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, flying::kLogicalW, flying::kLogicalH);
    SDL_RenderSetIntegerScale(renderer, SDL_FALSE);
    if (!windowed) {
        SDL_ShowCursor(SDL_DISABLE);
    }

    flying::Input input;
    input.init();

    auto renderLoading = [&](int step, int total) {
        SDL_PumpEvents();

        SDL_SetRenderDrawColor(renderer, 18, 28, 58, 255);
        SDL_RenderClear(renderer);

        const float pct = (total > 0)
            ? static_cast<float>(step) / static_cast<float>(total)
            : 0.f;

        constexpr int kBarW = 340;
        constexpr int kBarH = 12;
        constexpr int kBarX = (flying::kLogicalW - kBarW) / 2;
        constexpr int kBarY = flying::kLogicalH / 2 + 60;
        constexpr int kPad = 2;

        SDL_Rect outline{kBarX - kPad, kBarY - kPad,
                         kBarW + kPad * 2, kBarH + kPad * 2};
        SDL_SetRenderDrawColor(renderer, 60, 80, 120, 255);
        SDL_RenderFillRect(renderer, &outline);

        SDL_Rect bg{kBarX, kBarY, kBarW, kBarH};
        SDL_SetRenderDrawColor(renderer, 10, 16, 32, 255);
        SDL_RenderFillRect(renderer, &bg);

        const int fillW = static_cast<int>(pct * kBarW);
        if (fillW > 0) {
            SDL_Rect fill{kBarX, kBarY, fillW, kBarH};
            SDL_SetRenderDrawColor(renderer, 255, 214, 90, 255);
            SDL_RenderFillRect(renderer, &fill);
        }

        SDL_RenderPresent(renderer);
    };

    flying::Game game;
    if (!game.init(renderer, renderLoading)) {
        std::fprintf(stderr, "No se pudieron cargar los recursos.\n");
        input.shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    Uint64 prev = SDL_GetPerformanceCounter();
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());

    while (running) {
        const Uint64 now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>((now - prev) / freq);
        prev = now;
        if (dt > 0.05f) {
            dt = 0.05f;
        }

        input.beginFrame();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            input.handleEvent(e);
        }
        if (input.quitRequested()) {
            running = false;
        }

        game.handleInput(input);
        game.update(dt);
        game.render();
        SDL_RenderPresent(renderer);
    }

    game.shutdown();
    input.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
