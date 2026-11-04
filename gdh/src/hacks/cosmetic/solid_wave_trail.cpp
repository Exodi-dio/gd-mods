#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Solid Wave Trail", "Disables wave blending", false);

class $modify(SolidWaveTrailPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Solid Wave Trail");        
        
        hack.addHookPtr(self.getHook("PlayerObject::setupStreak").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::updateStreakBlend").unwrap());
    }

    void setupStreak() {
        auto gm = GameManager::get();

        bool switchWaveTrailColor = m_switchWaveTrailColor;
        auto playerColor = gm->m_playerColor;

        m_switchWaveTrailColor = false;
        gm->m_playerColor = 15;

        PlayerObject::setupStreak();

        m_switchWaveTrailColor = switchWaveTrailColor;
        gm->m_playerColor = playerColor;

        if (m_waveTrail) {
            m_waveTrail->m_isSolid = true;
        }
    }

    void updateStreakBlend(bool blend) {
        PlayerObject::updateStreakBlend(blend);
        if (m_waveTrail) {
            m_waveTrail->m_isSolid = true;
            m_waveTrail->setBlendFunc({GL_ONE, GL_ZERO});
        }
    }
};