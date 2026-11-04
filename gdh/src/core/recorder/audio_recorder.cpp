#include "audio_recorder.hpp"
#include "../config.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace GDH;
using namespace geode::prelude;

void AudioRecorder::init() {
    auto system = FMODAudioEngine::sharedEngine()->m_system;
    system->getSoftwareFormat((int*)&m_sampleRate, nullptr, nullptr);

    FMOD_DSP_DESCRIPTION desc = {};
    std::snprintf(desc.name, sizeof(desc.name), "GDH DSP Recorder");
    desc.numinputbuffers = 1;
    desc.numoutputbuffers = 1;
    
    desc.read = [](FMOD_DSP_STATE* dsp, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels) -> FMOD_RESULT {
        auto recorder = &AudioRecorder::get();
        auto channels = *outchannels;
        
        std::memcpy(outbuffer, inbuffer, length * channels * sizeof(float));

        if (!recorder->is_recording) return FMOD_OK;

        {
            std::lock_guard<std::mutex> lock(recorder->m_lock);
            recorder->m_data.insert(recorder->m_data.end(), inbuffer, inbuffer + length * channels);
            recorder->m_current_audio_time += static_cast<double>(length) / recorder->m_sampleRate;
        }
        recorder->m_cv.notify_all();

        return FMOD_OK;
    };

    system->createDSP(&desc, &m_dsp);
    system->getMasterChannelGroup(&m_masterGroup);
}

void AudioRecorder::start() {
    if (is_recording) return;
    if (!m_dsp) init();

    m_masterGroup->addDSP(0, m_dsp);
    m_masterGroup->setPaused(true);
    
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_data.clear();
        m_current_audio_time = 0.0;
        m_target_audio_time = 0.0;
        is_recording = true;
    }
}

void AudioRecorder::stop() {
    if (!is_recording) return;

    m_masterGroup->removeDSP(m_dsp);
    
    {
        std::lock_guard<std::mutex> lock(m_lock);
        is_recording = false;
    }
    
    m_masterGroup->setPaused(false);

    #ifdef GEODE_IS_WINDOWS
    std::filesystem::path folder = Config::get().get<std::string>("recorder::path", (getFolderPath() / "Showcases").string());
    save_to_wav(folder / audio_name);
    #endif
}

std::vector<float> AudioRecorder::get_data() {
    std::lock_guard<std::mutex> lock(m_lock);
    auto data = m_data;
    m_data.clear();
    return data;
}

void AudioRecorder::wait_for_audio(double dt) {
    if (!is_recording) return;

    auto pl = PlayLayer::get();
    if (pl && (!pl->m_started || pl->m_isPaused)) return;

    std::unique_lock<std::mutex> lock(m_lock);
    m_target_audio_time += dt;

    m_masterGroup->setPaused(false);

    bool reached_target = m_cv.wait_for(lock, std::chrono::milliseconds(100), [this] {
        return m_current_audio_time >= m_target_audio_time;
    });

    m_masterGroup->setPaused(true);

    if (!reached_target) {
        geode::log::warn("timeout waiting for audio! current: {}; target: {}", m_current_audio_time, m_target_audio_time);
    }
}

void AudioRecorder::save_to_wav(const std::filesystem::path& filename) {
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_data.empty()) return;

    int sampleRate = 0, channels = 0;
    FMODAudioEngine::sharedEngine()->m_system->getSoftwareFormat(&sampleRate, nullptr, &channels);
    if (sampleRate == 0 || channels == 0) return;

    std::filesystem::create_directories(filename.parent_path());

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) return;

    struct WavHeader {
        char chunkId[4] = {'R', 'I', 'F', 'F'};
        uint32_t chunkSize;
        char format[4] = {'W', 'A', 'V', 'E'};
        char subchunk1Id[4] = {'f', 'm', 't', ' '};
        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 3;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample = 32;
        char subchunk2Id[4] = {'d', 'a', 't', 'a'};
        uint32_t subchunk2Size;
    };

    WavHeader header;
    header.numChannels = static_cast<uint16_t>(channels);
    header.sampleRate = static_cast<uint32_t>(sampleRate);
    header.byteRate = header.sampleRate * header.numChannels * sizeof(float);
    header.blockAlign = header.numChannels * sizeof(float);
    header.subchunk2Size = static_cast<uint32_t>(m_data.size() * sizeof(float));
    header.chunkSize = 36 + header.subchunk2Size;

    outFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
    outFile.write(reinterpret_cast<const char*>(m_data.data()), m_data.size() * sizeof(float));
}