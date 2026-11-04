#pragma once
#include <Geode/Geode.hpp>
#include <fmod.hpp>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace GDH {
    class AudioRecorder {
    public:
        static AudioRecorder& get() {
            static AudioRecorder instance;
            return instance;
        }

        void init();
        void start();
        void stop();
        std::vector<float> get_data();
        
        void wait_for_audio(double dt);

        void save_to_wav(const std::filesystem::path& filename);

        bool is_recording = false;
        std::string audio_name = "audio.wav";

        std::vector<float> m_data;
        std::mutex m_lock;
        std::condition_variable m_cv;
        
        FMOD::DSP* m_dsp = nullptr;
        FMOD::ChannelGroup* m_masterGroup = nullptr;
        
        unsigned int m_sampleRate = 44100;
        double m_current_audio_time = 0.0;
        double m_target_audio_time = 0.0;

    private:
        AudioRecorder() = default;
    };
}