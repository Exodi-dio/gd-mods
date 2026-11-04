#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/imgui/widgetH.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"
#include "../../core/utils.hpp"
#include "../../core/ringBuffer.hpp"
#include "imgui.h"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Show Hitboxes", "Visualises level hitboxes", true);

static cocos2d::CCDrawNode* createDrawNode(cocos2d::CCLayer* layer, cocos2d::CCNode* debugNode, const char* id) {
    auto* parent = debugNode ? debugNode->getParent() : layer;
    if (!parent) return nullptr;

    auto* drawNode = cocos2d::CCDrawNode::create();
    drawNode->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
    drawNode->setZOrder(9999);
    drawNode->setID(id);
    drawNode->m_bUseArea = false;
    parent->addChild(drawNode);

    return drawNode;
}

static void drawRect(cocos2d::CCDrawNode* node, cocos2d::CCRect const& rect,
                     cocos2d::ccColor4F const& color, float borderWidth, cocos2d::ccColor4F const& borderColor) 
{
    cocos2d::CCPoint vertices[4] = {
        {rect.getMinX(), rect.getMinY()},
        {rect.getMinX(), rect.getMaxY()},
        {rect.getMaxX(), rect.getMaxY()},
        {rect.getMaxX(), rect.getMinY()}
    };
    node->drawPolygon(vertices, 4, color, borderWidth, borderColor);
}

static void drawOrientedOrRect(GJBaseGameLayer* layer, cocos2d::CCDrawNode* drawNode, GameObject* obj,
                               cocos2d::ccColor4F const& fill, float bw, cocos2d::ccColor4F const& border) 
{
    bool dirty = obj->m_isObjectRectDirty;
    bool boxOff = obj->m_boxOffsetCalculated;

    auto rect = obj->getObjectRect();

    int rotation = static_cast<int>(obj->getRotation());
    bool isRotated = (rotation % 90) != 0;

    if (isRotated && (layer->m_isEditor || obj->m_shouldUseOuterOb)) {
        obj->calculateOrientedBox();
        
        if (auto* ob = obj->getOrientedBox()) {
            drawNode->drawPolygon(ob->m_corners.data(), 4, fill, bw, border);
        } else {
            drawRect(drawNode, rect, fill, bw, border);
        }
    } else {
        drawRect(drawNode, rect, fill, bw, border);
    }

    obj->m_isObjectRectDirty = dirty;
    obj->m_boxOffsetCalculated = boxOff;
}

static RingBuffer<cocos2d::CCRect> playerTrail1, playerTrail2;
static RingBuffer<cocos2d::CCRect> playerInnerTrail1, playerInnerTrail2;
static size_t lastMaxLength = 0;

static void drawSingleObject(GJBaseGameLayer* layer, cocos2d::CCDrawNode* drawNode, GameObject* obj,
                             float size, float fillAlpha,
                             const cocos2d::ccColor4F& triggersColor, const cocos2d::ccColor4F& otherColor,
                             const cocos2d::ccColor4F& passableColor, const cocos2d::ccColor4F& solidColor,
                             const cocos2d::ccColor4F& dangerColor) 
{
    if (!obj) return;
    if (obj->m_objectType == GameObjectType::Decoration || !obj->m_isActivated || obj->m_isGroupDisabled)
        return;

    auto getFillColor = [fillAlpha](cocos2d::ccColor4F color) -> cocos2d::ccColor4F {
        return { color.r, color.g, color.b, fillAlpha };
    };

    switch (obj->m_objectType) {
        default: {
            if (obj == layer->m_player1 || obj == layer->m_player2) break;
            bool isSpeedPortal = obj->m_objectID == 200 || obj->m_objectID == 201 || obj->m_objectID == 202 || obj->m_objectID == 203 || obj->m_objectID == 1334;
            if (obj->m_objectType == GameObjectType::Modifier && !isSpeedPortal) {
                if (static_cast<EffectGameObject*>(obj)->m_isTouchTriggered)
                    drawOrientedOrRect(layer, drawNode, obj, getFillColor(triggersColor), size, triggersColor);
                break;
            }
            drawOrientedOrRect(layer, drawNode, obj, getFillColor(otherColor), size, otherColor);
            break;
        }
        case GameObjectType::Solid: {
            bool p = obj->m_isPassable;
            drawRect(drawNode, obj->getObjectRect(), getFillColor(p ? passableColor : solidColor), size, p ? passableColor : solidColor);
            break;
        }
        case GameObjectType::Slope: {
            auto rect = obj->getObjectRect();
            cocos2d::CCPoint verts[3] = {
                {rect.getMinX(), rect.getMinY()},
                {rect.getMinX(), rect.getMaxY()},
                {rect.getMaxX(), rect.getMinY()},
            };

            cocos2d::CCPoint const tr{rect.getMaxX(), rect.getMaxY()};
            switch (obj->m_slopeDirection) {
                case 0: case 7: verts[1] = tr; break;
                case 1: case 5: verts[0] = tr; break;
                case 3: case 6: verts[2] = tr; break;
                default: break;
            }
            
            bool const p = obj->m_isPassable;
            drawNode->drawPolygon(verts, 3, getFillColor(p ? passableColor : solidColor), size, p ? passableColor : solidColor);
            
            break;
        }
        case GameObjectType::AnimatedHazard:
        case GameObjectType::Hazard: {
            if (obj == layer->m_anticheatSpike) break;
            float const radius = std::max(obj->m_scaleX, obj->m_scaleY) * obj->m_objectRadius;

            if (radius > 0)
                drawNode->drawCircle(obj->getPosition(), radius, getFillColor(dangerColor), size, dangerColor, 16);
            else
                drawOrientedOrRect(layer, drawNode, obj, getFillColor(dangerColor), size, dangerColor);

            break;
        }
        case GameObjectType::CollisionObject: break;
    }
}

void updateHitboxes(GJBaseGameLayer* layer, cocos2d::CCDrawNode* drawNode, GameObject* deathObject = nullptr) {
    if (!drawNode) return;
    drawNode->clear();

    auto& config = Config::get();
    if (!config.get<bool>("level.show_hitboxes", false)) return;

    bool showPlayer = config.get<bool>("level.show_hitboxes::show_player", true);
    bool showOnDeath = config.get<bool>("level.show_hitboxes::show_on_death", false);
    bool showDeathObject = config.get<bool>("level.show_hitboxes::show_death_object", false);

    bool realDeath = layer->m_playerDied;
    bool collided = deathObject != nullptr;

    bool shouldDrawObjects = false;
    bool shouldDrawDeathOnly = false;

    if (layer->m_isEditor) {
        shouldDrawObjects = true;
    } else {
        if (showOnDeath && showDeathObject) {
            if (realDeath || collided) {
                shouldDrawObjects = true;
            }
        } else if (showOnDeath) {
            if (realDeath) {
                shouldDrawObjects = true;
            }
        } else if (showDeathObject) {
            if (realDeath || collided) {
                shouldDrawDeathOnly = true;
            }
        } else {
            shouldDrawObjects = true;
        }
    }

    if (!shouldDrawObjects && !shouldDrawDeathOnly && !showPlayer) return;

    auto triggersColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::triggersColor", "#FF00E6FF"));
    auto otherColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::otherColor", "#00FF00FF"));
    auto passableColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::passableColor", "#00FFFFFF"));
    auto solidColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::solidColor", "#003FFFFF"));
    auto dangerColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::dangerColor", "#FF0000FF"));

    auto playerRotateColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::playerRotateColor", "#800000FF"));
    auto playerInnerColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::playerInnerColor", "#003FFFFF"));
    auto playerColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::playerColor", "#FF0000FF"));

    auto trailColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::trailColor", "#FFFF00FF"));
    auto innerTrailColor = GDH::Utils::hexToColor4F(config.get<std::string>("level.show_hitboxes::innerTrailColor", "#FF0000FF"));

    auto size = config.get<float>("level.show_hitboxes::size", 0.25f);
    auto fillAlpha = config.get<float>("level.show_hitboxes::fill_alpha", 0.2f);

    auto getFillColor = [fillAlpha](cocos2d::ccColor4F color) -> cocos2d::ccColor4F {
        return { color.r, color.g, color.b, fillAlpha };
    };

    if (shouldDrawDeathOnly && deathObject) {
        drawSingleObject(layer, drawNode, deathObject, size, fillAlpha, triggersColor, otherColor, passableColor, solidColor, dangerColor);
    } else if (shouldDrawObjects) {
        if (!layer->m_sections.empty()) {
            int rightBound = std::min(layer->m_rightSectionIndex, static_cast<int>(layer->m_sections.size()) - 1);
            for (int i = layer->m_leftSectionIndex; i <= rightBound; ++i) {
                auto* leftSection = layer->m_sections[i];
                if (!leftSection) continue;
                int topBound = std::min(layer->m_topSectionIndex, static_cast<int>(leftSection->size()) - 1);
                auto* sizeRow = layer->m_sectionSizes[i];
                for (int j = layer->m_bottomSectionIndex; j <= topBound; ++j) {
                    auto* section = leftSection->at(j);
                    if (!section) continue;
                    int sectionSize = sizeRow->at(j);
                    auto* data = section->data();
                    for (int k = 0; k < sectionSize; ++k) {
                        drawSingleObject(layer, drawNode, data[k], size, fillAlpha, triggersColor, otherColor, passableColor, solidColor, dangerColor);
                    }
                }
            }
        }
    }

    bool shouldDrawPlayer = showPlayer;

    if (!layer->m_isEditor) {
        if (showOnDeath || showDeathObject) {
            shouldDrawPlayer = showPlayer && realDeath;
        }
    }

    if (shouldDrawPlayer) {
        auto drawPlayer = [&](PlayerObject* player) {
            if (!player) return;

            if (player->m_isBall) {
                drawRect(drawNode, player->getObjectRect(), getFillColor(playerColor), size, playerColor);
            } else {
                player->updateOrientedBox();
                if (auto* ob = player->getOrientedBox()) {
                    drawNode->drawPolygon(ob->m_corners.data(), 4, getFillColor(playerRotateColor), size, playerRotateColor);
                }
                drawRect(drawNode, player->getObjectRect(), getFillColor(playerColor), size, playerColor);
            }
            drawRect(drawNode, player->getObjectRect(0.3f, 0.3f), getFillColor(playerInnerColor), size, playerInnerColor);
        };

        drawPlayer(layer->m_player1);
        if (layer->m_gameState.m_isDualMode) drawPlayer(layer->m_player2);

        if (config.get<bool>("level.show_hitboxes::draw_trail", false)) {
            auto renderTrail = [drawNode, size](const RingBuffer<cocos2d::CCRect>& trail, const cocos2d::ccColor4F& color) {
                trail.for_each([drawNode, size, &color](const auto& rect) {
                    drawRect(drawNode, rect, {0.f, 0.f, 0.f, 0.f}, size, color);
                });
            };

            renderTrail(playerTrail1, trailColor);
            renderTrail(playerInnerTrail1, innerTrailColor);

            renderTrail(playerTrail2, trailColor);
            renderTrail(playerInnerTrail2, innerTrailColor);
        }
    }
}

class $modify(ShowHitboxesGJBaseGameLayer, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {        
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        
        auto& config = Config::get();
        if (config.get<bool>("level.show_hitboxes::draw_trail", false)) {
            auto maxLength = static_cast<size_t>(config.get<int>("level.show_hitboxes::trail_length", 240));
            
            if (lastMaxLength != maxLength) {
                playerTrail1.init(maxLength);
                playerTrail2.init(maxLength);
                playerInnerTrail1.init(maxLength);
                playerInnerTrail2.init(maxLength);
                lastMaxLength = maxLength;
            }

            if (!m_playerDied) {
                if (m_player1) {
                    playerTrail1.push(m_player1->getObjectRect());
                    playerInnerTrail1.push(m_player1->getObjectRect(0.3f, 0.3f));
                }
                if (m_gameState.m_isDualMode && m_player2) {
                    playerTrail2.push(m_player2->getObjectRect());
                    playerInnerTrail2.push(m_player2->getObjectRect(0.3f, 0.3f));
                }
            }
        }
    }
};

static GameObject* currentCheckingObject = nullptr;
class $modify(ShowHitboxesPlayerObject, PlayerObject) {
    bool collidedWithObjectInternal(float dt, GameObject* object, cocos2d::CCRect rect, bool skipCheck) {
        currentCheckingObject = object;
        bool result = PlayerObject::collidedWithObjectInternal(dt, object, rect, skipCheck);
        currentCheckingObject = nullptr;

        return result;
    }
};

class $modify(ShowHitboxesPlayLayer, PlayLayer) {
    struct Fields {
        cocos2d::CCDrawNode* m_drawNode = nullptr;
        GameObject* m_deathObject = nullptr;
    };

    static void onModify(auto& self) {
        (void) self.setHookPriority("PlayLayer::destroyPlayer", -40); 
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) override {
        if (m_anticheatSpike == object) {
            PlayLayer::destroyPlayer(player, object);
            return;
        }

        auto fields = m_fields.self();
        if (object != nullptr) {
            fields->m_deathObject = object;
        }
        else if (currentCheckingObject != nullptr) {
            fields->m_deathObject = currentCheckingObject;
        }
        
        PlayLayer::destroyPlayer(player, object);
    }

    void createObjectsFromSetupFinished() {
        PlayLayer::createObjectsFromSetupFinished();
        m_fields->m_drawNode = createDrawNode(this, m_debugDrawNode, "hitboxes"_spr);
    }

    void updateVisibility(float dt) override {
        PlayLayer::updateVisibility(dt);

        auto fields = m_fields.self();
        updateHitboxes(this, m_fields->m_drawNode, fields->m_deathObject);
    }

    void resetLevel() override {
        playerTrail1.clear();
        playerTrail2.clear();
        playerInnerTrail1.clear();
        playerInnerTrail2.clear();

        auto fields = m_fields.self();
        fields->m_deathObject = nullptr;

        PlayLayer::resetLevel();
    }
};

class $modify(ShowHitboxesLevelEditorLayer, LevelEditorLayer) {
    struct Fields {
        cocos2d::CCDrawNode* m_drawNode = nullptr;
    };

    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;
        m_fields->m_drawNode = createDrawNode(this, m_debugDrawNode, "hitboxes-editor"_spr);
        return true;
    }

    void updateVisibility(float dt) override {
        LevelEditorLayer::updateVisibility(dt);
        updateHitboxes(this, m_fields->m_drawNode);
    }

    void onPlaytest() {
        playerTrail1.clear();
        playerTrail2.clear();
        playerInnerTrail1.clear();
        playerInnerTrail2.clear();

        LevelEditorLayer::onPlaytest();
    }
};

$execute {
    auto& config = Config::get();
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Level").findHackByName("Show Hitboxes");   

    hack.setCustomWindowImGui([
        showPlayerKey = hack.formatAdditionalSetting("show_player"),
        onDeathKey = hack.formatAdditionalSetting("show_on_death"),
        showDeathObjKey = hack.formatAdditionalSetting("show_death_object"),
        sizeKey = hack.formatAdditionalSetting("size"),
        fillAlphaKey = hack.formatAdditionalSetting("fill_alpha"),
        drawTrailKey = hack.formatAdditionalSetting("draw_trail"),
        trailLengthKey = hack.formatAdditionalSetting("trail_length"),

        cTriggersKey = hack.formatAdditionalSetting("triggersColor"),
        cOtherKey = hack.formatAdditionalSetting("otherColor"),
        cPassableKey = hack.formatAdditionalSetting("passableColor"),
        cSolidKey = hack.formatAdditionalSetting("solidColor"),
        cDangerKey = hack.formatAdditionalSetting("dangerColor"),
        cPlayerRotateKey = hack.formatAdditionalSetting("playerRotateColor"),
        cPlayerInnerKey = hack.formatAdditionalSetting("playerInnerColor"),
        cPlayerKey = hack.formatAdditionalSetting("playerColor"),
        cTrailKey = hack.formatAdditionalSetting("trailColor"),
        cInnerTrailKey = hack.formatAdditionalSetting("innerTrailColor")
    ]{
        ImGuiWidgetConfig::Checkbox("Show Player", showPlayerKey, true);
        if (ImGuiWidgetConfig::Checkbox("Show on Death", onDeathKey, false)) GDH::Gui::get().rescanActiveCheats();
        if (ImGuiWidgetConfig::Checkbox("Show Death Object", showDeathObjKey, false)) GDH::Gui::get().rescanActiveCheats();
        ImGuiWidgetConfig::DragFloat("##Hitbox_Size", sizeKey, 0.05f, 0, 20.f, 0.25f, "Size: %.2f");
        ImGuiWidgetConfig::DragFloat("##Hitbox_FillAlpha", fillAlphaKey, 0.01f, 0, 1.f, 0.2f, "Fill Alpha: %.2f");

        ImGuiH::SpaceSeparator();

        ImGuiWidgetConfig::Checkbox("Draw Trail", drawTrailKey, false);
        ImGuiWidgetConfig::DragInt("##Trail_Length", trailLengthKey, 1, 0, 1000, 240, "Trail Length: %d");

        ImGuiH::SpaceSeparator();
        ImGui::Text("Hitbox Colors:");

        ImGuiWidgetConfig::ColorEdit4Hex("Triggers", cTriggersKey, "#FF00E6FF");
        ImGuiWidgetConfig::ColorEdit4Hex("Other Objects", cOtherKey, "#00FF00FF");
        ImGuiWidgetConfig::ColorEdit4Hex("Passable Blocks", cPassableKey, "#00FFFFFF");
        ImGuiWidgetConfig::ColorEdit4Hex("Solid (Blocks)", cSolidKey, "#003FFFFF");
        ImGuiWidgetConfig::ColorEdit4Hex("Danger (Spikes)", cDangerKey, "#FF0000FF");
        
        ImGuiH::SpaceSeparator();
        ImGui::Text("Player:");
        ImGuiWidgetConfig::ColorEdit4Hex("Player Rotated Box", cPlayerRotateKey, "#800000FF");
        ImGuiWidgetConfig::ColorEdit4Hex("Player Inner Box", cPlayerInnerKey, "#003FFFFF");
        ImGuiWidgetConfig::ColorEdit4Hex("Player Box", cPlayerKey, "#FF0000FF");

        ImGuiWidgetConfig::ColorEdit4Hex("Trail Color", cTrailKey, "#FFFF00FF");
        ImGuiWidgetConfig::ColorEdit4Hex("Inner Trail Color", cInnerTrailKey, "#FF0000FF");
    });

    hack.setCustomWindowCocos([
        showPlayerKey = hack.formatAdditionalSetting("show_player"),
        onDeathKey = hack.formatAdditionalSetting("show_on_death"),
        showDeathObjKey = hack.formatAdditionalSetting("show_death_object"),
        sizeKey = hack.formatAdditionalSetting("size"),
        fillAlphaKey = hack.formatAdditionalSetting("fill_alpha"),
        drawTrailKey = hack.formatAdditionalSetting("draw_trail"),
        trailLengthKey = hack.formatAdditionalSetting("trail_length"),

        cTriggersKey = hack.formatAdditionalSetting("triggersColor"),
        cOtherKey = hack.formatAdditionalSetting("otherColor"),
        cPassableKey = hack.formatAdditionalSetting("passableColor"),
        cSolidKey = hack.formatAdditionalSetting("solidColor"),
        cDangerKey = hack.formatAdditionalSetting("dangerColor"),
        cPlayerRotateKey = hack.formatAdditionalSetting("playerRotateColor"),
        cPlayerInnerKey = hack.formatAdditionalSetting("playerInnerColor"),
        cPlayerKey = hack.formatAdditionalSetting("playerColor"),
        cTrailKey = hack.formatAdditionalSetting("trailColor"),
        cInnerTrailKey = hack.formatAdditionalSetting("innerTrailColor")
    ](cocos2d::CCNode* popupNode){
        auto* popup = static_cast<HackSettingsPopup*>(popupNode);
        popup->addConfigToggle("Show Player", showPlayerKey, true);
        popup->addConfigToggle("Show on Death", onDeathKey, false, [](bool enabled) { GDH::Gui::get().rescanActiveCheats(); });
        popup->addConfigToggle("Show Death Object", showDeathObjKey, false, [](bool enabled) { GDH::Gui::get().rescanActiveCheats(); });
        popup->addConfigFloatInput("Size", sizeKey, 0, 20.f, 0.25f);
        popup->addConfigFloatInput("Fill Alpha", fillAlphaKey, 0, 1.f, 0.2f);

        popup->addSeparator();

        popup->addConfigToggle("Draw Trail", drawTrailKey, false);
        popup->addConfigIntInput("Trail Length", trailLengthKey, 0, 1000, 240);
        popup->addConfigColor4Hex("Trail Color", cTrailKey, "#FFFF00FF");
        popup->addConfigColor4Hex("Inner Trail Color", cInnerTrailKey, "#FF0000FF");

        popup->addSeparator();

        popup->addConfigColor4Hex("Triggers", cTriggersKey, "#FF00E6FF");
        popup->addConfigColor4Hex("Other Objects", cOtherKey, "#00FF00FF");
        popup->addConfigColor4Hex("Passable Blocks", cPassableKey, "#00FFFFFF");
        popup->addConfigColor4Hex("Solid (Blocks)", cSolidKey, "#003FFFFF");
        popup->addConfigColor4Hex("Danger (Spikes)", cDangerKey, "#FF0000FF");
        
        popup->addSeparator();

        popup->addConfigColor4Hex("Player Rotated Box", cPlayerRotateKey, "#800000FF");
        popup->addConfigColor4Hex("Player Inner Box", cPlayerInnerKey, "#003FFFFF");
        popup->addConfigColor4Hex("Player Box", cPlayerKey, "#FF0000FF");
    });

    hack.setCustomCheatingCheck([&config, onDeathKey = hack.formatAdditionalSetting("show_on_death"), showDeathObjKey = hack.formatAdditionalSetting("show_death_object")]() {
        if (config.get<bool>(onDeathKey, false) || config.get<bool>(showDeathObjKey, false)) return false;
        return true;
    });
}