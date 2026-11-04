#include <Geode/Geode.hpp>
#include <Geode/modify/GameToolbox.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Short Numbers", "All numbers are displayed in full\n(For example, \"1.5M\" becomes \"1500000\")", false);

// TODO: GameToolbox::intToShortString signature changed in GD 2.2091 (now TodoReturn(int, bool))
// Hook disabled until signature is confirmed
