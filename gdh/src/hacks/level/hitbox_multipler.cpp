#include <Geode/Geode.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/OBB2D.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Level", "Hitbox Multiplier", "Multiplies object hitbox sizes", true);

static GameObject* g_rotatedObj = nullptr;

static float getMultiplier(GameObject* obj) {
    if (!obj) return 1.f;
    
    auto gjbgl = GJBaseGameLayer::get();
    auto& config = Config::get();

    if (!gjbgl || !config.get<bool>("level.hitbox_multiplier", false)) return 1.f;

    if (typeinfo_cast<PlayerObject*>(obj))
        return config.get<float>("level.hitbox_multiplier::player", 1.f);
    if (obj->m_objectType == GameObjectType::Hazard || obj->m_objectType == GameObjectType::AnimatedHazard)
        return config.get<float>("level.hitbox_multiplier::hazard", 1.f);
    if (obj->m_objectType == GameObjectType::Solid || obj->m_objectType == GameObjectType::Slope)
        return config.get<float>("level.hitbox_multiplier::solid", 1.f);

    return 1.f;
}

static void forceHitboxRecalculation();

class $modify(HitboxMultiplierGameObject, GameObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Hitbox Multiplier");
        
        hack.addHookPtr(self.getHook("GameObject::getObjectRect").unwrap());
        hack.addHookPtr(self.getHook("GameObject::updateOrientedBox").unwrap());

        hack.setCustomWindowImGui([
            pKey = hack.formatAdditionalSetting("player"),
            sKey = hack.formatAdditionalSetting("solid"),
            hKey = hack.formatAdditionalSetting("hazard")
        ] {
            bool p = ImGuiWidgetConfig::DragFloat("##Player Multiplier", pKey, 0.05f, 0.f, 10.f, 1.f, "Player Multiplier: %.2fx");
            bool s = ImGuiWidgetConfig::DragFloat("##Solid Multiplier", sKey, 0.05f, 0.f, 10.f, 1.f, "Solid Multiplier: %.2fx");
            bool h = ImGuiWidgetConfig::DragFloat("##Hazard Multiplier", hKey, 0.05f, 0.f, 10.f, 1.f, "Hazard Multiplier: %.2fx");

            if (p || s || h) {
                forceHitboxRecalculation();
            }
        });

        hack.setCustomWindowCocos([
            pKey = hack.formatAdditionalSetting("player"),
            sKey = hack.formatAdditionalSetting("solid"),
            hKey = hack.formatAdditionalSetting("hazard")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);

            auto onChange = [](float) { forceHitboxRecalculation(); };
            popup->addConfigFloatInput("Player", pKey, 0.f, 10.f, 1.f, onChange);
            popup->addConfigFloatInput("Solid", sKey, 0.f, 10.f, 1.f, onChange);
            popup->addConfigFloatInput("Hazard", hKey, 0.f, 10.f, 1.f, onChange);
        });

        hack.setHandler([](bool enabled) {
            forceHitboxRecalculation();
        });
    }

    struct Fields { 
        float m_origRadius = -1.f; 
    };

    cocos2d::CCRect getObjectRect(float x, float y) {
        if (m_fields->m_origRadius < 0.f) {
            m_fields->m_origRadius = m_objectRadius;
        }

        float mult = getMultiplier(this);
        m_objectRadius = m_fields->m_origRadius * mult;

        return GameObject::getObjectRect(x * mult, y * mult);
    }

    void updateOrientedBox() {
        g_rotatedObj = this;
        GameObject::updateOrientedBox();
    }
};

static void forceHitboxRecalculation() {
    auto gjbgl = GJBaseGameLayer::get();
    if (!gjbgl || !gjbgl->m_objects) return;

    for (auto obj : CCArrayExt<GameObject*>(gjbgl->m_objects)) {
        if (!obj) continue;
        auto customObj = static_cast<HitboxMultiplierGameObject*>(obj);
        if (customObj->m_fields->m_origRadius >= 0.f) {
            obj->m_objectRadius = customObj->m_fields->m_origRadius;
        }

        obj->setObjectRectDirty(true);
        obj->setOrientedRectDirty(true);
    }
}

class $modify(HitboxMultiplierOBB2D, OBB2D) {
    void calculateWithCenter(cocos2d::CCPoint center, float width, float height, float rotation) {
        if (!g_rotatedObj || !GJBaseGameLayer::get() || !Config::get().get<bool>("level.hitbox_multiplier", false)) {
            g_rotatedObj = nullptr;
            return OBB2D::calculateWithCenter(center, width, height, rotation);
        }

        float mult = getMultiplier(g_rotatedObj);
        OBB2D::calculateWithCenter(center, width * mult, height * mult, rotation);
        g_rotatedObj = nullptr;
    }
};