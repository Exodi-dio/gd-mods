#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Trail Always Off", "Always turns off the player's trail", false);

class $modify(TrailAlwaysOffPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Trail Always Off");        
        
        hack.addHookPtr(self.getHook("PlayerObject::activateStreak").unwrap());
    }

    void activateStreak() {
        PlayerObject::activateStreak();
        this->m_regularTrail->stopStroke();
    }
};