#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/keybinds.hpp"
#include "../../core/recorder/recorder.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Startpos Switcher", "The ability to switch between starting positions using the keys that you setted in keybinds", false);

class $modify(StartposSwitcherPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<StartPosObject*> m_startPositions;
        int m_selectedStartpos = -1;
        cocos2d::CCMenu* startposSwitcherUI = nullptr;
        cocos2d::CCLabelBMFont* switcherLabel = nullptr;

        ~Fields() {
            auto& kb = GDH::Keybinds::get();
            kb.clearCallback("startpos_switcher::left");
            kb.clearCallback("startpos_switcher::right");
        }
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Startpos Switcher");        
        
        hack.addHookPtr(self.getHook("PlayLayer::addObject").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::createObjectsFromSetupFinished").unwrap());

        hack.setCustomWindowImGui([
            resetCamera_key = hack.formatAdditionalSetting("reset_camera"),
            minOpacity_key = hack.formatAdditionalSetting("min_opacity"),
            maxOpacity_key = hack.formatAdditionalSetting("max_opacity")
        ] {
            ImGuiWidgetConfig::Checkbox("Reset Camera", resetCamera_key, false);
            ImGuiWidgetConfig::DragInt("##Min Opacity", minOpacity_key, 1.f, 0, 255, 100, "Min Opacity: %d");
            ImGuiWidgetConfig::DragInt("##Max Opacity", maxOpacity_key, 1.f, 0, 255, 200, "Max Opacity: %d");
            ImGuiWidgetConfig::DrawCustomKeybindButton("startpos_switcher::left", "Left Switch", geode::Keybind(cocos2d::KEY_Q, KeyboardModifier::None));
            ImGuiWidgetConfig::DrawCustomKeybindButton("startpos_switcher::right", "Right Switch", geode::Keybind(cocos2d::KEY_E, KeyboardModifier::None));
        });

        hack.setCustomWindowCocos([
            resetCamera_key = hack.formatAdditionalSetting("reset_camera"),
            minOpacity_key = hack.formatAdditionalSetting("min_opacity"),
            maxOpacity_key = hack.formatAdditionalSetting("max_opacity")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigToggle("Reset Camera", resetCamera_key, false);
            popup->addConfigIntInput("Min Opacity (0 - 255)", minOpacity_key, 0, 255, 100);
            popup->addConfigIntInput("Max Opacity (0 - 255)", maxOpacity_key, 0, 255, 200);
        });
    }

    void updateSwitcherUI() {
        auto fields = m_fields.self();
        if (fields->switcherLabel && !fields->m_startPositions.empty()) {
            fields->switcherLabel->setString(
                fmt::format("{}/{}", fields->m_selectedStartpos + 1, fields->m_startPositions.size()).c_str()
            );
        }
        delayOpacityUI();
    }

    void switchStartPos(int incBy, bool direction = true) {
        auto pl = PlayLayer::get();
        if (!pl) return;
        
        auto fields = m_fields.self();
        if (fields->m_startPositions.empty() || pl->m_levelEndAnimationStarted) return;

        auto& config = Config::get();

        fields->m_selectedStartpos += incBy;

        if (fields->m_selectedStartpos < -1)
            fields->m_selectedStartpos = fields->m_startPositions.size() - 1;

        if (fields->m_selectedStartpos >= fields->m_startPositions.size())
            fields->m_selectedStartpos = -1;

        if (direction) {
            StartPosObject* obj = fields->m_selectedStartpos == -1 ? nullptr : fields->m_startPositions[fields->m_selectedStartpos];
            
            bool practiceMode = pl->m_isPracticeMode;
            if (practiceMode) {
                pl->togglePracticeMode(false);
            }

            pl->m_isTestMode = (obj != nullptr);
            pl->m_currentCheckpoint = nullptr;
            pl->setStartPosObject(obj);
            pl->resetLevel();

            if (config.get<bool>("level.startpos_switcher::reset_camera", false))
                pl->resetCamera();

            pl->startMusic();

            if (practiceMode) {
                pl->togglePracticeMode(true);
            }
        }
    }

    void delayOpacityUI() {
        auto fields = m_fields.self();
        if (fields->startposSwitcherUI) {
            auto& config = Config::get();
            GLubyte maxOpacity = static_cast<GLubyte>(config.get<int>("level.startpos_switcher::max_opacity", 200));
            GLubyte minOpacity = static_cast<GLubyte>(config.get<int>("level.startpos_switcher::min_opacity", 100));

            fields->startposSwitcherUI->stopAllActions();
            fields->startposSwitcherUI->setOpacity(maxOpacity);
            fields->startposSwitcherUI->runAction(CCSequence::create(
                CCDelayTime::create(2.f),
                CCFadeTo::create(0.3f, minOpacity),
                nullptr
            ));
        }
    }

    void addObject(GameObject* obj) {
        PlayLayer::addObject(obj);

        if (obj->m_objectID == 31) {
            m_fields->m_startPositions.push_back(static_cast<StartPosObject*>(obj));
        }
    }

    void createObjectsFromSetupFinished() {
        PlayLayer::createObjectsFromSetupFinished();
        
        auto& config = Config::get();
        auto fields = m_fields.self();

        std::sort(fields->m_startPositions.begin(), fields->m_startPositions.end(), 
            [](StartPosObject* a, StartPosObject* b) {
                return a->getPositionX() < b->getPositionX();
            });

        fields->m_selectedStartpos = -1;
        if (m_startPosObject) {
            auto& positions = fields->m_startPositions;
            for (int i = 0; i < positions.size(); i++) {
                if (positions[i] == m_startPosObject) {
                    fields->m_selectedStartpos = i;
                    break;
                }
            }
        }

        if (!fields->m_startPositions.empty()) {
            auto director = cocos2d::CCDirector::sharedDirector();
            auto win_size = director->getWinSize();
            float bottom = director->getScreenBottom();

            fields->switcherLabel = cocos2d::CCLabelBMFont::create(
                fmt::format("{}/{}", fields->m_selectedStartpos + 1, fields->m_startPositions.size()).c_str(), 
                "bigFont.fnt"
            );
            fields->switcherLabel->setScale(0.5f);
            fields->switcherLabel->setPosition(win_size.width / 2, 20.f);
            fields->switcherLabel->setID("startposSwitcherLabels"_spr);

            auto left_arrow = cocos2d::CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
            left_arrow->setScale(0.5f);
            auto left_arrowClick = geode::cocos::CCMenuItemExt::createSpriteExtra(left_arrow, [this](CCMenuItemSpriteExtra* sender) {
                switchStartPos(-1);
                updateSwitcherUI();
            });
            left_arrowClick->setPosition(win_size.width / 2 - 50, bottom + left_arrowClick->getScaledContentHeight());
            left_arrowClick->setID("startposSwitcherLeftArrowClick"_spr);

            auto right_arrow = cocos2d::CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");  
            right_arrow->setScale(0.5f);
            right_arrow->setFlipX(true);
            auto right_arrowClick = geode::cocos::CCMenuItemExt::createSpriteExtra(right_arrow, [this](CCMenuItemSpriteExtra* sender) {
                switchStartPos(1);
                updateSwitcherUI();
            });        
            right_arrowClick->setPosition(win_size.width / 2 + 50, bottom + right_arrowClick->getScaledContentHeight());
            right_arrowClick->setID("startpos_switcher_rightArrowClick"_spr);

            fields->startposSwitcherUI = cocos2d::CCMenu::create();
            fields->startposSwitcherUI->setID("startposSwitcherUI"_spr);
            fields->startposSwitcherUI->setPosition(0, 0);
            fields->startposSwitcherUI->setZOrder(999);

            fields->startposSwitcherUI->addChild(left_arrowClick);
            fields->startposSwitcherUI->addChild(right_arrowClick);
            fields->startposSwitcherUI->addChild(fields->switcherLabel);

            m_uiLayer->addChild(fields->startposSwitcherUI);

            delayOpacityUI();

            auto& kb = GDH::Keybinds::get();
            kb.addCallback("startpos_switcher::left", geode::Keybind(cocos2d::KEY_Q, KeyboardModifier::None), [this](bool repeat) {
                if (repeat) return;
                switchStartPos(-1);
                updateSwitcherUI();
            });

            kb.addCallback("startpos_switcher::right", geode::Keybind(cocos2d::KEY_E, KeyboardModifier::None), [this](bool repeat) {
                if (repeat) return;
                switchStartPos(1);
                updateSwitcherUI();
            });
        }
    }
    
    void resetLevel() {
        PlayLayer::resetLevel();

        auto fields = m_fields.self();
        auto& recorder = GDH::Recorder::get();
        if (!m_isPracticeMode && !fields->m_startPositions.empty()) {
            recorder.delay = m_gameState.m_levelTime;
        }
        else if (fields->m_startPositions.empty()) {
            recorder.delay = 0;
        }      
    }
};