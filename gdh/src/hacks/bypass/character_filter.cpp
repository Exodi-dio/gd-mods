#include <Geode/Geode.hpp>
#include <Geode/modify/CCTextInputNode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Character Filter", "Lets you input any character in all text inputs", false);

class $modify(CharacterFilterCCTextInputNode, CCTextInputNode) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Character Filter");        
        
        hack.addHookPtr(self.getHook("CCTextInputNode::updateLabel").unwrap());
    }

    void updateLabel(gd::string str) {
        this->setAllowedChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ");
        CCTextInputNode::updateLabel(std::move(str));
    }
};