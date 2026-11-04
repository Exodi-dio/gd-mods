#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Treasure Room", "Unlocks the Treasure Room", false);

class $modify(TreasureRoomGameManager, GameManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Treasure Room");        
        
        hack.addHookPtr(self.getHook("GameManager::getUGV").unwrap());
    }

    bool getUGV(char const* p0) {
        if (GameManager::getUGV(p0))
            return true;

        if (std::strcmp(p0, "5") == 0)
            return true;

        return false;
    }
};