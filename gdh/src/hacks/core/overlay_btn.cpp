#ifdef GEODE_IS_MOBILE
#include <Geode/Geode.hpp>
#include <Geode/binding/EditorPauseLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include "../../core/config.hpp"
#include "../../interface/cocos/overlay_button.hpp"

class $modify(OverlayButtonVisiblityPL, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (Config::get().get<bool>("ui_icon.hide_on_game", true))
            OverlayButton::get()->setVisible(false);

        return true;
    }

    void pauseGame(bool unfocused) {
        PlayLayer::pauseGame(unfocused);

        OverlayButton::get()->setVisible(true);
    }
    
    void resume() {
        PlayLayer::resume();
        
        if (Config::get().get<bool>("ui_icon.hide_on_game", true))
            OverlayButton::get()->setVisible(false);
    }

    void resumeAndRestart(bool fromStart) {
        PlayLayer::resumeAndRestart(fromStart);

        if (Config::get().get<bool>("ui_icon.hide_on_game", true))
            OverlayButton::get()->setVisible(false);
    }

    void showEndLayer() {
        PlayLayer::showEndLayer();
        OverlayButton::get()->setVisible(true);
    }
};

class $modify(OverlayButtonVisiblityLEL, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;

        if (Config::get().get<bool>("ui_icon.hide_on_editor", false))
            OverlayButton::get()->setVisible(false);

        return true;
    }
};

class $modify(OverlayButtonVisiblityEUI, EditorUI) {
    void onPause(cocos2d::CCObject *sender) {
        EditorUI::onPause(sender);

        OverlayButton::get()->setVisible(true);
    }
};

class $modify(OverlayButtonVisiblityEPL, EditorPauseLayer) {
    void onResume(cocos2d::CCObject *sender) {
        EditorPauseLayer::onResume(sender);

        if (Config::get().get<bool>("ui_icon.hide_on_editor", false))
            OverlayButton::get()->setVisible(false);
    }
};

class $modify(OverlayButtonVisiblityELL, EndLevelLayer) {
    void onRestartCheckpoint(cocos2d::CCObject *sender) {
        EndLevelLayer::onRestartCheckpoint(sender);

        if (Config::get().get<bool>("ui_icon.hide_on_game", true) && !this->m_exiting)
            OverlayButton::get()->setVisible(false);
    }

    void onReplay(cocos2d::CCObject *sender) {
        EndLevelLayer::onRestartCheckpoint(sender);

        if (Config::get().get<bool>("ui_icon.hide_on_game", true) && !this->m_exiting)
            OverlayButton::get()->setVisible(false);
    }
};
#endif