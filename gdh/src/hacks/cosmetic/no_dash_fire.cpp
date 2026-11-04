#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Dash Fire", "", false);

class $modify(NoDashFirelayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Dash Fire");        
        
        hack.addHookPtr(self.getHook("PlayerObject::startDashing").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::stopDashing").unwrap());
    }

    void startDashing(DashRingObject *object) {
        if (object == nullptr) {
            return PlayerObject::startDashing(object);
        }

        bool orig = object->m_hasNoEffects;

        object->m_hasNoEffects = true;
        PlayerObject::startDashing(object);
        object->m_hasNoEffects = orig;

        if (m_dashFireSprite) {
            m_dashFireSprite->setVisible(false);
            m_dashFireSprite->stopAllActions();
        }
    }

    void stopDashing() {
        bool orig = m_playEffects;

        m_playEffects = false;
        PlayerObject::stopDashing();
        m_playEffects = orig;
    }
};