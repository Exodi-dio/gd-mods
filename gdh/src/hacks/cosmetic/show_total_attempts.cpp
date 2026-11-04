#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Show Total Attempts", "", false);

class $modify(ShowTotalAttemptsPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Show Total Attempts");        
        
        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_attemptLabel->setCString(fmt::format("Attempt {}", static_cast<int>(m_level->m_attempts)).c_str());
    }
};