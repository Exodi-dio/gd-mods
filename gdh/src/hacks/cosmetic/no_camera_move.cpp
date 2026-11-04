#include <Geode/Geode.hpp>
#include <Geode/modify/CameraTriggerGameObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Camera Move", "Disables camera movement via trigger", true);

class $modify(NoCameraMoveCameraTriggerGameObject, CameraTriggerGameObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Camera Move");        
        
        hack.addHookPtr(self.getHook("CameraTriggerGameObject::triggerObject").unwrap());
    }

    void triggerObject(GJBaseGameLayer *p0, int p1, const gd::vector<int> *p2) {}
};