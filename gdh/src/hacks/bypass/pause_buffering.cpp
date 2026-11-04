#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"

GUI_HACK_CREATE("Bypass", "Pause Buffering", "", false);

class $modify(PauseBufferingUILayer, UILayer) {
    void onPause(cocos2d::CCObject* sender) {
        auto pl = PlayLayer::get();
        if (!pl) return UILayer::onPause(sender);

        auto& config = Config::get();
        bool enabled = config.get<bool>("bypass.pause_buffering", false) || config.get<bool>("level.frame_stepper", false);
        auto pauseCounter = pl->m_gameState.m_pauseCounter;

        pl->m_gameState.m_pauseCounter = enabled ? 0 : pauseCounter;
        UILayer::onPause(sender);
        pl->m_gameState.m_pauseCounter = pauseCounter;
    }
};