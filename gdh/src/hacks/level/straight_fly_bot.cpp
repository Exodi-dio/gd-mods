#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include <imgui-cocos.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Level", "Straight Fly Bot", "", true);

static float m_startY1 = 0.0f;
static float m_startY2 = 0.0f;

class $modify(StraightFlyGJBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        ~Fields() {
            m_startY1 = 0.f;
            m_startY2 = 0.f;
        }
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Straight Fly Bot");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::processQueuedButtons").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::teleportPlayer").unwrap());

        hack.setCustomWindowImGui([] {
            ImGuiWidgetConfig::DragFloat("##straightfly_bot::accuracy", "level.straightfly_bot::accuracy", 1.f, 0.f, 100.f, 40.f, "Y Accuracy: %.1f");
            ImGuiWidgetConfig::Checkbox("First Player", "level.straightfly_bot::p1", true);
            ImGuiWidgetConfig::Checkbox("Second Player", "level.straightfly_bot::p2", false);
        });

        hack.setCustomWindowCocos([](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigFloatInput("Accuracy", "level.straightfly_bot::accuracy", 0.f, 100.f, 40.f);
            popup->addConfigToggle("First Player", "level.straightfly_bot::p1", true);
            popup->addConfigToggle("Second Player", "level.straightfly_bot::p2", false);
        });

        hack.setHandler([](bool enabled) {
            m_startY1 = 0.f;
            m_startY2 = 0.f;
        });
    }

    bool init() {
        if (!GJBaseGameLayer::init()) return false;
        m_fields.self();
        return true;
    }

    void teleportPlayer(TeleportPortalObject* object, PlayerObject* player) {
        GJBaseGameLayer::teleportPlayer(object, player);

        if (!player || !object || !object->m_orangePortal) return;

        if (player == m_player1) {
            m_startY1 = object->m_orangePortal->getPositionY();
        } else if (player == m_player2) {
            m_startY2 = object->m_orangePortal->getPositionY();
        }
    }

    void processStraightFlyForPlayer(PlayerObject* player, float& startY, bool isPlayer2) {
        if (!player && !player->m_isShip) return;

        auto& config = Config::get();
        float accuracy = config.get<float>("level.straightfly_bot::accuracy", 40.0f);

        if (startY == 0.0f) {
            startY = player->m_position.y;
        }

        float y = player->m_position.y;
        double accel = player->m_yVelocity;
        bool holding = player->m_jumpBuffered;

        if (player->m_isUpsideDown) {
            float delta_y = y - startY;
            y = startY - delta_y;
            accel *= -1;
        }

        if (accel < 0 && y < startY - accel - accuracy / 100.f && !holding) {
            this->handleButton(true, 1, !isPlayer2);
        }
        else if (accel > 0 && y > startY - accel + accuracy / 100.f && holding) {
            this->handleButton(false, 1, !isPlayer2);
        }
    }

    void processQueuedButtons(float dt, bool clearInputQueue) {
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);

        if (m_levelEndAnimationStarted) return;

        auto& config = Config::get();
        if (config.get<bool>("level.straightfly_bot::p1", true)) {
            processStraightFlyForPlayer(m_player1, m_startY1, false);
        }
        if (config.get<bool>("level.straightfly_bot::p2", false)) {
            processStraightFlyForPlayer(m_player2, m_startY2, true);
        }
    }
};