#include <Geode/Geode.hpp>
#include <Geode/modify/GameToolbox.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Short Numbers", "All numbers are displayed in full\n(For example, \"1.5M\" becomes \"1500000\")", false);

class $modify(NoShortNumbersGameToolbox, GameToolbox) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Short Numbers");        
        
        hack.addHookPtr(self.getHook("GameToolbox::intToShortString").unwrap());
    }

    static gd::string intToShortString(int value) {
        return fmt::format("{}", value);
    }
};