#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Shipcopter", "Changes swing mode physics to behave like a ship", false);

class $modify(ShipcopterPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Shipcopter");

        hack.addHookPtr(self.getHook("PlayerObject::pushButton").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::releaseButton").unwrap());
        hack.setCheating(true);
    }

    struct Fields {
        bool m_isHolding = false;
    };

    bool pushButton(PlayerButton button) {
        if (!m_gameLayer) return PlayerObject::pushButton(button);

        bool ret = PlayerObject::pushButton(button);
        if (ret && m_isSwing) {
            if (m_touchedRing && !m_isDashing) {
                m_fields->m_isHolding = false;
            } else {
                m_fields->m_isHolding = true;
                this->flipGravity(m_isUpsideDown, true);
            }
        }

        return ret;
    }

    bool releaseButton(PlayerButton button) {
        if (!m_gameLayer) return PlayerObject::releaseButton(button);

        bool ret = PlayerObject::releaseButton(button);
        if (ret && m_isSwing) {
            if (m_fields->m_isHolding) {
                m_fields->m_isHolding = false;
                this->flipGravity(!m_isUpsideDown, true);
            }
        }

        return ret;
    }
};

class $modify(ShipcopterPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Shipcopter");

        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (m_player1) {
            static_cast<ShipcopterPlayerObject*>(m_player1)->m_fields->m_isHolding = false;
        }

        if (m_player2) {
            static_cast<ShipcopterPlayerObject*>(m_player2)->m_fields->m_isHolding = false;
        }
    }
};