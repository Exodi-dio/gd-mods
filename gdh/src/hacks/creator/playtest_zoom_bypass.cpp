#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Playtest Zoom Bypass", "Disables auto-zoom 1.x when starting playtesting. 2.1 players will remember", false);

static bool m_onPlaytest = false;

class $modify(PlaytestZoomBypassLevelEditorLayer, LevelEditorLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Playtest Zoom Bypass");        
        hack.addHookPtr(self.getHook("LevelEditorLayer::onPlaytest").unwrap());
    }

    void onPlaytest() {
        m_onPlaytest = true;
        LevelEditorLayer::onPlaytest();
        m_onPlaytest = false;
    }
};

class $modify(PlaytestZoomBypassGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Playtest Zoom Bypass");        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::updateZoom").unwrap());
    }

    void updateZoom(float zoom, float duration, int easing, float rate, int uniqueID, int controlID) {
        if (m_onPlaytest) {
            return;
        }

        GJBaseGameLayer::updateZoom(zoom, duration, easing, rate, uniqueID, controlID);
    }
};