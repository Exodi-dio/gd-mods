#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"

GUI_HACK_CREATE("Core", "Safe Mode", "Disables progress on levels", false);

class $modify(SafeModePlayLayer, PlayLayer) {
    static void onModify(auto& self) {   
        (void) self.setHookPriority("PlayLayer::destroyPlayer", geode::Priority::Early); 
        (void) self.setHookPriority("PlayLayerr::resetLevel", geode::Priority::Early); 
        (void) self.setHookPriority("PlayLayer::levelComplete", geode::Priority::Early); 
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        bool enabled = Config::get().get("core.safe_mode", false);
        bool testmode = m_isTestMode;
        m_isTestMode = enabled ? true : testmode;

        PlayLayer::destroyPlayer(player, obj);
        m_isTestMode = testmode;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (Config::get().get("core.safe_mode", false)) {
            m_level->m_attempts = m_level->m_attempts - 1;
        }
    }

    void levelComplete() {
        bool enabled = Config::get().get("core.safe_mode", false);
        bool testmode = m_isTestMode;
        m_isTestMode = enabled ? true : testmode;
        PlayLayer::levelComplete();
        m_isTestMode = testmode;
    }
};

class $modify(SafeModePlayerObject, PlayerObject) {
    static void onModify(auto& self) {     
        (void) self.setHookPriority("PlayerObject::incrementJumps", geode::Priority::Early);
    }


    void incrementJumps() {
        if (!Config::get().get("core.safe_mode", false))
            PlayerObject::incrementJumps();
    }
};