#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/replayEngine.hpp"
#include "../../core/labels.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Watch Replay", "Restores the classic Watch Replay button that was in 2.1. A reworked concept that replays your last attempt", false);

static std::vector<input_data> g_actions;
static bool g_isWatchingReplay = false;
static int g_indexAction = 0;

class $modify(WatchReplayPlayLayer, PlayLayer) {
    struct Fields {
        ~Fields() {
            g_actions.clear();
            g_isWatchingReplay = 0;
            g_indexAction = 0;
        }
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRun) {
        if (!PlayLayer::init(level, useReplay, dontRun)) return false;
        m_fields.self();
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        g_indexAction = 0;

        if (!g_isWatchingReplay)
            g_actions.clear();
    }
    
    void pauseGame(bool unfocused) {
        if (!this->canPauseGame() || AppDelegate::get()->m_gamePaused) {
            return PlayLayer::pauseGame(unfocused);
        }

        bool swapControls = GameManager::get()->getGameVariable(GameVar::Flip2PlayerControls);
        g_actions.push_back({m_gameState.m_currentProgress+1, false, 1, swapControls ? false : true});
        
        if (m_isPlatformer) {
            g_actions.push_back({m_gameState.m_currentProgress+1, false, 2, swapControls ? false : true});
            g_actions.push_back({m_gameState.m_currentProgress+1, false, 3, swapControls ? false : true});
        }

        PlayLayer::pauseGame(unfocused);
    }

    void playEndAnimationToPos(cocos2d::CCPoint pos) {
        PlayLayer::playEndAnimationToPos(pos);
        geode::queueInMainThread([]() { g_isWatchingReplay = false; });
    }
    
    void playPlatformerEndAnimationToPos(cocos2d::CCPoint pos, bool instant) {
        PlayLayer::playPlatformerEndAnimationToPos(pos, instant);
        geode::queueInMainThread([]() { g_isWatchingReplay = false; });
    }
};

class $modify(WatchReplayGJBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (!g_isWatchingReplay) {
            g_actions.push_back({m_gameState.m_currentProgress, down, button, isPlayer1});
        }
    }

    void processQueuedButtons(float dt, bool clearInputQueue) {
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);

        if (!g_isWatchingReplay || g_actions.empty()) return;

        auto& cps = CpsCounter::get();
        while (g_indexAction < g_actions.size() && m_gameState.m_currentProgress >= g_actions[g_indexAction].frame)
        {
            #ifdef GEODE_IS_ANDROID
            this->m_allowedButtons.clear();
            #endif

            this->handleButton(g_actions[g_indexAction].down, g_actions[g_indexAction].button, g_actions[g_indexAction].isPlayer2);

            g_indexAction++;
        }
    }
};

class $modify(WatchReplayEndLevelLayer, EndLevelLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Watch Replay");        
        
        hack.addHookPtr(self.getHook("EndLevelLayer::showLayer").unwrap());
    }

    void showLayer(bool p0) {
        EndLevelLayer::showLayer(p0);

        auto pl = PlayLayer::get();
        auto& engine = GDH::ReplayEngine::get();

        if (pl && !pl->m_isPracticeMode) {
            auto watchReplay = CCSprite::create("GJ_watchReplay_001.png"_spr);
    
            auto watchReplayClick = CCMenuItemExt::createSpriteExtra(watchReplay, [this](CCMenuItemSpriteExtra* sender) {
                auto& engine = GDH::ReplayEngine::get();
                if (engine.mode != state::disable) {
                    FLAlertLayer::create("Replay Engine", "Please disable Replay Engine to Watch Replay", "OK")->show();
                    return;
                }
                
                g_isWatchingReplay = true;
                this->onReplay(nullptr);
            });
    
            watchReplayClick->setID("watchReplay"_spr);
            watchReplayClick->setRotation(-10);
    
            auto winsize = CCDirector::get()->getWinSize();
            watchReplayClick->setPosition({winsize.width/2 - 149.f, winsize.height/2 - 14.f});
    
            auto menu = CCMenu::create();
            menu->setPosition({0, 0});
            menu->addChild(watchReplayClick);
    
            m_mainLayer->addChild(menu);
        }
    }
};