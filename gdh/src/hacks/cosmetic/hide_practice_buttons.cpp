#ifdef GEODE_IS_DESKTOP
#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Hide Practice Buttons", "Hides the checkpoint buttons shown in practice mode", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Practice Buttons");   
    hack.setGameVariableID(GameVar::HidePracticeButtons);
    hack.setHandler([](bool enabled) {
        if (auto pl = PlayLayer::get()) {
            if (!pl->m_uiLayer || !pl->m_uiLayer->m_checkpointMenu || !pl->m_isPracticeMode) return;
            pl->m_uiLayer->m_checkpointMenu->setVisible(!enabled);
        }
    });
}   
#endif