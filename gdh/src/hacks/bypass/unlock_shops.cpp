#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Unlock Shops", "Unlocks all shops", false);

class $modify(UnlockShopsGameManager, GameManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Unlock Shops");        
        
        hack.addHookPtr(self.getHook("GameManager::getUGV").unwrap());
    }

    bool getUGV(char const* p0) {
        if (GameManager::getUGV(p0))
            return true;

        if (std::strcmp(p0, "11") == 0 ||
            std::strcmp(p0, "20") == 0 ||
            std::strcmp(p0, "35") == 0 ||
            std::strcmp(p0, "34") == 0)
            return true;

        return false;
    }
};