#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"

GUI_HACK_CREATE("Core", "No Respawn Blink", "Upon respawning, the cube will not produce an unpleasant flicker", false);

class $modify(NoRespawnBlinkPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Core").findHackByName("No Respawn Blink");        
        
        // hack.addHookPtr(self.getHook("PlayerObject::playSpawnEffect").unwrap());
    }

    void playSpawnEffect() {
        if (!Config::get().get("core.no_respawn_blink", false)) {
            PlayerObject::playSpawnEffect();
        }
    }
};