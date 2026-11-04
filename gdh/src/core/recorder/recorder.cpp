#include "recorder.hpp"
#include "audio_recorder.hpp"
#include "../config.hpp"
#include "../gui.hpp"

#ifdef GEODE_IS_WINDOWS
#include "subprocess.hpp"
#endif

#if defined(GEODE_IS_ANDROID) || defined(GEODE_IS_MACOS)
#include "h264_encoder.hpp"
#include "log_overlay.hpp"
#endif

using namespace GDH;

void Recorder::start() {
    if (is_recording) return;
    
    is_recording = true;

    #ifdef GEODE_IS_WINDOWS
    compile_command();
    #endif
    
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Achievements");
    if (!hack.getEnabled()) hack.enable();

    if (auto pl = PlayLayer::get()) {
        if (pl->m_percentageLabel) pl->m_percentageLabel->setVisible(false);
        if (pl->m_progressBar) pl->m_progressBar->setVisible(false);
    }

    last_frame_time = 0;
    extra_time = 0;
    after_end_extra_time = 0;
    frame_has_data = false;

    current_frame.resize(width * height * 4, 0);
    texture.init(width, height);

    setupResolution();

    #ifdef GEODE_IS_WINDOWS
    startWindowsEncoder();
    #elif defined(GEODE_IS_ANDROID) || defined(GEODE_IS_MACOS)
    LogOverlay::get().create();
    startAndroidEncoder();
    #endif
}

void Recorder::setupResolution() {
    auto view = cocos2d::CCEGLView::get();
    if (!view) return;

    oldDesignResolution = view->getDesignResolutionSize();
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    newDesignResolution = cocos2d::CCSize(std::round(320.f * aspectRatio), 320.f);

    originalScreenScale = cocos2d::CCSize(view->m_fScaleX, view->m_fScaleY);
    newScreenScale = cocos2d::CCSize(
        static_cast<float>(width) / newDesignResolution.width, 
        static_cast<float>(height) / newDesignResolution.height
    );
}

#ifdef GEODE_IS_WINDOWS
void Recorder::startWindowsEncoder() {
    std::thread([this]() {
        auto process = subprocess::Popen(command, !console);
        std::vector<uint8_t> local_frame(width * height * 4, 0);

        while (is_recording || frame_has_data) {
            std::unique_lock<std::mutex> u_lock(lock);
            cv.wait(u_lock, [this] { return frame_has_data || !is_recording; });

            if (frame_has_data) {
                std::swap(local_frame, current_frame);
                frame_has_data = false;
                u_lock.unlock();
                cv.notify_one();

                if (!process.m_stdin.write(local_frame.data(), local_frame.size())) {
                    geode::queueInMainThread([] { Recorder::get().force_stop = true; });
                    geode::log::error("failed to write frame to ffmpeg pipe!");
                }

            } else {
                u_lock.unlock();
            }
        }
        
        int render_code = process.close();

        if (render_code != 0) {
            geode::log::error("render process failed ({})", render_code);
            return;
        }

        if (record_audio) {
            std::filesystem::path folder = Config::get().get<std::string>("recorder::path", (getFolderPath() / "Showcases").string());
            std::filesystem::path video_path = folder / videoName;
            std::filesystem::path audio_path = folder / AudioRecorder::get().audio_name;
            std::filesystem::path temp_video_path = folder / ("temp_" + videoName);

            std::error_code ec;
            if (!std::filesystem::exists(video_path, ec) || !std::filesystem::exists(audio_path, ec)) {
                geode::log::error("video or audio file missing before merge");
                return;
            }

            std::filesystem::rename(video_path, temp_video_path, ec);
            if (ec) {
                geode::log::error("failed to rename video for temp processing");
                return;
            }

            std::string merge_cmd = fmt::format(
                "ffmpeg.exe -y -i \"{}\" -i \"{}\" -c:v copy {} -shortest \"{}\"",
                temp_video_path.string(),
                audio_path.string(),
                audio_args,
                video_path.string()
            );

            auto merge_process = subprocess::Popen(merge_cmd, !console);
            int merge_code = merge_process.close();

            if (merge_code == 0 && std::filesystem::exists(video_path, ec)) {
                std::filesystem::remove(temp_video_path, ec);
                std::filesystem::remove(audio_path, ec);
            } else {
                geode::log::error("merge process failed ({}), restoring original video", merge_code);
                std::filesystem::rename(temp_video_path, video_path, ec);
            }
        }
    }).detach();
}
#endif

#if defined(GEODE_IS_ANDROID) || defined(GEODE_IS_MACOS)
void Recorder::startAndroidEncoder() {
    std::thread([this]() {
        std::filesystem::path folder = Config::get().get<std::string>("recorder::path", (getFolderPath() / "Showcases").string());
        auto encoder = new H264Encoder(width, height, fps, bitrate, ultrafast ? "ultrafast" : "medium");

        if (!encoder->is_valid() || !encoder->start((folder / videoName).string())) {
            delete encoder;
            geode::queueInMainThread([] {
                Recorder::get().force_stop = true;
                GDH::MaterialLayer(FLAlertLayer::create("Encoder Error", "Recorder failure during startup, check the logs", "OK")
                )->show();
            });
            return;
        }

        geode::queueInMainThread([folder = std::move(folder)] {
            auto& recorder = Recorder::get();
            GDH::MaterialLayer(FLAlertLayer::create("Recorder", 
                fmt::format("Recording started: {}\n\nRestart level to start recording", folder / recorder.videoName), "OK")
            )->show();
        });

        std::vector<uint8_t> local_frame(width * height * 4, 0);

        while (is_recording || frame_has_data) {
            std::unique_lock<std::mutex> u_lock(lock);
            cv.wait(u_lock, [this] { return frame_has_data || !is_recording; });

            if (frame_has_data) {
                std::swap(local_frame, current_frame);
                frame_has_data = false;
                u_lock.unlock();
                cv.notify_one();

                encoder->encode_frame(local_frame);
            } else {
                u_lock.unlock();
            }
        }
        
        if (record_audio) {
            auto audio_samples = AudioRecorder::get().get_data();
            encoder->finalize_with_audio(audio_samples);
        }

        encoder->stop();

        geode::queueInMainThread([] {
            LogOverlay::get().destroy();
            GDH::MaterialLayer(FLAlertLayer::create("Encoder", "Recorder has finished working", "OK")
            )->show();
        });
        delete encoder;
    }).detach();
}
#endif

void Recorder::stop() {
    if (!is_recording) return;

    texture.cleanup();

    needRevertOld = true;
    force_stop = false;

    if (record_audio) {
        AudioRecorder::get().stop();
    }

    {
        std::lock_guard<std::mutex> u_lock(lock);
        is_recording = false;
        enabled = false;
    }
    cv.notify_all();
}

void Recorder::capture_frame() {
    std::unique_lock<std::mutex> u_lock(lock);
    cv.wait(u_lock, [this] { return !frame_has_data; });
    u_lock.unlock();

    texture.capture(PlayLayer::get(), lock, cv, current_frame, frame_has_data);
}

void Recorder::handle_recording(float dt) {
    auto playLayer = PlayLayer::get();
    
    if (!force_stop && (!playLayer->m_hasCompletedLevel || after_end_extra_time < after_end_duration)) {
        if (playLayer->m_hasCompletedLevel) {
            after_end_extra_time += dt;
        }

        if (record_audio && !AudioRecorder::get().is_recording && playLayer->m_resumeTimer < 1) {
            AudioRecorder::get().start();
        }

        double frame_dt = 1.0 / static_cast<double>(fps);
        double time = (playLayer->m_gameState.m_levelTime - delay) + extra_time - last_frame_time;
        
        if (time >= frame_dt) {
            extra_time = time - frame_dt;
            last_frame_time = playLayer->m_gameState.m_levelTime - delay;
            capture_frame();
        }
    } else {        
        stop();
    } 
}

void Recorder::compile_command() {
    command = fmt::format("ffmpeg.exe -y -f rawvideo -pix_fmt rgba -s {}x{} -r {} -i -", width, height, fps);

    if (!codec.empty()) command += fmt::format(" -c:v {}", codec);
    if (!bitrate.empty()) command += fmt::format(" -b:v {}", bitrate);
    
    if (!extra_args.empty()) {
        command += fmt::format(" {}", extra_args);
    } else {
        command += " -pix_fmt yuv420p";
    }
    
    if (!vf_args.empty()) command += fmt::format(" -vf {}", vf_args);

    std::filesystem::path folder = Config::get().get<std::string>("recorder::path", (getFolderPath() / "Showcases").string());
    command += fmt::format(" -an \"{}\\{}\"", folder.string(), videoName);
}

void Recorder::resize_resolution(geode::Function<void()> action) {
    auto view = cocos2d::CCEGLView::get();

    if (needRevertOld) {
        if (oldDesignResolution.width != 0 && oldDesignResolution.height != 0 && view) {
            cocos2d::CCDirector::get()->m_obWinSizeInPoints = oldDesignResolution;
            view->setDesignResolutionSize(oldDesignResolution.width, oldDesignResolution.height, ResolutionPolicy::kResolutionExactFit);
            view->m_fScaleX = originalScreenScale.width;
            view->m_fScaleY = originalScreenScale.height;
        }
        needRevertOld = false;
    }

    if (!is_recording || (PlayLayer::get() && PlayLayer::get()->m_isPaused)) {
        action();
        return;
    }
    
    if (newDesignResolution.width != 0 && newDesignResolution.height != 0) {    
        cocos2d::CCDirector::get()->m_obWinSizeInPoints = newDesignResolution;        
        view->setDesignResolutionSize(newDesignResolution.width, newDesignResolution.height, ResolutionPolicy::kResolutionExactFit);
        view->m_fScaleX = newScreenScale.width;
        view->m_fScaleY = newScreenScale.height;
    }

    action();

    if (oldDesignResolution.width != 0 && oldDesignResolution.height != 0) {
        cocos2d::CCDirector::get()->m_obWinSizeInPoints = oldDesignResolution;
        view->setDesignResolutionSize(oldDesignResolution.width, oldDesignResolution.height, ResolutionPolicy::kResolutionExactFit);
        view->m_fScaleX = originalScreenScale.width;
        view->m_fScaleY = originalScreenScale.height;
    }
}