#pragma once

#include "render_settings.hpp"

#include <Geode/Geode.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace ffmpeg::events {

class Recorder {
public:
    Recorder() = default;
    ~Recorder() = default;

    bool isValid() const { return false; }

    geode::Result<> init(RenderSettings const& settings) {
        return geode::Err("Rendering is not available in this build.");
    }

    void stop() {}

    geode::Result<> writeFrame(const std::vector<uint8_t>& frameData) {
        return geode::Err("Rendering is not available in this build.");
    }

    static std::vector<std::string> getAvailableCodecs() {
        return {};
    }
};

class AudioMixer {
public:
    AudioMixer() = delete;

    static geode::Result<> mixVideoAudio(
        std::filesystem::path const& videoFile,
        std::filesystem::path const& audioFile,
        std::filesystem::path const& outputMp4File
    ) {
        return geode::Err("Rendering is not available in this build.");
    }

    static geode::Result<> mixVideoRaw(
        std::filesystem::path const& videoFile,
        const std::vector<float>& raw,
        std::filesystem::path const& outputMp4File
    ) {
        return geode::Err("Rendering is not available in this build.");
    }
};

}
