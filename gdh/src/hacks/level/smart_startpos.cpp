#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Smart Startpos", "Restores correct gameplay without configuring startpos settings", false);

class $modify(SmartStartposPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<GameObject*> gravityPortals, dualPortals, gamemodePortals, miniPortals, speedChanges;
        std::vector<StartPosObject*> startPositions;
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Smart Startpos");        
        
        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::addObject").unwrap());

        hack.setCustomWindowImGui([
            gravityPortals = hack.formatAdditionalSetting("gravityPortals"),
            dualPortals = hack.formatAdditionalSetting("dualPortals"),
            gamemodePortals = hack.formatAdditionalSetting("gamemodePortals"),
            miniPortals = hack.formatAdditionalSetting("miniPortals"),
            speedChanges = hack.formatAdditionalSetting("speedChanges")
        ]() {
            ImGuiWidgetConfig::Checkbox("Gravity Portals", gravityPortals, false);
            ImGuiWidgetConfig::Checkbox("Dual Portals", dualPortals, true);
            ImGuiWidgetConfig::Checkbox("Gamemode Portals", gamemodePortals, true);
            ImGuiWidgetConfig::Checkbox("Mini Portals", miniPortals, true);
            ImGuiWidgetConfig::Checkbox("Speed Changes", speedChanges, true);
        });

        hack.setCustomWindowCocos([
            gravityPortals = hack.formatAdditionalSetting("gravityPortals"),
            dualPortals = hack.formatAdditionalSetting("dualPortals"),
            gamemodePortals = hack.formatAdditionalSetting("gamemodePortals"),
            miniPortals = hack.formatAdditionalSetting("miniPortals"),
            speedChanges = hack.formatAdditionalSetting("speedChanges")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigToggle("Gravity Portals", gravityPortals, false);
            popup->addConfigToggle("Dual Portals", dualPortals, true);
            popup->addConfigToggle("Gamemode Portals", gamemodePortals, true);
            popup->addConfigToggle("Mini Portals", miniPortals, true);
            popup->addConfigToggle("Speed Changes", speedChanges, true);
        });
    }

    void setupStartPos(StartPosObject* startPos) {
        auto fields = m_fields.self();
        auto& config = Config::get();
        LevelSettingsObject* startPosSettings = startPos->m_startSettings;

        auto getClosestObject = [](std::vector<GameObject*>& vec, StartPosObject* startPos) -> GameObject* {
            GameObject* closest = nullptr;

            std::sort(vec.begin(), vec.end(), [](GameObject* a, GameObject* b) {
                return a->getPositionX() < b->getPositionX();
            });

            for (auto obj : vec) {
                if (obj->getPositionX() - 10 > startPos->getPositionX())
                    break;
                if (obj->getPositionX() - 10 < startPos->getPositionX())
                    closest = obj;
            }

            return closest;
        };

        bool checkGravity = config.get<bool>("level.smart_startpos::gravityPortals", false);
        bool checkDual = config.get<bool>("level.smart_startpos::dualPortals", true);
        bool checkGamemode = config.get<bool>("level.smart_startpos::gamemodePortals", true);
        bool checkMini = config.get<bool>("level.smart_startpos::miniPortals", true);
        bool checkSpeed = config.get<bool>("level.smart_startpos::speedChanges", true);

        if (checkGravity) {
            if (auto obj = getClosestObject(fields->gravityPortals, startPos)) {
                startPosSettings->m_isFlipped = (obj->m_objectID == 11);
            }    
        }

        if (checkDual) {
            if (auto obj = getClosestObject(fields->dualPortals, startPos)) {
                startPosSettings->m_startDual = (obj->m_objectID == 286);
            }
        }

        if (checkGamemode) {
            if (auto obj = getClosestObject(fields->gamemodePortals, startPos)) {
                switch (obj->m_objectID) {
                    case 12:   startPosSettings->m_startMode = 0; break;
                    case 13:   startPosSettings->m_startMode = 1; break;
                    case 47:   startPosSettings->m_startMode = 2; break;
                    case 111:  startPosSettings->m_startMode = 3; break;
                    case 660:  startPosSettings->m_startMode = 4; break;
                    case 745:  startPosSettings->m_startMode = 5; break;
                    case 1331: startPosSettings->m_startMode = 6; break;
                    case 1933: startPosSettings->m_startMode = 7; break;
                }
            }
        }

        if (checkMini) {
            if (auto obj = getClosestObject(fields->miniPortals, startPos)) {
                startPosSettings->m_startMini = (obj->m_objectID == 101);
            }  
        }

        if (checkSpeed) {
            if (auto obj = getClosestObject(fields->speedChanges, startPos)) {
                switch (obj->m_objectID) {
                    case 200:  startPosSettings->m_startSpeed = Speed::Slow; break;
                    case 201:  startPosSettings->m_startSpeed = Speed::Normal; break;
                    case 202:  startPosSettings->m_startSpeed = Speed::Fast; break;
                    case 203:  startPosSettings->m_startSpeed = Speed::Faster; break;
                    case 1334: startPosSettings->m_startSpeed = Speed::Fastest; break;
                }
            }
        }
    }

    void resetLevel() {
        auto fields = m_fields.self();
        for (StartPosObject* obj : fields->startPositions)
            setupStartPos(obj);
            
        PlayLayer::resetLevel();
    }

    void addObject(GameObject* obj) {
        PlayLayer::addObject(obj);
        
        auto fields = m_fields.self();
        switch (obj->m_objectID) {
            case 9:
            case 10:
            case 11:
                fields->gravityPortals.push_back(obj);
            break;
            case 31:
                fields->startPositions.push_back(static_cast<StartPosObject*>(obj));
                break;
            case 12:
            case 13:
            case 47:
            case 111:
            case 660:
            case 745:
            case 1331:
            case 1933:
                fields->gamemodePortals.push_back(obj);
                break;
            case 99:
            case 101:
                fields->miniPortals.push_back(obj);
                break;
            case 286:
            case 287:
                fields->dualPortals.push_back(obj);
                break;
            case 200:
            case 201:
            case 202:
            case 203:
            case 1334:
                fields->speedChanges.push_back(obj);
                break;
            default:
                break;
        }
    }
};