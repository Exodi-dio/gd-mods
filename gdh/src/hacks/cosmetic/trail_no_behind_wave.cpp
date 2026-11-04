#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Trail No Behind Wave", "Disables the player's normal trail behind the wave's trail", false);

class $modify(TrailNoBehindWavePlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Trail No Behind Wave");        
        
        hack.addHookPtr(self.getHook("PlayerObject::activateStreak").unwrap());
    }

    void activateStreak() {
        PlayerObject::activateStreak();

        if (this->m_isDart)
            this->m_regularTrail->stopStroke();
    }
};