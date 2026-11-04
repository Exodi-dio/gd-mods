#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Custom Object Bypass", "Removes the limit restricted to 1000 objects", false);

class $modify(CustomObjectBypassEditorUI, EditorUI) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Custom Object Bypass");        
        
        hack.addHookPtr(self.getHook("EditorUI::onNewCustomItem").unwrap());
    }

    void onNewCustomItem(CCObject* sender) {        
        auto gm = GameManager::get();
        if (gm) {
            cocos2d::CCArray* objects;
            if (m_selectedObjects->count()) {
                objects = m_selectedObjects;
            }
            else {
                objects = cocos2d::CCArray::create();
                objects->addObject(m_selectedObject);
            }
            gm->addNewCustomObject(copyObjects(objects, false, false));
            m_selectedObjectIndex = 0;
            reloadCustomItems();
        }
    }
};