#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Robot Fire", "Hides robot boost and fire particles", false);

class $modify(NoRobotFirePlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Robot Fire");        
        
        hack.addHookPtr(self.getHook("PlayerObject::toggleRobotMode").unwrap());
    }

    void toggleRobotMode(bool enable, bool noEffects) {
        PlayerObject::toggleRobotMode(enable, noEffects);
        m_robotFire->setOpacity(0);
        m_robotBurstParticles->setOpacity(0);
    }
};