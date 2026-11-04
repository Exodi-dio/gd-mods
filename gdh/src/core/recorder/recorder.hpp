#pragma once
#include <Geode/Geode.hpp>
#include "render_textute.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

namespace GDH {
    class Recorder {
    public:
        static Recorder& get() {
            static Recorder instance;
            return instance;
        }
    
        Recorder(const Recorder&) = delete;
        Recorder& operator=(const Recorder&) = delete;
    
        std::string videoName = "video.mp4";
        
        bool enabled = false;
        bool lock_aspect_ratio = true;
        bool record_audio = true;
        bool console = false;
        bool force_stop = false;
    
        int width = 1920;
        int height = 1080;
        int fps = 60;
        
        std::string bitrate = "50M";
        std::string codec = "libx264";
        std::string extra_args = "-pix_fmt yuv420p -preset ultrafast";
        std::string vf_args = "vflip,colorspace=all=bt709:iall=bt470bg:fast=1";
        std::string audio_args = "-c:a aac -b:a 320k -ar 48000";
        
        float after_end_duration = 3.f;
        double delay = 0.0;

        bool ultrafast = true;

        void start();
        void stop();
        void handle_recording(float dt);
        void compile_command();
        void resize_resolution(geode::Function<void()> action);
        
        bool isRecording() const { return is_recording; }

    private:
        Recorder() = default;

        bool is_recording = false;
        bool needRevertOld = false;
        float after_end_extra_time = 0.f;
        double last_frame_time = 0.0;
        double extra_time = 0.0;
        
        std::string command;
        RenderTexture texture;
        std::vector<uint8_t> current_frame;
        bool frame_has_data = false;

        std::mutex lock;
        std::condition_variable cv;

        cocos2d::CCSize oldDesignResolution;
        cocos2d::CCSize newDesignResolution;
        cocos2d::CCSize originalScreenScale;
        cocos2d::CCSize newScreenScale;

        void capture_frame();
        void setupResolution();
        
        #ifdef GEODE_IS_WINDOWS
        void startWindowsEncoder();
        #endif

        #if defined(GEODE_IS_ANDROID) || defined(GEODE_IS_MACOS)
        void startAndroidEncoder();
        #endif
    };
}