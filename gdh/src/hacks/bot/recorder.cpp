#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include "../../core/recorder/recorder.hpp"

class $modify(PlayLayer) {
    struct Fields {
        ~Fields() {
            GDH::Recorder::get().stop();
        }
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        m_fields.self();
        
        auto pl = PlayLayer::get();
        auto& recorder = GDH::Recorder::get();

        if (!recorder.isRecording()) {
            auto time = geode::localtime(std::time(nullptr));
            recorder.videoName = fmt::format("{} {:%Y-%m-%d_%H-%M-%S}.mp4", m_level->m_levelName, time);
        }

        return true;
    }

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        auto& recorder = GDH::Recorder::get();
        if (recorder.isRecording()) {
            if (m_percentageLabel) m_percentageLabel->setVisible(false);
            if (m_progressBar) m_progressBar->setVisible(false);
        }
    }
};

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto& recorder = GDH::Recorder::get();
        
        if (!recorder.isRecording()) {
            auto time = geode::localtime(std::time(nullptr));
            recorder.videoName = fmt::format("{} {:%Y-%m-%d_%H-%M-%S}.mp4", m_level->m_levelName, time);
        }

        return true;
    }
};

class $modify(EditLevelLayer) {
    bool init(GJGameLevel *level) {
        if (!EditLevelLayer::init(level)) return false;

        auto& recorder = GDH::Recorder::get();
        
        if (!recorder.isRecording()) {
            auto time = geode::localtime(std::time(nullptr));
            recorder.videoName = fmt::format("{} {:%Y-%m-%d_%H-%M-%S}.mp4", m_level->m_levelName, time);
        }

        return true;
    }
};

class $modify(GJBaseGameLayer) {
    static void onModify(auto& self) {
        (void) self.setHookPriority("GJBaseGameLayer::update", geode::Priority::Last); 
    }

    void update(float dt) {       
        GJBaseGameLayer::update(dt);
        
        auto& recorder = GDH::Recorder::get();
        if (!recorder.isRecording()) return;

        if (auto pl = PlayLayer::get()) {
            recorder.handle_recording(dt);
        }
    }
};

class $modify(FMODAudioEngine) {
    int playEffect(gd::string path, float speed, float unknown, float volume) {
        if (GDH::Recorder::get().isRecording() && path == "playSound_01.ogg")
            path = "";

        return FMODAudioEngine::playEffect(path, speed, unknown, volume);
    }
};

class $modify(UILayer) {
    void toggleMenuVisibility(bool visible) {        
        UILayer::toggleMenuVisibility(visible);

        if (GDH::Recorder::get().isRecording()) {
            this->m_pauseBtn->getNormalImage()->setVisible(false);

            for (auto child : this->getChildrenExt()) {                    
                if (geode::cast::typeinfo_cast<GJUINode*>(child)) {
                    static_cast<cocos2d::CCNode*>(child)->setVisible(false);
                }
            }
        }
    }
};