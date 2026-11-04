#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Auto Pickup Coins", "Collects all coins in the level", true);

class $modify(AutoPickupCoinsPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<GameObject*> coinsObjects;
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Auto Pickup Coins");        
        
        hack.addHookPtr(self.getHook("PlayLayer::addObject").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
    }

    void addObject(GameObject* obj) {
        PlayLayer::addObject(obj);
        
        int id = obj->m_objectID;
        if (id == 1329 || id == 142)
            m_fields->coinsObjects.push_back(obj);
    }


    void resetLevel() {
        PlayLayer::resetLevel();

        for (auto* coin : m_fields->coinsObjects) {
            if (!coin) continue;
            destroyObject(coin);
            pickupItem(static_cast<EffectGameObject*>(coin));
        }
    }
};