#include <Geode/Geode.hpp>
#include <Geode/modify/CCParticleSystemQuad.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Cosmetic", "No Particles", "Disables aspects of the particle system", false);

static const char* g_effects[][2] = {
    {"Boost Effect", "boost"},
    {"Bubble Effect", "bubble"},
    {"Bump Effect", "bump"},
    {"Burst Effect", "burst"},
    {"Chest Effects", "chest"},
    {"Coin Effect", "coin"},
    {"Dash Effect", "dash"},
    {"Destroy Effect", "destroy"},
    {"Drag Effect", "drag"},
    {"End Portal Effect", "end_portal"},
    {"Explode Effects", "explode"},
    {"Fireball Effect", "fireball"},
    {"Firework Effect", "firework"},
    {"Glitter", "glitter"},
    {"Key Effect", "key"},
    {"Land Effect", "land"},
    {"Portal Effect", "portal"},
    {"Ring Effect", "ring"},
    {"Ship Drag", "ship_drag"},
    {"Speed Effect", "speed"},
    {"Star Effect", "star"},
    {"Swing Burst", "swing_burst"}
};

class $modify(NoParticlesCCParticleSystemQuad, CCParticleSystemQuad) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Particles");

        hack.addHookPtr(self.getHook("cocos2d::CCParticleSystemQuad::create").unwrap());

        hack.setCustomWindowImGui([] {
            auto& hackRef = GDH::Gui::get().getWindow("Cosmetic").findHackByName("No Particles");
            for (const auto& effect : g_effects) {
                std::string fullKey = hackRef.formatAdditionalSetting(effect[1]);
                ImGuiWidgetConfig::Checkbox(effect[0], fullKey, true);
            }
        });

        hack.setCustomWindowCocos([](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            auto& hackRef = GDH::Gui::get().getWindow("Cosmetic").findHackByName("No Particles");
            for (const auto& effect : g_effects) {
                std::string fullKey = hackRef.formatAdditionalSetting(effect[1]);
                popup->addConfigToggle(effect[0], fullKey, true);
            }
        });
    }

    static CCParticleSystemQuad* create(const char* plistFile, bool idk) {
        auto* particle = cocos2d::CCParticleSystemQuad::create(plistFile, idk);
        if (!particle || !plistFile) return particle;

        std::string plist = plistFile;
        auto& config = Config::get();
        bool shouldHide = false;

        auto isEnabled = [&](const char* key) {
            std::string fullKey = std::string("cosmetic.no_particles::") + key;
            return config.get<bool>(fullKey, true);
        };

        if (isEnabled("boost") && (plist.find("boost_") != std::string::npos)) shouldHide = true;
        else if (isEnabled("bubble") && plist == "bubbleEffect.plist") shouldHide = true;
        else if (isEnabled("bump") && plist == "bumpEffect.plist") shouldHide = true;
        else if (isEnabled("burst") && (plist == "burstEffect.plist" || plist == "burstEffect2.plist")) shouldHide = true;
        else if (isEnabled("chest") && (plist == "chestOpen.plist" || plist == "chestOpened.plist")) shouldHide = true;
        else if (isEnabled("coin") && (plist == "coinPickupEffect.plist" || plist == "coinEffect.plist")) shouldHide = true;
        else if (isEnabled("dash") && plist == "dashEffect.plist") shouldHide = true;
        else if (isEnabled("destroy") && plist == "glassDestroy01.plist") shouldHide = true;
        else if (isEnabled("drag") && plist == "dragEffect.plist") shouldHide = true;
        else if (isEnabled("end_portal") && plist == "endEffectPortal.plist") shouldHide = true;
        else if (isEnabled("explode") && (plist == "explodeEffect.plist" || plist == "explodeEffectVortex.plist" || plist == "explodeEffectGrav.plist" || plist.find("PlayerExplosion") != std::string::npos)) shouldHide = true;
        else if (isEnabled("fireball") && plist == "fireballEffect.plist") shouldHide = true;
        else if (isEnabled("firework") && plist == "firework.plist") shouldHide = true;
        else if (isEnabled("glitter") && plist == "glitterEffect.plist") shouldHide = true;
        else if (isEnabled("key") && plist == "keyEffect.plist") shouldHide = true;
        else if (isEnabled("land") && plist == "landEffect.plist") shouldHide = true;
        else if (isEnabled("portal") && (plist.find("portalEffect") != std::string::npos)) shouldHide = true;
        else if (isEnabled("ring") && plist == "ringEffect.plist") shouldHide = true;
        else if (isEnabled("ship_drag") && plist == "shipDragEffect.plist") shouldHide = true;
        else if (isEnabled("speed") && (plist.find("speedEffect_") != std::string::npos)) shouldHide = true;
        else if (isEnabled("star") && plist == "starEffect.plist") shouldHide = true;
        else if (isEnabled("swing_burst") && plist == "swingBurstEffect.plist") shouldHide = true;

        if (shouldHide) {
            particle->setTotalParticles(0);
            particle->stopSystem();

            particle->setOpacity(0);
            particle->setVisible(false);
        }

        return particle;
    }
};