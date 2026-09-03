#include "Input.hpp"

#include "Util.hpp"

#include <SDL.h>

#include <cstdio>
#include <string>

namespace flying {
namespace {

constexpr Sint16 kStickFlapThreshold = -16000;

}  // namespace

bool Input::init() {
    pads_.fill(nullptr);

    const std::string dbPath = findAssetsRoot() + "/gamecontrollerdb.txt";
    const int added = SDL_GameControllerAddMappingsFromFile(dbPath.c_str());
    if (added < 0) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", dbPath.c_str(), SDL_GetError());
    }

    SDL_GameControllerEventState(SDL_ENABLE);

    const int n = SDL_NumJoysticks();
    for (int i = 0; i < n; ++i) {
        if (SDL_IsGameController(i)) {
            openController(i);
        }
    }
    return true;
}

void Input::shutdown() {
    for (auto& pad : pads_) {
        if (pad) {
            SDL_GameControllerClose(pad);
            pad = nullptr;
        }
    }
}

void Input::beginFrame() {
    flapPressed_ = false;
    confirmPressed_ = false;
    pausePressed_ = false;
}

void Input::onFlap() {
    flapPressed_ = true;
}

void Input::onConfirm() {
    confirmPressed_ = true;
}

void Input::onStickY(Sint16 value) {
    const bool up = value <= kStickFlapThreshold;
    if (up && !stickFlapHeld_) {
        onFlap();
        onConfirm();
    }
    stickFlapHeld_ = up;
}

void Input::openController(int joystickIndex) {
    for (auto& slot : pads_) {
        if (slot) {
            continue;
        }
        slot = SDL_GameControllerOpen(joystickIndex);
        return;
    }
}

void Input::closeController(SDL_JoystickID instanceId) {
    for (auto& slot : pads_) {
        if (!slot) {
            continue;
        }
        SDL_Joystick* js = SDL_GameControllerGetJoystick(slot);
        if (js && SDL_JoystickInstanceID(js) == instanceId) {
            SDL_GameControllerClose(slot);
            slot = nullptr;
            return;
        }
    }
}

int Input::padCount() const {
    int n = 0;
    for (auto* pad : pads_) {
        if (pad) {
            ++n;
        }
    }
    return n;
}

void Input::handleEvent(const SDL_Event& e) {
    switch (e.type) {
        case SDL_QUIT:
            quit_ = true;
            break;

        case SDL_KEYDOWN:
            if (e.key.repeat) {
                break;
            }
            switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    quit_ = true;
                    break;
                case SDL_SCANCODE_SPACE:
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_K:
                    onFlap();
                    onConfirm();
                    break;
                case SDL_SCANCODE_RETURN:
                case SDL_SCANCODE_KP_ENTER:
                    onConfirm();
                    break;
                case SDL_SCANCODE_P:
                    pausePressed_ = true;
                    break;
                default:
                    break;
            }
            break;

        case SDL_CONTROLLERDEVICEADDED:
            openController(e.cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            closeController(e.cdevice.which);
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            switch (e.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_A:
                case SDL_CONTROLLER_BUTTON_B:
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    onFlap();
                    onConfirm();
                    break;
                case SDL_CONTROLLER_BUTTON_START:
                    pausePressed_ = true;
                    onConfirm();
                    break;
                case SDL_CONTROLLER_BUTTON_BACK:
                    quit_ = true;
                    break;
                default:
                    break;
            }
            break;

        case SDL_CONTROLLERAXISMOTION:
            if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                onStickY(e.caxis.value);
            }
            break;

        default:
            break;
    }
}

}  // namespace flying
