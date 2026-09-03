#include "Parallax.hpp"

#include "Assets.hpp"
#include "Config.hpp"

namespace flying {

void Parallax::init(const Assets& assets) {
    layers_[0] = {assets.clouds, 0.f, kCloudSpeed, 0, 0};
    layers_[1] = {assets.farBuildings, 0.f, kFarBuildingSpeed, 0, 0};
    layers_[2] = {assets.nearBuildings, 0.f, kNearBuildingSpeed, 0, 0};
    layers_[3] = {assets.trees, 0.f, kTreeSpeed, 0, 0};

    for (auto& layer : layers_) {
        if (layer.tex) {
            SDL_QueryTexture(layer.tex, nullptr, nullptr, &layer.width, &layer.height);
        }
    }
}

void Parallax::reset() {
    for (auto& layer : layers_) {
        layer.offset = 0;
    }
}

void Parallax::update(float dt, bool moving, float speedScale) {
    if (!moving) {
        return;
    }
    for (auto& layer : layers_) {
        if (layer.width <= 0) {
            continue;
        }
        layer.offset += layer.speed * speedScale * dt;
        const float w = static_cast<float>(layer.width);
        while (layer.offset >= w) {
            layer.offset -= w;
        }
    }
}

void Parallax::drawBack(SDL_Renderer* renderer) const {
    drawRange(renderer, 0, 3);
}

void Parallax::drawFront(SDL_Renderer* renderer) const {
    drawRange(renderer, 3, 4);
}

void Parallax::drawRange(SDL_Renderer* renderer, std::size_t from, std::size_t to) const {
    for (std::size_t i = from; i < to && i < layers_.size(); ++i) {
        const auto& layer = layers_[i];
        if (!layer.tex || layer.width <= 0) {
            continue;
        }
        const float w = static_cast<float>(layer.width);
        const float h = static_cast<float>(kLogicalH);
        float x = -layer.offset;
        while (x < static_cast<float>(kLogicalW)) {
            SDL_FRect dst{x, 0.f, w, h};
            SDL_RenderCopyF(renderer, layer.tex, nullptr, &dst);
            x += w;
        }
    }
}

}  // namespace flying
