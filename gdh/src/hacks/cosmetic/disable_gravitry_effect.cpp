#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Disable Gravity Effect", "Disables the scaling effect on all orbs", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Cosmetic").findHackByName("Disable Gravity Effect");   
    hack.setGameVariableID(GameVar::DisableGravityEffect);
}   