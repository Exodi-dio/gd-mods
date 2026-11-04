#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Hide Attempts", "Hides the attempt count in-game", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Attempts");   
    hack.setGameVariableID(GameVar::HideAttemptsNormal);
}