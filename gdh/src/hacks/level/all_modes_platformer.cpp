#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "All Modes Platformer", "Enables all modes in platformer levels", false);

class $modify(AllModesPlatformerGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("All Modes Platformer");

        hack.addHookPtr(self.getHook("GJBaseGameLayer::collisionCheckObjects").unwrap());
    }

    void collisionCheckObjects(PlayerObject* object, gd::vector<GameObject*>* objects, int objectCount, float dt) {
    GJBaseGameLayer::collisionCheckObjects(object, objects, objectCount, dt);

    if (!m_isPlatformer || !object || !objects) return;

    auto playerRect = object->getObjectRect();

    for (auto obj : *objects) {
        if (!obj) continue;

        if (obj->m_isGroupDisabled || obj->m_isDisabled || obj->hasBeenActivatedByPlayer(object)) 
            continue;

        if (obj->m_objectType != GameObjectType::WavePortal && obj->m_objectType != GameObjectType::SwingPortal)
            continue;

        if (!playerRect.intersectsRect(obj->getObjectRect()))
            continue;

        auto effectObj = static_cast<EffectGameObject*>(obj);
        
        if (this->canTouchObject(obj) && this->canBeActivatedByPlayer(object, effectObj)) {
            this->playerWillSwitchMode(object, obj);
            
            this->switchToFlyMode(object, obj, false, static_cast<int>(obj->m_objectType));

            obj->playShineEffect();
            obj->activatedByPlayer(object);
        }
    }
}
};