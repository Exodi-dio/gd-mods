#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Swing Fire", "Hides animation and fire particles from the swing copter", false);

class $modify(NoSwingFirePlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Swing Fire");        
        
        hack.addHookPtr(self.getHook("PlayerObject::toggleSwingMode").unwrap());
    }

    void toggleSwingMode(bool enable, bool noEffects) {
        PlayerObject::toggleSwingMode(enable, noEffects);
        m_swingFireMiddle->setOpacity(0);
        m_swingFireBottom->setOpacity(0);
        m_swingFireTop->setOpacity(0);
        m_swingBurstParticles1->setOpacity(0);
        m_swingBurstParticles2->setOpacity(0);
    }
};