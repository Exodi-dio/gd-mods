#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Hide Pause Button", "Removes the pause button when the cursor is shown", false);

class $modify(HidePauseButtonUILayer, UILayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Pause Button");        
        
        hack.addHookPtr(self.getHook("UILayer::init").unwrap());
    }

    bool init(GJBaseGameLayer *layer) {
        if (!UILayer::init(layer)) return false;        
        
        auto menu = getChildByType<cocos2d::CCMenu>(0);
        auto btn = menu->getChildByType<CCMenuItemSpriteExtra>(0);

        if (menu && btn)
            btn->getNormalImage()->setVisible(false);

        return true;
    }
};