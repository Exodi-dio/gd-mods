#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Disable Orb Scale", "Disables the scaling effect on all orbs", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Cosmetic").findHackByName("Disable Orb Scale");   
    hack.setGameVariableID(GameVar::DisableOrbScale);
}