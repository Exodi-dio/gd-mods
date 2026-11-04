#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Mini Cube Icon", "", false);

class $modify(MiniCubeIconPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Mini Cube Icon");        
        
        hack.setGameVariableID(GameVar::DefaultMiniIcon);
        hack.addHookPtr(self.getHook("PlayerObject::updatePlayerFrame").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::updatePlayerRollFrame").unwrap());
    }

    void updatePlayerFrame(int frame) {
        m_defaultMiniIcon = true;
        PlayerObject::updatePlayerFrame(0);
    }
    
    void updatePlayerRollFrame(int frame) {
        m_defaultMiniIcon = true;
        PlayerObject::updatePlayerRollFrame(0);
    }
};