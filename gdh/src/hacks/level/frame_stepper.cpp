#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/keybinds.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../core/ringBuffer.hpp"
#include <imgui-cocos.hpp>
#include "frame_stepper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Level", "Frame Stepper", "Allows forwards and backwards stepping frame-by-frame", true);

static bool g_nextFrame = false;
static bool g_prevFrame = false;
static bool g_pause = true;
static int g_framesToAdvance = 0;

// for android
static bool g_isHolding = false;
static float g_holdTimer = 0.0f;
//

bool FrameStepper::g_back = false;

static const auto release_cp = [](CheckpointObject* cp) {
    if (cp) cp->release();
};

static RingBuffer<CheckpointObject*> g_checkpoints(240);

class $modify(FrameStepperCCScheduler, cocos2d::CCScheduler) {
    static void onModify(auto& self) {
        (void) self.setHookPriority("cocos2d::CCScheduler::update", geode::Priority::Late); 

        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Frame Stepper");
        hack.addHookPtr(self.getHook("cocos2d::CCScheduler::update").unwrap());

        hack.setHandler([](bool enabled) {
            if (enabled) {
                g_prevFrame = false;
                g_pause = true;
                g_framesToAdvance = 0;
                FrameStepper::g_back = false;
            }
        });

        hack.setCustomWindowImGui([]() {
            ImGuiWidgetConfig::DrawCustomKeybindButton("frame_stepper::step_key", "Step");
            ImGuiWidgetConfig::DrawCustomKeybindButton("frame_stepper::advance_key", "Multi-Step");
            ImGuiWidgetConfig::DrawCustomKeybindButton("frame_stepper::pause_key", "Pause");
            ImGuiWidgetConfig::DrawCustomKeybindButton("frame_stepper::back_key", "Back");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGuiWidgetConfig::DragInt("##AdvanceFramesCount", "frame_stepper::advance_frames_count", 1, 1, 100, 5, "Multi-Step Frames Count: %d");
            ImGuiWidgetConfig::Checkbox("Backward Stepping", "frame_stepper::backward", false);
            ImGuiWidgetConfig::DragInt("##BackFrameLength", "frame_stepper::back_frames_length", 1, 1, 1000, 240, "Back Frames Length: %d");
        });

        hack.setCustomWindowCocos([](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigIntInput("Multi-Step Frames Count", "frame_stepper::advance_frames_count", 1, 100, 5);
            popup->addConfigFloatInput("Hold Timer (s)", "frame_stepper::hold_timer", 0.01f, 1.f, 0.15f);
            popup->addSeparator();
            popup->addConfigToggle("Backward Stepping", "frame_stepper::backward", false);
            popup->addConfigIntInput("Back Frames Length", "frame_stepper::back_frames_length", 1, 1000, 240);
        });
    }

    void update(float dt) {
        auto pl = PlayLayer::get();
        auto& config = Config::get();
        if (!pl || pl->m_isPaused || !pl->m_started || pl->m_levelEndAnimationStarted || (pl->m_resumeTimer > 0 && !g_prevFrame)) {
            return CCScheduler::update(dt);
        }

        if (g_prevFrame) {
            g_prevFrame = false;

            if (!g_checkpoints.empty()) {
                bool respawn_blink = config.get<bool>("core.no_respawn_blink", false);
                config.set<bool>("core.no_respawn_blink", true);

                bool safe_mode = config.get<bool>("core.safe_mode", false);
                config.set<bool>("core.safe_mode", true);

                FrameStepper::g_back = true;
                pl->resetLevel();
                FrameStepper::g_back = false;
                
                config.set<bool>("core.no_respawn_blink", respawn_blink);
                config.set<bool>("core.safe_mode", safe_mode);

                CheckpointObject* last = nullptr;
                g_checkpoints.pop_back(last);
                if (last) last->release();

                g_nextFrame = true;
            }
        }

        if (g_isHolding) {
            g_holdTimer += dt;
            if (g_holdTimer > config.get<float>("frame_stepper::hold_timer", 0.15f)) {
                g_holdTimer = 0;
                CCScheduler::update(dt);
                return;
            }
        }

        if (g_pause) {
            if (g_nextFrame) {
                g_nextFrame = false;
                CCScheduler::update(dt);
            } else if (g_framesToAdvance > 0) {
                g_framesToAdvance--;
                CCScheduler::update(dt);
            }
            return;
        }

        CCScheduler::update(dt);
    }
};

static size_t g_lastFramesLength = 0;
class $modify(FrameStepperGJBaseGameLayer, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

        if (!Config::get().get<bool>("frame_stepper::backward", false)) return;

        auto pl = PlayLayer::get();
        if (!pl || !pl->m_started || pl->m_playerDied) return;

        auto& config = Config::get();
        auto framesLength = static_cast<size_t>(config.get<int>("frame_stepper::back_frames_length", 240));

        if (g_lastFramesLength != framesLength) {
            g_checkpoints.clear(release_cp);
            g_checkpoints.init(framesLength);
            g_lastFramesLength = framesLength;
        }

        auto cp = pl->createCheckpoint();
        cp->retain(); 
        g_checkpoints.push(cp, release_cp);
    }
};

class ArrowNext : public CCMenuItemSpriteExtra {
public:
    static ArrowNext* create(cocos2d::CCNode* sprite) {
        auto ret = new ArrowNext();
        if (ret && ret->init(sprite, nullptr, ret, nullptr)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    virtual void selected() override {
        CCMenuItemSpriteExtra::selected();
        g_pause = true;
        g_nextFrame = true;
        g_isHolding = true;
        g_holdTimer = 0.0f;
    }
    
    virtual void unselected() override {
        CCMenuItemSpriteExtra::unselected();
        g_isHolding = false;
        g_pause = true;
    }
};

class $modify(FrameStepperPlayLayer, PlayLayer) {
    struct Fields {
        ~Fields() {
            g_checkpoints.clear(release_cp);
        }

        cocos2d::CCMenu* m_stepperMenu = nullptr;
        bool isMenuVisible = false;
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Frame Stepper");
        
        hack.addHookPtr(self.getHook("PlayLayer::updateAttempts").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        m_fields.self();
        return true;
    }

    void updateAttempts() {
        if (FrameStepper::g_back) return;

        PlayLayer::updateAttempts();
    }

    void resetLevel() {
        CheckpointObject* last = nullptr;
        bool testmode = m_isTestMode;

        if (FrameStepper::g_back && g_checkpoints.back(last) && last) {
            m_checkpointArray->addObject(last);
            m_isTestMode = true;
        }
        else {
            g_checkpoints.clear(release_cp);
        }

        PlayLayer::resetLevel();

        if (FrameStepper::g_back && !g_checkpoints.empty()) {
            m_isTestMode = testmode;
            removeCheckpoint(false);
        }
    }

    void storeCheckpoint(CheckpointObject *checkpoint) {
        PlayLayer::storeCheckpoint(checkpoint);
        g_checkpoints.clear(release_cp);
    }
    
    #ifdef GEODE_IS_MOBILE
    void createObjectsFromSetupFinished() {
        PlayLayer::createObjectsFromSetupFinished();

        auto director = cocos2d::CCDirector::sharedDirector();

        // Back Frame
        auto back_arrow = cocos2d::CCSprite::create("GDH_arrow.png"_spr);  
        back_arrow->setScale(1.5f);

        auto back_arrow_click = geode::cocos::CCMenuItemExt::createSpriteExtra(back_arrow, [](CCMenuItemSpriteExtra* sender) {
            g_pause = true;
            g_prevFrame = true;
        });  

        back_arrow_click->setPosition({50.f, 50.f});
        back_arrow_click->setOpacity(200);
        back_arrow_click->setID("frame_stepper_back_btn"_spr);

        // Next Frame
        auto next_arrow = cocos2d::CCSprite::create("GDH_arrow.png"_spr);  
        next_arrow->setScale(1.5f);
        next_arrow->setFlipX(true);

        auto next_step_click = ArrowNext::create(next_arrow);
        next_step_click->setPosition({125.f, 50.f});
        next_step_click->setOpacity(255);
        next_step_click->setID("frame_stepper_next_btn"_spr);

        // Multi-Step Frame
        auto next_multi_arrow = cocos2d::CCSprite::create("GDH_arrowMulti.png"_spr);  
        next_multi_arrow->setScale(1.5f);
        next_multi_arrow->setFlipX(true);

        auto next_multi_arrow_click = geode::cocos::CCMenuItemExt::createSpriteExtra(next_multi_arrow, [](CCMenuItemSpriteExtra* sender) {
            g_pause = true;
            g_framesToAdvance = Config::get().get<int>("frame_stepper::advance_frames_count", 5);
        });        
        
        next_multi_arrow_click->setPosition({200.f, 50.f});
        next_multi_arrow_click->setOpacity(255);
        next_multi_arrow_click->setID("frame_stepper_next_multi_btn"_spr);

        auto fields = m_fields.self();
        fields->m_stepperMenu = cocos2d::CCMenu::create();
        fields->m_stepperMenu->setID("frame_stepper_menu"_spr);
        fields->m_stepperMenu->setPosition(0, 0);
        fields->m_stepperMenu->setZOrder(999);

        fields->m_stepperMenu->addChild(back_arrow_click);
        fields->m_stepperMenu->addChild(next_step_click);
        fields->m_stepperMenu->addChild(next_multi_arrow_click);

        auto& config = Config::get();
        bool enabled = config.get<bool>("level.frame_stepper", false);
        if (m_fields->m_stepperMenu) {
            m_fields->m_stepperMenu->setVisible(enabled);
        }

        m_uiLayer->addChild(fields->m_stepperMenu);
    }
    
    void toggleUI() {
        auto& config = Config::get();
        bool enabled = config.get<bool>("level.frame_stepper", false);
        if (m_fields->m_stepperMenu) {
            m_fields->m_stepperMenu->setVisible(enabled);
        }
    }

    void pauseGame(bool unfocused) {
        PlayLayer::pauseGame(unfocused);
        toggleUI();
    }
    
    void resume() {
        PlayLayer::resume();
        toggleUI();
    }

    void resumeAndRestart(bool fromStart) {
        PlayLayer::resumeAndRestart(fromStart);

        toggleUI();
    }

    #endif
};

#ifdef GEODE_IS_DESKTOP
$execute {
    auto& kb = GDH::Keybinds::get();
    kb.addCallback("frame_stepper::step_key", geode::Keybind(cocos2d::KEY_G, KeyboardModifier::None), [](bool repeat) {
        g_pause = true;
        g_nextFrame = true;
    });

    kb.addCallback("frame_stepper::advance_key", geode::Keybind(cocos2d::KEY_G, KeyboardModifier::Control), [](bool repeat) {
        g_pause = true;
        g_framesToAdvance = Config::get().get<int>("frame_stepper::advance_frames_count", 5);
    });

    kb.addCallback("frame_stepper::pause_key", geode::Keybind(cocos2d::KEY_H, KeyboardModifier::None), [](bool repeat) {
        if (repeat) return;
        g_pause = !g_pause;
    });

    kb.addCallback("frame_stepper::back_key", geode::Keybind(cocos2d::KEY_F, KeyboardModifier::None), [](bool repeat) {
        g_pause = true;
        g_prevFrame = true;
    });
}
#endif