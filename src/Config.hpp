#pragma once

namespace flying {

constexpr int kLogicalW = 720;
constexpr int kLogicalH = 720;

constexpr float kGravity = 1280.f;
constexpr float kFlapVelocity = -400.f;
constexpr float kMaxFallSpeed = 720.f;
constexpr float kBirdX = 176.f;
constexpr float kBirdDrawSize = 120.f;
constexpr float kBirdHitRadius = 30.f;

constexpr float kScrollSpeed = 195.f;
constexpr float kPipeScale = 0.42f;
constexpr float kPipeMinSpacing = 380.f;
constexpr float kPipeMaxSpacing = 450.f;
constexpr float kGapEasyMin = 280.f;
constexpr float kGapEasyMax = 350.f;
constexpr float kGapHardMin = 188.f;
constexpr float kGapHardMax = 255.f;
constexpr int kGapDifficultyScore = 18;
constexpr float kGapMargin = 84.f;

constexpr float kCloudSpeed = 16.f;
constexpr float kFarBuildingSpeed = 38.f;
constexpr float kNearBuildingSpeed = 82.f;
constexpr float kTreeSpeed = 150.f;

constexpr float kDeathRestartDelay = 0.55f;

// Blue head art is 2px to the right of its body at native 326px.
constexpr float kBlueHeadAlignPx = -2.f;

constexpr int kOpeningGreenPipes = 3;
constexpr int kMinGreensBetweenSpecials = 3;

constexpr int kMovingPipeInterval = 50;
constexpr int kMaxMovingPipes = 6;
constexpr float kMovingPipeSpeedMin = 1.2f;
constexpr float kMovingPipeSpeedMax = 2.4f;
constexpr float kMovingPipeAmpMin = 40.f;
constexpr float kMovingPipeAmpMax = 80.f;

// Head / body content insets measured from the 326px pipe textures.
constexpr float kHeadPadL = 10.f / 326.f;
constexpr float kHeadPadR = 10.f / 326.f;
constexpr float kBodyPadL = 30.f / 326.f;
constexpr float kBodyPadR = 30.f / 326.f;

}  // namespace flying
