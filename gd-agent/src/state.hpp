#pragma once

#include <Geode/Geode.hpp>
#include <vector>
#include <array>
#include <cmath>

using namespace geode::prelude;

// Observation space for cube mode.
// Features: player y, player vy, on_ground, distance to next obstacle (x),
// obstacle height, obstacle type (one-hot-ish scalar). Keep small for mobile.
constexpr int OBS_DIM = 8;
constexpr int HIDDEN = 32;
constexpr int ACTION_DIM = 1; // 0 = release, 1 = tap/hold

struct Obs {
    float player_y = 0.f;
    float player_vy = 0.f;
    float on_ground = 0.f;
    float next_obs_x = 1.f;   // normalized distance
    float next_obs_y = 0.f;
    float next_obs_h = 0.f;
    float next_obs_type = 0.f;
    float speed = 1.f;
};

inline std::array<float, OBS_DIM> obsToArray(const Obs& o) {
    return {o.player_y, o.player_vy, o.on_ground, o.next_obs_x,
            o.next_obs_y, o.next_obs_h, o.next_obs_type, o.speed};
}

// Tiny MLP: OBS_DIM -> HIDDEN -> HIDDEN -> ACTION_DIM (sigmoid)
// Weights stored as flat arrays; filled by trained model (see trainer/export.py).
struct ModelWeights {
    // W1: HIDDEN x OBS_DIM, b1: HIDDEN
    std::array<float, HIDDEN * OBS_DIM> W1{};
    std::array<float, HIDDEN> b1{};
    // W2: HIDDEN x HIDDEN, b2: HIDDEN
    std::array<float, HIDDEN * HIDDEN> W2{};
    std::array<float, HIDDEN> b2{};
    // W3: ACTION_DIM x HIDDEN, b3: ACTION_DIM
    std::array<float, ACTION_DIM * HIDDEN> W3{};
    std::array<float, ACTION_DIM> b3{};
};

// Forward pass. Returns action probability (0..1). >0.5 => tap.
float infer(const ModelWeights& w, const Obs& obs);

// Load weights from a file (optional). Returns false if missing/invalid.
bool loadWeights(ModelWeights& w, const std::filesystem::path& path);
