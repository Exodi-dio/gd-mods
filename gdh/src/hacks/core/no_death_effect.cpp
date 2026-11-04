#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Core", "No Death Effect", "Upon death, the cube will not emit an exploding effect", false);

class $modify(NoDeathEffectPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Core").findHackByName("No Death Effect");        
        
        hack.addHookPtr(self.getHook("PlayerObject::playDeathEffect").unwrap());
    }

    void playDeathEffect() {}
};