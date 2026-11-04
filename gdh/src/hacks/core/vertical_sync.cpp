#ifdef GEODE_IS_WINDOWS
#include <Geode/Geode.hpp>
#include <Geode/binding/AppDelegate.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Invisible", "Vertical Sync", "", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Invisible").findHackByName("Vertical Sync");   
    hack.setGameVariableID(GameVar::VerticalSync);
    hack.setEarlyInit(false);

    hack.setHandler([](bool enabled) {
        auto appDelegate = AppDelegate::get();
        if (appDelegate)
            appDelegate->toggleVerticalSync(enabled);
    });
}
#endif