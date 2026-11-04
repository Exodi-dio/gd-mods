#include <unordered_map>
#include <Geode/Geode.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/utils.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Cosmetic", "Show Touches", "Displays visual indicators on screen touches", false);

struct TouchInfo {
    cocos2d::CCPoint lastPos;
    float currentOpacity = 0.f;
    bool released = false;
};

static std::unordered_map<cocos2d::CCTouch*, TouchInfo> g_touches;

static void renderTouches() {
    auto& config = Config::get();
    if (!config.get<bool>("cosmetic.show_touches", false) || g_touches.empty()) {
        return;
    }

    float dt = cocos2d::CCDirector::sharedDirector()->getDeltaTime();
    float fadeSpeed = config.get<float>("cosmetic.show_touches::fade_speed", 6.f);

    bool fill = config.get<bool>("cosmetic.show_touches::fill", true);
    float scale = config.get<float>("cosmetic.show_touches::scale", 0.5f);
    int stroke = config.get<int>("cosmetic.show_touches::stroke", 0);
    float maxOpacity = config.get<float>("cosmetic.show_touches::opacity", 0.5f);

    int radius = static_cast<int>(scale * 16.f);
    int segments = static_cast<int>(scale * 32.f);

    auto color4f = GDH::Utils::hexToColor4F(
        config.get<std::string>("cosmetic.show_touches::color", "#FFFFFFFF")
    );

    cocos2d::ccGLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::erase_if(g_touches, [dt, fadeSpeed, maxOpacity](auto& item) {
        auto* touch = item.first;
        auto& info = item.second;

        if (!info.released) {
            info.lastPos = touch->getLocation();
            info.currentOpacity = std::min(maxOpacity, info.currentOpacity + fadeSpeed * dt);
        } else {
            info.currentOpacity -= fadeSpeed * dt;
        }
        return info.currentOpacity <= 0.f;
    });

    for (const auto& [touch, info] : g_touches) {
        if (info.currentOpacity <= 0.f) continue;

        cocos2d::ccDrawColor4B(
            static_cast<GLubyte>(color4f.r * 255.f),
            static_cast<GLubyte>(color4f.g * 255.f),
            static_cast<GLubyte>(color4f.b * 255.f),
            static_cast<GLubyte>(info.currentOpacity * 255.f)
        );

        if (fill) {
            cocos2d::ccDrawFilledCircle(info.lastPos, radius, 0, segments);
            continue;
        }

        cocos2d::ccDrawCircle(info.lastPos, radius, 360, segments, false);
        
        for (int i = 0; i < stroke * 5; i++) {
            cocos2d::ccDrawCircle(info.lastPos, radius + (i * 0.1f), 360, segments, false);
        }
    }
}

#if defined(GEODE_IS_WINDOWS) 
class $modify(ShowTouchesCCEGLView, cocos2d::CCEGLView) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Show Touches");
        hack.addHookPtr(self.getHook("cocos2d::CCEGLView::swapBuffers").unwrap());
    }

    void swapBuffers() override {
        renderTouches();
        CCEGLView::swapBuffers();
    }
};
#endif

#if defined(GEODE_IS_ANDROID) || defined(GEODE_IS_MACOS)
class $modify(ShowTouchesCCDirector, cocos2d::CCDirector) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Show Touches");
        hack.addHookPtr(self.getHook("cocos2d::CCDirector::drawScene").unwrap());
    }

    void drawScene() {
        CCDirector::drawScene();
        renderTouches();
    }
};
#endif

class $modify(ShowTouchesCCTouchDispatcher, cocos2d::CCTouchDispatcher) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Show Touches");
        hack.addHookPtr(self.getHook("cocos2d::CCTouchDispatcher::touches").unwrap());
    }

    void touches(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent, unsigned int uIndex) {
        auto& config = Config::get();
        if (config.get<bool>("cosmetic.show_touches", false)) {
            if (auto* touch = static_cast<cocos2d::CCTouch*>(pTouches->anyObject())) {
                switch (uIndex) {
                    case cocos2d::CCTOUCHBEGAN: {
                        g_touches[touch] = { touch->getLocation(), 0.f, false };
                    } break;
                    case cocos2d::CCTOUCHENDED:
                    case cocos2d::CCTOUCHCANCELLED: {
                        if (g_touches.contains(touch)) {
                            g_touches[touch].released = true;
                        }
                    } break;
                    default: break;
                }
            }
        }
        CCTouchDispatcher::touches(pTouches, pEvent, uIndex);
    }
};

$execute {
    auto& hack = GDH::Gui::get().getWindow("Cosmetic").findHackByName("Show Touches");

    hack.setCustomWindowImGui([
        fillKey = hack.formatAdditionalSetting("fill"),
        scaleKey = hack.formatAdditionalSetting("scale"),
        strokeKey = hack.formatAdditionalSetting("stroke"),
        opacityKey = hack.formatAdditionalSetting("opacity"),
        fadeSpeedKey = hack.formatAdditionalSetting("fade_speed"),
        colorKey = hack.formatAdditionalSetting("color")
    ]{
        ImGuiWidgetConfig::Checkbox("Fill", fillKey, true);
        ImGuiWidgetConfig::DragFloat("##ShowTouches_Scale", scaleKey, 0.01f, 0.01f, 5.f, 0.5f, "Scale: %.2f");
        ImGuiWidgetConfig::DragInt("##ShowTouches_Stroke", strokeKey, 1, 0, 10, 0, "Stroke: %d");
        ImGuiWidgetConfig::DragFloat("##ShowTouches_Opacity", opacityKey, 0.01f, 0.01f, 1.f, 0.5f, "Opacity: %.2f");
        ImGuiWidgetConfig::DragFloat("##ShowTouches_FadeSpeed", fadeSpeedKey, 0.1f, 1.f, 20.f, 6.f, "Fade Speed: %.1f");
        ImGuiWidgetConfig::ColorEdit4Hex("Color", colorKey, "#FFFFFFFF");
    });

    hack.setCustomWindowCocos([
        fillKey = hack.formatAdditionalSetting("fill"),
        scaleKey = hack.formatAdditionalSetting("scale"),
        strokeKey = hack.formatAdditionalSetting("stroke"),
        opacityKey = hack.formatAdditionalSetting("opacity"),
        fadeSpeedKey = hack.formatAdditionalSetting("fade_speed"),
        colorKey = hack.formatAdditionalSetting("color")
    ](cocos2d::CCNode* popupNode){
        auto* popup = static_cast<HackSettingsPopup*>(popupNode);
        popup->addConfigToggle("Fill", fillKey, true);
        popup->addConfigFloatInput("Scale (0.01 - 5.0)", scaleKey, 0.01f, 5.f, 0.5f);
        popup->addConfigIntInput("Stroke (0 - 10)", strokeKey, 0, 10, 0);
        popup->addConfigFloatInput("Opacity (0.01 - 1.0)", opacityKey, 0.01f, 1.f, 0.5f);
        popup->addConfigFloatInput("Fade Speed (1 - 20)", fadeSpeedKey, 1.f, 20.f, 6.f);
        popup->addConfigColor4Hex("Color", colorKey, "#FFFFFFFF");
    });
}