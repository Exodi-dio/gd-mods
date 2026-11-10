#include "state.hpp"
#include <Geode/Geode.hpp>
#include <fstream>

using namespace geode::prelude;

static inline float relu(float x) { return x > 0.f ? x : 0.f; }
static inline float sigmoid(float x) { return 1.f / (1.f + std::exp(-x)); }

float infer(const ModelWeights& w, const Obs& obs) {
    auto x = obsToArray(obs);

    // layer 1
    std::array<float, HIDDEN> h1{};
    for (int i = 0; i < HIDDEN; i++) {
        float s = w.b1[i];
        for (int j = 0; j < OBS_DIM; j++) s += w.W1[i * OBS_DIM + j] * x[j];
        h1[i] = relu(s);
    }
    // layer 2
    std::array<float, HIDDEN> h2{};
    for (int i = 0; i < HIDDEN; i++) {
        float s = w.b2[i];
        for (int j = 0; j < HIDDEN; j++) s += w.W2[i * HIDDEN + j] * h1[j];
        h2[i] = relu(s);
    }
    // output
    float out = w.b3[0];
    for (int j = 0; j < HIDDEN; j++) out += w.W3[j] * h2[j];
    return sigmoid(out);
}

bool loadWeights(ModelWeights& w, const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.read(reinterpret_cast<char*>(&w), sizeof(w));
    return f.good() || f.eof();
}
