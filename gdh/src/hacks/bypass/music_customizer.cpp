#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Music Customizer", "Unlocks RobTop's Music Customizer", false);

class $modify(MusicUnlockerGameStatsManager, GameStatsManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Music Customizer");        
        
        hack.addHookPtr(self.getHook("GameStatsManager::isItemUnlocked").unwrap());
    }

    bool isItemUnlocked(UnlockType type, int key) {
        if (GameStatsManager::isItemUnlocked(type, key)) return true;

        if (type == UnlockType::GJItem && key == 16)
            return true;

        return false;
    }
};