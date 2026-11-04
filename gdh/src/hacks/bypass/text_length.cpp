#include <Geode/Geode.hpp>
#include <Geode/modify/CCTextInputNode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Text Length", "Removes the text input limit", false);

class $modify(TextLengthCCTextInputNode, CCTextInputNode) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Text Length");        
        
        hack.addHookPtr(self.getHook("CCTextInputNode::updateLabel").unwrap());
    }

    void updateLabel(gd::string str) {
        this->setMaxLabelLength(9999);
        CCTextInputNode::updateLabel(str);
    }
};