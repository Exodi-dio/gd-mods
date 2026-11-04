#pragma once
#include <Geode/Geode.hpp>
#include "Geode/utils/general.hpp"
#include "utils.hpp"

namespace GDH { namespace Shortcuts {
    inline void openOptions() {
        auto options_layer = OptionsLayer::create();
        auto scene = cocos2d::CCScene::get();
        if (options_layer && scene) {
            auto zOrder = scene->getHighestChildZ();
            scene->addChild(options_layer, zOrder + 1);
            options_layer->showLayer(false);
        }
    }

    inline void openGraphicsSettings() {
        if (auto video_options_layer = VideoOptionsLayer::create()) {
            video_options_layer->show();
        }
    }

    inline void openQuests() {
        if (auto challenges_page = ChallengesPage::create()) {
            challenges_page->show();
        }
    }

    inline void resetLevel() {
        if (auto pl = PlayLayer::get()) pl->resetLevel();
    }

    inline void togglePracticeMode() {
        if (auto pl = PlayLayer::get()) {
            pl->togglePracticeMode(!pl->m_isPracticeMode);
        }
    }

    inline void resetVolume() {
        auto fmod_engine = FMODAudioEngine::sharedEngine();
        fmod_engine->setBackgroundMusicVolume(0.5f);
        fmod_engine->setEffectsVolume(0.5f);
    }

    inline void uncompleteLevel() {
        GDH::Utils::UncompleteLevel();
    }

    inline void openResourcesFolder() {
        geode::utils::file::openFolder(geode::dirs::getGameDir());
    }

    inline void openAppDataFolder() {
        auto path = geode::dirs::getSaveDir();
        geode::utils::file::openFolder(path);
    }

    inline void openGDHAppDataFolder() {
        geode::utils::file::openFolder(geode::Mod::get()->getSaveDir());
    }

    inline void triggerCrash() {
        volatile int* ptr = nullptr;
        *ptr = 0;
    }

    inline void restart() {
        geode::utils::game::restart(true);
    }
} }