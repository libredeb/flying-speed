#pragma once

#include <SDL.h>
#include <array>

namespace flying {

// Keyboard plus SDL2 gamepads, using assets/gamecontrollerdb.txt mappings.
class Input {
public:
    static constexpr int kMaxPads = 4;

    bool init();
    void shutdown();

    void beginFrame();
    void handleEvent(const SDL_Event& e);

    bool flapPressed() const { return flapPressed_; }
    bool confirmPressed() const { return confirmPressed_; }
    bool pausePressed() const { return pausePressed_; }
    bool quitRequested() const { return quit_; }
    int padCount() const;

private:
    void openController(int joystickIndex);
    void closeController(SDL_JoystickID instanceId);
    void onFlap();
    void onConfirm();
    void onStickY(Sint16 value);

    bool flapPressed_ = false;
    bool confirmPressed_ = false;
    bool pausePressed_ = false;
    bool quit_ = false;
    bool stickFlapHeld_ = false;

    std::array<SDL_GameController*, kMaxPads> pads_{};
};

}  // namespace flying
