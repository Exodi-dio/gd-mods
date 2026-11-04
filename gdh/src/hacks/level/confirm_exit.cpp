#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Confirm Exit", "Warning before level exit", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Level").findHackByName("Confirm Exit");   
    hack.setGameVariableID(GameVar::ConfirmExit);
}