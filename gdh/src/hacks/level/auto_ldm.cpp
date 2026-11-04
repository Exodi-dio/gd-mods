#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Auto LDM", "", false);

class $modify(AutoLDMLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Auto LDM");        
        
        hack.addHookPtr(self.getHook("LevelInfoLayer::init").unwrap());
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        level->m_lowDetailModeToggled = true;

        return true;
    }
};