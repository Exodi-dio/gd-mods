#ifdef GEODE_IS_WINDOWS
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MoreVideoOptionsLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"

GUI_HACK_CREATE("Invisible", "FPS Bypass", "", false);

$execute {
    auto& config = Config::get();
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Invisible").findHackByName("FPS");   
    hack.setGameVariableID(GameVar::UnlockFPS);
    hack.setEarlyInit(false);

    auto valueKey = hack.formatAdditionalSetting("value");

    hack.setHandler([&gui, &config, valueKey](bool enabled) {
        auto& hack = gui.getWindow("Invisible").findHackByName("Vertical Sync");  
        if (hack.getEnabled() && enabled) hack.toggle();

        auto gm = GameManager::get();
        if (gm->m_customFPSTarget == 0) gm->m_customFPSTarget = 60.f;

        auto fps_value = config.get<float>(valueKey, gm->m_customFPSTarget);
        fps_value = std::clamp(fps_value, 10.f, 10000.f);
        gm->m_customFPSTarget = fps_value;

        float dt = 1.f / (enabled ? fps_value : 60.f);
        CCDirector::get()->setAnimationInterval(dt);
    });
}

class $modify(FPSBypassGameManager, GameManager) {
    void updateCustomFPS() {
        geode::queueInMainThread([this] {
            auto& config = Config::get();
            if (m_customFPSTarget != 0) config.set<float>("invisible.fps::value", m_customFPSTarget); 
            auto enabled = getGameVariable(GameVar::UnlockFPS);
            
            auto& gui = GDH::Gui::get();
            auto& hack = gui.getWindow("Invisible").findHackByName("FPS");   
            hack.setEnabled(enabled);
        });
    }
};

class $modify(FPSBypassMoreVideoOptionsLayer, MoreVideoOptionsLayer) {
    void onToggle(cocos2d::CCObject* sender) {
        MoreVideoOptionsLayer::onToggle(sender);

        auto& gui = GDH::Gui::get();
        auto gm = GameManager::get();
        
        auto enabled = gm->getGameVariable(GameVar::UnlockFPS);
        auto& hack = gui.getWindow("Invisible").findHackByName("FPS");   
        hack.setEnabled(enabled);
    }
};
#endif