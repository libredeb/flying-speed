#include "Util.hpp"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <vector>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#endif

#include <SDL.h>
#include <cstdio>

namespace flying {
namespace {

bool isFile(const std::string& path) {
    struct stat st {};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

std::string parentDir(std::string path) {
    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }
    const auto slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return ".";
    }
    if (slash == 0) {
        return "/";
    }
    return path.substr(0, slash);
}

}  // namespace

float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

int randInt(int lo, int hi) {
    if (hi <= lo) {
        return lo;
    }
    return lo + std::rand() % (hi - lo + 1);
}

float randFloat(float lo, float hi) {
    const float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return lerpf(lo, hi, t);
}

void seedRng() {
    std::srand(static_cast<unsigned>(std::time(nullptr)) ^ static_cast<unsigned>(SDL_GetTicks()));
}

std::string executableDir() {
    std::string path;

#ifdef __APPLE__
    uint32_t size = 1024;
    std::string buf(size, '\0');
    if (_NSGetExecutablePath(buf.data(), &size) != 0) {
        buf.assign(size, '\0');
        _NSGetExecutablePath(buf.data(), &size);
    }
    path = buf.c_str();
#elif defined(__linux__)
    char buf[PATH_MAX];
    const ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        path = buf;
    }
#endif

    if (path.empty()) {
        return ".";
    }

    char* resolved = realpath(path.c_str(), nullptr);
    if (resolved) {
        path = resolved;
        std::free(resolved);
    }
    return parentDir(path);
}

std::string toUpperAscii(std::string text) {
    for (char& c : text) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 32);
        }
    }
    return text;
}

std::string findAssetsRoot() {
    const char* env = std::getenv("FLYING_SPEED_ASSETS");
    std::vector<std::string> candidates;
    if (env && env[0] != '\0') {
        candidates.emplace_back(env);
    }

    const std::string exe = executableDir();
    candidates.push_back(join(exe, "assets"));
    candidates.push_back(join(parentDir(exe), "assets"));
    candidates.emplace_back("assets");
    candidates.emplace_back("/usr/local/share/flying-speed/assets");
    candidates.emplace_back("/usr/share/flying-speed/assets");

    for (const auto& dir : candidates) {
        if (isFile(join(dir, "bird.png"))) {
            return dir;
        }
    }
    return join(exe, "assets");
}

bool isDir(const std::string& path) {
    struct stat st {};
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool ensureDir(const std::string& path) {
    if (path.empty() || path == "/") {
        return true;
    }
    if (isDir(path)) {
        return true;
    }
    if (!ensureDir(parentDir(path))) {
        return false;
    }
    if (mkdir(path.c_str(), 0755) == 0 || errno == EEXIST) {
        return true;
    }
    return isDir(path);
}

std::string userDataDir() {
    const char* home = std::getenv("HOME");
    const std::string homeDir = (home && home[0] != '\0') ? home : ".";
#ifdef __APPLE__
    return join(join(homeDir, "Library/Application Support"), "flying-speed");
#else
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (xdg && xdg[0] != '\0') {
        return join(xdg, "flying-speed");
    }
    return join(join(homeDir, ".local/share"), "flying-speed");
#endif
}

std::string bestScorePath() {
    return join(userDataDir(), "best.txt");
}

int loadBestScore() {
    std::ifstream in(bestScorePath());
    if (!in) {
        return 0;
    }
    int value = 0;
    in >> value;
    if (!in || value < 0) {
        return 0;
    }
    return value;
}

void saveBestScore(int best) {
    const std::string dir = userDataDir();
    if (!ensureDir(dir)) {
        std::fprintf(stderr, "No se pudo crear %s\n", dir.c_str());
        return;
    }
    std::ofstream out(bestScorePath(), std::ios::trunc);
    if (!out) {
        std::fprintf(stderr, "No se pudo guardar el score en %s\n", bestScorePath().c_str());
        return;
    }
    out << best << '\n';
}

}  // namespace flying
