#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Hide Playtest Text", "Hides text in the top left when using start positions or ignore damage", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Playtest Text");   
    hack.setGameVariableID(GameVar::HidePlaytestText);
}