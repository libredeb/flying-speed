#include "Game.hpp"

#include "Config.hpp"
#include "Util.hpp"

#include <SDL_ttf.h>

#include <algorithm>
#include <cmath>

namespace flying {

bool Game::init(SDL_Renderer* renderer, ProgressFn progress) {
    renderer_ = renderer;

    constexpr int kAssetSteps = Assets::kLoadSteps;
    constexpr int kTotalSteps = kAssetSteps + 2;

    if (progress) {
        progress(0, kTotalSteps);
    }

    auto assetProgress = progress
        ? ProgressFn([&](int step, int /*assetTotal*/) {
              progress(step, kTotalSteps);
          })
        : nullptr;

    if (!assets_.load(renderer_, assetProgress)) {
        return false;
    }

    audio_.init(assets_.root);
    if (progress) {
        progress(kAssetSteps + 1, kTotalSteps);
    }

    parallax_.init(assets_);
    pipes_.init(assets_);
    best_ = loadBestScore();
    resetToTitle();
    if (progress) {
        progress(kTotalSteps, kTotalSteps);
    }
    return true;
}

void Game::shutdown() {
    audio_.shutdown();
    assets_.unload();
}

void Game::resetToTitle() {
    state_ = State::Title;
    score_ = 0;
    deathTimer_ = 0;
    flash_ = 0;
    paused_ = false;
    bird_.reset();
    trail_.reset();
    pipes_.reset();
    parallax_.reset();
}

void Game::startRun() {
    state_ = State::Playing;
    score_ = 0;
    deathTimer_ = 0;
    paused_ = false;
    bird_.reset();
    bird_.flap();
    trail_.reset();
    trail_.emitFlap(bird_.tailX(), bird_.tailY());
    pipes_.reset();
    audio_.play(SoundId::Flap);
}

void Game::die() {
    if (state_ != State::Playing) {
        return;
    }
    state_ = State::Dead;
    deathTimer_ = 0;
    flash_ = 1.f;
    if (score_ > best_) {
        best_ = score_;
    }
    saveBestScore(best_);
    audio_.play(SoundId::Hit);
}

void Game::handleInput(const Input& input) {
    if (input.pausePressed() && state_ == State::Playing) {
        paused_ = !paused_;
        audio_.setPaused(paused_);
    }

    if (paused_) {
        return;
    }

    if (state_ == State::Title && (input.flapPressed() || input.confirmPressed())) {
        startRun();
        return;
    }

    if (state_ == State::Playing && input.flapPressed()) {
        bird_.flap();
        trail_.emitFlap(bird_.tailX(), bird_.tailY());
        audio_.play(SoundId::Flap);
        return;
    }

    if (state_ == State::Dead && deathTimer_ >= kDeathRestartDelay &&
        (input.flapPressed() || input.confirmPressed())) {
        startRun();
    }
}

void Game::update(float dt) {
    if (paused_) {
        return;
    }

    flash_ = std::max(0.f, flash_ - dt * 3.6f);

    const bool worldMoving = (state_ == State::Title) || (state_ == State::Playing);
    const float parallaxScale = (state_ == State::Title) ? 0.42f : 1.f;
    parallax_.update(dt, worldMoving, parallaxScale);

    if (state_ == State::Title) {
        bird_.update(dt, false);
        trail_.emitContinuous(bird_.tailX(), bird_.tailY(), 0.f);
        trail_.update(dt);
        return;
    }

    if (state_ == State::Playing) {
        bird_.update(dt, true);
        pipes_.update(dt, true, score_);
        trail_.emitContinuous(bird_.tailX(), bird_.tailY(), bird_.vy());
        trail_.update(dt);

        const int gained = pipes_.collectScores(bird_.x());
        if (gained > 0) {
            score_ += gained;
            audio_.play(SoundId::Score);
        }

        const bool hitPipe = pipes_.hits(bird_.x(), bird_.y(), kBirdHitRadius);
        const bool outOfBounds =
            bird_.y() - kBirdHitRadius < 0.f ||
            bird_.y() + kBirdHitRadius > static_cast<float>(kLogicalH);
        if (hitPipe || outOfBounds) {
            die();
        }
        return;
    }

    bird_.update(dt, true);
    trail_.update(dt);
    deathTimer_ += dt;
}

void Game::drawSky() const {
    if (!assets_.sky) {
        SDL_SetRenderDrawColor(renderer_, 18, 28, 58, 255);
        SDL_RenderClear(renderer_);
        return;
    }
    SDL_FRect dst{0.f, 0.f, static_cast<float>(kLogicalW), static_cast<float>(kLogicalH)};
    SDL_RenderCopyF(renderer_, assets_.sky, nullptr, &dst);
}

void Game::drawText(TTF_Font* font, const std::string& text, int x, int y, SDL_Color color,
                    bool center) const {
    if (!font || text.empty()) {
        return;
    }
    const std::string upper = toUpperAscii(text);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, upper.c_str(), color);
    if (!surface) {
        return;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_Rect dst{x, y, surface->w, surface->h};
    if (center) {
        dst.x = x - surface->w / 2;
    }
    SDL_FreeSurface(surface);
    if (!tex) {
        return;
    }

    SDL_SetTextureColorMod(tex, 0, 0, 0);
    SDL_SetTextureAlphaMod(tex, 255);
    constexpr int kOutline = 4;
    for (int oy = -kOutline; oy <= kOutline; ++oy) {
        for (int ox = -kOutline; ox <= kOutline; ++ox) {
            if (ox == 0 && oy == 0) {
                continue;
            }
            SDL_Rect border = dst;
            border.x += ox;
            border.y += oy;
            SDL_RenderCopy(renderer_, tex, nullptr, &border);
        }
    }

    SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(tex, 255);
    SDL_RenderCopy(renderer_, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void Game::drawUi() const {
    const SDL_Color white{255, 255, 255, 255};
    const SDL_Color gold{255, 214, 90, 255};

    if (state_ == State::Title) {
        drawText(assets_.fontTitle, "Flying Speed", kLogicalW / 2, 40, gold, true);
        drawText(assets_.fontUi, "Pulsa ESPACIO para volar", kLogicalW / 2, 520, white, true);
        drawText(assets_.fontUi, "ESC para salir", kLogicalW / 2, 590, white, true);
        if (best_ > 0) {
            drawText(assets_.fontUi, "Mejor " + std::to_string(best_), kLogicalW / 2, 170, white, true);
        }
        return;
    }

    drawText(assets_.fontScore, std::to_string(score_), kLogicalW / 2, 8, white, true);

    if (paused_ && state_ == State::Playing) {
        drawText(assets_.fontTitle, "PAUSA", kLogicalW / 2, 250, gold, true);
        drawText(assets_.fontUi, "P para continuar", kLogicalW / 2, 390, white, true);
    }

    if (state_ == State::Dead) {
        drawText(assets_.fontTitle, "GAME OVER", kLogicalW / 2, 200, gold, true);
        drawText(assets_.fontUi, "Mejor " + std::to_string(best_), kLogicalW / 2, 340, white, true);
        if (deathTimer_ >= kDeathRestartDelay) {
            drawText(assets_.fontUi, "Pulsa ESPACIO para reiniciar", kLogicalW / 2, 500, white, true);
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    drawSky();
    parallax_.drawBack(renderer_);
    parallax_.drawFront(renderer_);
    trail_.draw(renderer_, assets_.trailDot);
    bird_.draw(renderer_, assets_);
    pipes_.draw(renderer_, assets_);
    drawUi();

    if (flash_ > 0.f) {
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, static_cast<Uint8>(flash_ * 180.f));
        SDL_Rect full{0, 0, kLogicalW, kLogicalH};
        SDL_RenderFillRect(renderer_, &full);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    }
}

}  // namespace flying
