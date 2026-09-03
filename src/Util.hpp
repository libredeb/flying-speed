#pragma once

#include <string>

namespace flying {

float clampf(float v, float lo, float hi);
float lerpf(float a, float b, float t);
int randInt(int lo, int hi);
float randFloat(float lo, float hi);
void seedRng();

std::string executableDir();
std::string findAssetsRoot();
std::string toUpperAscii(std::string text);
int loadBestScore();
void saveBestScore(int best);

}  // namespace flying
