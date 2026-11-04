#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Core", "Practice Music", "Unlocks RobTop's \"Practice Music Sync\"", false);

class $modify(PracticeMusicGameStatsManager, GameStatsManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Core").findHackByName("Practice Music"); 
        
        hack.setEarlyInit(false);
        hack.setHandler([](bool enabled) {
            auto gsm = GameStatsManager::get();
            auto gjbgl = GJBaseGameLayer::get();

            if (gsm) gsm->toggleEnableItem(UnlockType::GJItem, 17, enabled);
            if (gjbgl) gjbgl->toggleMusicInPractice();
        });

        hack.addHookPtr(self.getHook("GameStatsManager::isItemUnlocked").unwrap());
    }

    bool isItemUnlocked(UnlockType type, int key) {
        if (GameStatsManager::isItemUnlocked(type, key)) return true;

        if (type == UnlockType::GJItem && key == 17)
            return true;

        return false;
    }
};