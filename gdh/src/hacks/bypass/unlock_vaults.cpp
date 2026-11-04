#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/SecretLayer2.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include "../../core/gui.hpp"

bool bypassStat = false;
int statValue = 0;

GUI_HACK_CREATE("Bypass", "Unlock Vaults", "Unlocks all Vaults and doors", false);

class $modify(UnlockVaultsGameManager, GameManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Unlock Vaults");        
        
        hack.addHookPtr(self.getHook("GameManager::getUGV").unwrap());
    }

    bool getUGV(char const* p0) {
        if (GameManager::getUGV(p0))
            return true;

        if (std::strcmp(p0, "4") == 0)
            return true;

        return false;
    }
};

class $modify(UnlockVaultsGameStatsManager, GameStatsManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Unlock Vaults");        
        
        hack.addHookPtr(self.getHook("GameStatsManager::isItemUnlocked").unwrap());
        hack.addHookPtr(self.getHook("GameStatsManager::getStat").unwrap());
    }

    bool isItemUnlocked(UnlockType p0, int p1) {
        if (GameStatsManager::isItemUnlocked(p0, p1))
            return true;

        if (p0 == UnlockType::GJItem && p1 >= 1 && p1 <= 4) {
            return true;
        }

        return false;
    }

    int getStat(const char* key) {
        if (bypassStat) {
            bypassStat = false;
            return statValue;
        }

        return GameStatsManager::getStat(key);
    }
};

class $modify(UnlockVaultsOptionsLayer, OptionsLayer) {   
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Unlock Vaults");        
        
        hack.addHookPtr(self.getHook("OptionsLayer::customSetup").unwrap());
        hack.addHookPtr(self.getHook("OptionsLayer::onSecretVault").unwrap());
    }

    void customSetup() {
        bypassStat = true;
        statValue = 10;
        
        OptionsLayer::customSetup();
    }

    void onSecretVault(cocos2d::CCObject* sender) {
        bypassStat = true;
        statValue = 10;

        OptionsLayer::onSecretVault(sender);
    }
};

class $modify(UnlockVaultsSecretLayer2, SecretLayer2) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Unlock Vaults");        
        
        hack.addHookPtr(self.getHook("SecretLayer2::init").unwrap());
        hack.addHookPtr(self.getHook("SecretLayer2::onSecretLevel").unwrap());
    }

    bool init() {
        bypassStat = true;
        statValue = 200;

        return SecretLayer2::init();
    }

    void onSecretLevel(cocos2d::CCObject* sender) {
        bypassStat = true;
        statValue = 200;

        SecretLayer2::onSecretLevel(sender);
    }
};

class $modify(UnlockVaultsCreatorLayer, CreatorLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Unlock Vaults");        
        
        hack.addHookPtr(self.getHook("CreatorLayer::init").unwrap());
        hack.addHookPtr(self.getHook("CreatorLayer::onSecretVault").unwrap());
    }

    bool init() {
        bypassStat = true;
        statValue = 50;

        return CreatorLayer::init();
    }

    void onSecretVault(cocos2d::CCObject* sender) {
        bypassStat = true;
        statValue = 51;

        CreatorLayer::onSecretVault(sender);
    }
};