#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Coins In Practice", "The ability to collect coins in practice", false);

class $modify(CoinsInPracticeGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Coins In Practice");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::collisionCheckObjects").unwrap());
    }

    void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* sectionObjects, int objectCount, float dt) {
        if (!m_isPracticeMode)
            return GJBaseGameLayer::collisionCheckObjects(player, sectionObjects, objectCount, dt);
        
        auto first = sectionObjects->begin();
        auto last = sectionObjects->begin() + std::min(static_cast<size_t>(objectCount), sectionObjects->size());
        auto playerRect = player->getObjectRect();

        while (true) {
            auto it = std::find_if(first, last, [&](GameObject* obj) {
                if (obj->m_objectType != GameObjectType::SecretCoin && obj->m_objectType != GameObjectType::UserCoin) return false;
                if (obj->m_isGroupDisabled || obj->m_isDisabled) return false;
                if (obj->hasBeenActivatedByPlayer(player)) return false;

                if (!playerRect.intersectsRect(obj->getObjectRect())) return false;
                if (!this->canTouchObject(obj)) return false;

                return true;
            });

            if (it == last) break;

            GameObject* obj = *it;

            if (obj->m_objectType == GameObjectType::SecretCoin || obj->m_objectType == GameObjectType::UserCoin) {
                obj->triggerObject(this, player->m_uniqueID, nullptr);
                obj->triggerActivated(0.0f);
                this->destroyObject(obj);
                this->gameEventTriggered(GJGameEvent::UserCoin, 0, 0);
            }

            first = it + 1;
        }

        GJBaseGameLayer::collisionCheckObjects(player, sectionObjects, objectCount, dt);
    }
};