#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Auto Disable Shake", "", false);

class $modify(AutoDisableShakeLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Auto Disable Shake");        
        
        hack.addHookPtr(self.getHook("LevelInfoLayer::init").unwrap());
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        level->m_disableShakeToggled = true;

        return true;
    }
};