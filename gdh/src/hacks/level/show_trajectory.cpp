#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/EffectGameObject.hpp>
#include <Geode/modify/HardStreak.hpp>

#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"
#include "../../core/utils.hpp"
#include "../../core/checkpointData.hpp"
#include "imgui.h"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Show Trajectory", "Draws the predicted paths of a player for continuous button holds/release", true);

namespace TrajectoryManager {
    static PlayerObject* dummy_jump = nullptr;
    static PlayerObject* dummy_normal = nullptr;

    static bool is_simulating = false;
    static bool simulation_dead = false;

    static float dt_step = 0.0f;
    static cocos2d::CCDrawNode* draw_node = nullptr;

    static void createDrawNode(cocos2d::CCLayer* layer, cocos2d::CCNode* debugNode) {
        auto* parent = debugNode ? debugNode->getParent() : layer;
        if (!parent) return;

        draw_node = cocos2d::CCDrawNode::create();
        draw_node->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        draw_node->setZOrder(9999);
        draw_node->setID("trajectory-node"_spr);
        draw_node->m_bUseArea = false;
        parent->addChild(draw_node);
    }

    static PlayerObject* createVirtualPlayer(PlayLayer* layer) {
        if (!layer) return nullptr;
        auto* dummy = PlayerObject::create(1, 1, layer, layer, true);
        if (dummy) {
            dummy->setPosition({0, 105});
            dummy->setVisible(false);
            dummy->setID("trajectory-fake-player"_spr);
            if (layer->m_objectLayer) {
                layer->m_objectLayer->addChild(dummy);
            }
        }
        return dummy;
    }

    static void clearCollisionLogs(PlayerObject* player) {
        if (!player) return;
        if (player->m_collisionLogTop) player->m_collisionLogTop->removeAllObjects();
        if (player->m_collisionLogBottom) player->m_collisionLogBottom->removeAllObjects();
        if (player->m_collisionLogLeft) player->m_collisionLogLeft->removeAllObjects();
        if (player->m_collisionLogRight) player->m_collisionLogRight->removeAllObjects();
    }

    static void renderHitboxOutline(const cocos2d::CCRect& rect, const cocos2d::ccColor4F& border) {
        if (!draw_node) return;
        cocos2d::CCPoint vertices[4] = {
            {rect.getMinX(), rect.getMinY()},
            {rect.getMinX(), rect.getMaxY()},
            {rect.getMaxX(), rect.getMaxY()},
            {rect.getMaxX(), rect.getMinY()}
        };
        draw_node->drawPolygon(vertices, 4, {0.f, 0.f, 0.f, 0.f}, 0.25f, border);
    }

    static void simulatePath(PlayLayer* layer, PlayerObject* simPlayer, PlayerObject* sourcePlayer, bool isJumpInput) {
        if (!layer || !simPlayer || !sourcePlayer) return;

        auto& config = Config::get();
        bool performanceMode = config.get<bool>("level.show_trajectory::performance", true);
        bool disableCollisions = config.get<bool>("level.show_trajectory::no_collisions", false);
        int stepMultiplier = config.get<int>("level.show_trajectory::step_multiplier", 1);
        if (stepMultiplier < 1) stepMultiplier = 1;

        simPlayer->copyAttributes(sourcePlayer);
        GDH::PracticeFix::copyPlayerState(sourcePlayer, simPlayer);

        simPlayer->m_playEffects = false;
        simPlayer->m_hasNoEffects = true;
        simPlayer->m_maybeReducedEffects = performanceMode;

        simPlayer->m_gravityMod = sourcePlayer->m_gravityMod;
        simPlayer->m_isSideways = sourcePlayer->m_isSideways;

        simPlayer->m_isDashing = sourcePlayer->m_isDashing;
        simPlayer->m_dashX = sourcePlayer->m_dashX;
        simPlayer->m_dashY = sourcePlayer->m_dashY;
        simPlayer->m_dashAngle = sourcePlayer->m_dashAngle;
        simPlayer->m_isOnGround = sourcePlayer->m_isOnGround;

        if (isJumpInput) {
            if (sourcePlayer->m_isSpider) {
                if (sourcePlayer->m_isOnGround || sourcePlayer->m_jumpBuffered) {
                    simPlayer->spiderTestJump(false);
                }
            } else {
                simPlayer->pushButton(PlayerButton::Jump);
            }
        } else {
            simPlayer->releaseButton(PlayerButton::Jump);
            simPlayer->m_jumpBuffered = false;
        }

        simPlayer->setVisible(false);
        simulation_dead = false;

        int iterations = config.get<int>("level.show_trajectory::iterations", 150);
        int effectiveIterations = iterations / stepMultiplier;
        if (effectiveIterations < 1) effectiveIterations = 1;

        auto jumpColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_trajectory::jumpColor", "#E900FFFF"));
        auto normalColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_trajectory::normalColor", "#00FFFFFF"));

        auto playerInnerColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::playerInnerColor", "#003FFFFF"));
        auto playerColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::playerColor", "#FF0000FF"));

        cocos2d::ccColor4F lineColor = isJumpInput ? jumpColor : normalColor;

        if (config.get<bool>("invisible.tps", false)) {
            dt_step = 1.f / config.get<float>("invisible.tps::value", 240.f) * 60.f;
        }

        float effectiveDt = dt_step * static_cast<float>(stepMultiplier);

        for (int i = 0; i < effectiveIterations; ++i) {
            cocos2d::CCPoint currentPos = simPlayer->getPosition();
            clearCollisionLogs(simPlayer);

            if (simulation_dead) break;

            simPlayer->update(effectiveDt);

            if (!disableCollisions && layer->checkCollisions(simPlayer, effectiveDt, false) == 1) {
                simulation_dead = true;
            }

            draw_node->drawSegment(currentPos, simPlayer->getPosition(), 0.65f, lineColor);
        }

        renderHitboxOutline(simPlayer->getObjectRect(), playerColor);
        renderHitboxOutline(simPlayer->getObjectRect(0.25f, 0.25f), playerInnerColor);
    }

    static void runTrajectorySimulation(PlayLayer* layer) {
        if (!layer || !draw_node) return;

        auto& config = Config::get();
        if (!config.get<bool>("level.show_trajectory", false)) {
            draw_node->clear();
            return;
        }

        if (!dummy_jump || !dummy_normal) return;

        is_simulating = true;
        draw_node->clear();

        if (layer->m_player1) {
            simulatePath(layer, dummy_jump, layer->m_player1, true);
            simulatePath(layer, dummy_normal, layer->m_player1, false);
        }

        if (layer->m_gameState.m_isDualMode && layer->m_player2) {
            simulatePath(layer, dummy_jump, layer->m_player2, true);
            simulatePath(layer, dummy_normal, layer->m_player2, false);
        }
        is_simulating = false;
    }

    static void reset() {
        dummy_jump = nullptr;
        dummy_normal = nullptr;
        draw_node = nullptr;
        is_simulating = false;
        simulation_dead = false;
    }
}

class $modify(ShowTrajectoryPlayLayer, PlayLayer) {
    struct Fields {
        ~Fields() {
            TrajectoryManager::reset();
        }
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");       

        (void) self.setHookPriority("PlayLayer::destroyPlayer", -50); 
        hack.addHookPtr(self.getHook("PlayLayer::destroyPlayer").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::playEndAnimationToPos").unwrap());
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        m_fields.self();
        TrajectoryManager::dummy_jump = TrajectoryManager::createVirtualPlayer(this);
        TrajectoryManager::dummy_normal = TrajectoryManager::createVirtualPlayer(this);
        TrajectoryManager::createDrawNode(this, m_debugDrawNode);

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (object != m_anticheatSpike && TrajectoryManager::is_simulating) {
            if (player == TrajectoryManager::dummy_jump || player == TrajectoryManager::dummy_normal) {
                TrajectoryManager::simulation_dead = true;
                return;
            }
        }
        PlayLayer::destroyPlayer(player, object);
    }

    void playEndAnimationToPos(cocos2d::CCPoint position) {
        if (TrajectoryManager::is_simulating) return;
        PlayLayer::playEndAnimationToPos(position);
    }
};

class $modify(ShowTrajectoryBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");       

        hack.addHookPtr(self.getHook("GJBaseGameLayer::canBeActivatedByPlayer").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::collisionCheckObjects").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::flipGravity").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::playerTouchedRing").unwrap());
    }

    void updateCamera(float dt) {
        if (auto pl = PlayLayer::get()) 
            TrajectoryManager::runTrajectorySimulation(pl);

        GJBaseGameLayer::updateCamera(dt);
    }

    bool canBeActivatedByPlayer(PlayerObject* player, EffectGameObject* object) {
        if (TrajectoryManager::is_simulating) return false;
        return GJBaseGameLayer::canBeActivatedByPlayer(player, object);
    }

    void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* objects, int objectCount, float dt) {
        if (!TrajectoryManager::is_simulating) {
            GJBaseGameLayer::collisionCheckObjects(player, objects, objectCount, dt);
            return;
        }

        if (Config::get().get<bool>("level.show_trajectory::no_collisions", false)) {
            return;
        }

        if (!player || !objects || objectCount <= 0) return;

        auto addCollisionObject = [](GameObject* obj, gd::vector<GameObject*>& vec, int& count, int& index) {
            if (count < index) {
                vec.at(count) = obj;
            } else {
                vec.push_back(obj);
                index++;
            }
            count++;
        };

        const CCRect playerRect = player->getObjectRect();

        for (int i = 0; i < objectCount; ++i) {
            GameObject* object = objects->at(i);

            if (!object || object->m_isGroupDisabled || object->m_isDisabled || object == m_anticheatSpike) {
                continue;
            }

            const auto type = object->m_objectType;

            if (type == GameObjectType::Solid || type == GameObjectType::Breakable) {
                addCollisionObject(object, m_solidCollisionObjects, m_solidCollisionObjectsCount, m_solidCollisionObjectsIndex);
                continue;
            }

            if (type == GameObjectType::Hazard || type == GameObjectType::AnimatedHazard) {
                addCollisionObject(object, m_hazardCollisionObjects, m_hazardCollisionObjectsCount, m_hazardCollisionObjectsIndex);
                continue;
            }

            if (type == GameObjectType::Slope) {
                const cocos2d::CCRect slopeRect = object->getObjectRect(2.0f, 2.0f);
                if (!playerRect.intersectsRect(slopeRect)) continue;

                const bool needsFix = !m_levelSettings->m_fixRadiusCollision || object->m_objectRadius <= 0.0;
                if (object->m_shouldUseOuterOb && needsFix) {
                    OBB2D* box = object->getOrientedBox();
                    player->updateOrientedBox();

                    OBB2D* playerBox = static_cast<GameObject*>(player)->getOrientedBox();
                    if (!box->overlaps1Way(playerBox)) continue;
                }

                if (!player->m_isSideways) {
                    player->collidedWithSlopeInternal(dt, object, false);
                } else {
                    player->handleRotatedCollisionInternal(dt, object, {0.0f, 0.0f, 0.0f, 0.0f}, false, false, true);
                }
            }
        }
    }

    void flipGravity(PlayerObject* object, bool flip, bool noEffects) {
        if (TrajectoryManager::is_simulating) return;
        GJBaseGameLayer::flipGravity(object, flip, noEffects);
    }

    void playerTouchedRing(PlayerObject* player, RingObject* object) {
        if (TrajectoryManager::is_simulating) return;
        GJBaseGameLayer::playerTouchedRing(player, object);
    }

    void destroyObject(GameObject *object) {
        if (TrajectoryManager::is_simulating) {
            if (object->m_objectType == GameObjectType::Breakable) return;
        }
        GJBaseGameLayer::destroyObject(object);
    }
};

class $modify(ShowTrajectoryPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");       

        hack.addHookPtr(self.getHook("PlayerObject::update").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::stopDashing").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::playSpiderDashEffect").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::ringJump").unwrap());
        hack.addHookPtr(self.getHook("PlayerObject::incrementJumps").unwrap());
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!TrajectoryManager::is_simulating) {
            if (auto pl = PlayLayer::get()) {
                float timeWarp = pl->m_gameState.m_timeWarp;
                TrajectoryManager::dt_step = (timeWarp > 0.001f) ? (dt / timeWarp) : dt;
            }
        }
    }

    void stopDashing() {
        if (!TrajectoryManager::is_simulating)
            return PlayerObject::stopDashing();

        bool orig = m_playEffects;

        m_playEffects = false;
        PlayerObject::stopDashing();
        m_playEffects = orig;
    }

    void playSpiderDashEffect(cocos2d::CCPoint from, cocos2d::CCPoint to) {
        if (TrajectoryManager::is_simulating) return;
        PlayerObject::playSpiderDashEffect(from, to);
    }

    void ringJump(RingObject* object, bool skipCheck) {
        if (TrajectoryManager::is_simulating) return;
        PlayerObject::ringJump(object, skipCheck);
    }

    void incrementJumps() {
        if (TrajectoryManager::is_simulating) return;
        PlayerObject::incrementJumps();
    }
};

class $modify(ShowTrajectoryEffectGameObject, EffectGameObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");       

        hack.addHookPtr(self.getHook("EffectGameObject::triggerObject").unwrap());
    }

    void triggerObject(GJBaseGameLayer* layer, int uniqueID, const gd::vector<int>* remapKeys) {
        if (TrajectoryManager::is_simulating) return;
        EffectGameObject::triggerObject(layer, uniqueID, remapKeys);
    }
};

class $modify(ShowTrajectoryGameObject, GameObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");       

        hack.addHookPtr(self.getHook("GameObject::playShineEffect").unwrap());
    }

    void playShineEffect() {
        if (TrajectoryManager::is_simulating) return;
        GameObject::playShineEffect();
    }
};

class $modify(ShowTrajectoryHardStreak, HardStreak) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");       

        hack.addHookPtr(self.getHook("HardStreak::addPoint").unwrap());
    }

    void addPoint(cocos2d::CCPoint point) {
        if (TrajectoryManager::is_simulating) return;
        HardStreak::addPoint(point);
    }
};

$execute {
    auto& config = Config::get();
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Level").findHackByName("Show Trajectory");

    hack.setCustomWindowImGui([
        iterationsKey = hack.formatAdditionalSetting("iterations"),
        cJumpKey = hack.formatAdditionalSetting("jumpColor"),
        cNormalKey = hack.formatAdditionalSetting("normalColor"),
        cPlayerKey = hack.formatAdditionalSetting("playerColor"),
        cPlayerInnerKey = hack.formatAdditionalSetting("playerInnerColor"),
        performanceKey = hack.formatAdditionalSetting("performance"),
        noCollisionsKey = hack.formatAdditionalSetting("no_collisions"),
        stepMultiplierKey = hack.formatAdditionalSetting("step_multiplier")
    ]{
        ImGuiWidgetConfig::DragInt("##Trajectory_Iterations", iterationsKey, 1, 10, 2000, 150, "Iterations: %d");
        ImGuiWidgetConfig::DragInt("##Trajectory_StepMultiplier", stepMultiplierKey, 1, 1, 10, 1, "Inaccuracy Multiplier: %dx");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGuiWidgetConfig::Checkbox("Performance Mode", performanceKey, true);
        ImGuiWidgetConfig::Checkbox("Disable Collisions", noCollisionsKey, false);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Line Colors:");
        ImGuiWidgetConfig::ColorEdit4Hex("Jump Trajectory", cJumpKey, "#E900FFFF");
        ImGuiWidgetConfig::ColorEdit4Hex("Normal Trajectory", cNormalKey, "#00FFFFFF");
    });

    hack.setCustomWindowCocos([
        iterationsKey = hack.formatAdditionalSetting("iterations"),
        cJumpKey = hack.formatAdditionalSetting("jumpColor"),
        cNormalKey = hack.formatAdditionalSetting("normalColor"),
        cPlayerKey = hack.formatAdditionalSetting("playerColor"),
        cPlayerInnerKey = hack.formatAdditionalSetting("playerInnerColor"),
        performanceKey = hack.formatAdditionalSetting("performance"),
        noCollisionsKey = hack.formatAdditionalSetting("no_collisions"),
        stepMultiplierKey = hack.formatAdditionalSetting("step_multiplier")
    ](cocos2d::CCNode* popupNode){
        auto* popup = static_cast<HackSettingsPopup*>(popupNode);
        popup->addConfigIntInput("Iterations", iterationsKey, 10, 2000, 150);
        popup->addConfigIntInput("Inaccuracy Multiplier (1 - 10)", stepMultiplierKey, 1, 10, 1);

        popup->addConfigToggle("Performance Mode", performanceKey, true);
        popup->addConfigToggle("Disable Collisions", noCollisionsKey, false);

        popup->addSeparator();

        popup->addConfigColor4Hex("Jump Trajectory", cJumpKey, "#E900FFFF");
        popup->addConfigColor4Hex("Normal Trajectory", cNormalKey, "#00FFFFFF");
    });
}