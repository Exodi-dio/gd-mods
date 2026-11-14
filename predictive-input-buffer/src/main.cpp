// Predictive Input Buffer - Geode 6 / GD 2.2091 port of ai.predictivebuffer by lilWC.
//
// The mod simulates the trajectory of the player with a ghost copy of it and,
// when the input that is being made right now would kill the player, delays
// that input until the simulated trajectory is safe again.
//
//   * 1 tick = 1/240 s, 4 ticks = 1 frame at 60 fps.
//   * The simulation horizon is 100 ticks (50 for mini waves, 65 for waves).
//   * The input state machine keeps one pending input at a time:
//     -1 idle, -2 release pending, >= 0 ticks left.

#include <Geode/Geode.hpp>
#include <Geode/modify/EffectGameObject.hpp>
#include <Geode/modify/EnhancedGameObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <algorithm>
#include <cmath>

#include "state.hpp"

using namespace geode::prelude;

namespace ib {

    // The simulation runs at 240 Hz, the game at 60 fps.
    constexpr float kTickFactor = 0.25f;
    // How many ticks of the trajectory are simulated at most.
    constexpr int kHorizonNormal = 100;
    constexpr int kHorizonWave = 65;
    constexpr int kHorizonMini = 50;
    // The simulation budget. It is refilled by 40 ticks per frame.
    constexpr int kBudgetMax = 660;
    constexpr int kBudgetRefill = 40;
    // A new delay has to improve the survival by this many ticks.
    constexpr int kMinImprovement = 12;

    State& S() {
        static State state;
        return state;
    }

    bool isWave(PlayLayer* playLayer) {
        if (!playLayer) return false;
        return playLayer->m_gameState.m_isDualMode && playLayer->m_player2 != nullptr;
    }

    void syncConfig() {
        auto* mod = Mod::get();
        auto& cfg = S().cfg;

        cfg.enabled = mod->getSettingValue<bool>("enabled");
        cfg.normalTicks = std::clamp(mod->getSettingValue<int>("normal-ticks"), 0, 40);
        cfg.waveTicks = std::clamp(mod->getSettingValue<int>("wave-ticks"), 0, 40);
        cfg.spamGuard = mod->getSettingValue<bool>("spam-guard");
        cfg.spamInterval = std::clamp(mod->getSettingValue<int>("spam-interval"), 4, 60);
        cfg.noclip = mod->getSettingValue<bool>("noclip");
        cfg.gameSpeed = std::clamp(mod->getSettingValue<float>("game-speed"), 0.1f, 2.f);
        cfg.autoClose = std::clamp(mod->getSettingValue<int>("auto-close"), 0, 120);
        cfg.buttonCorner = mod->getSettingValue<std::string>("button-corner");
        cfg.buttonScale = std::clamp(mod->getSettingValue<float>("button-scale"), 0.4f, 1.2f);
    }

    void applySettings() {
        syncConfig();
        auto& cfg = S().cfg;

        log::info(
            "[InputBuffer] Config -> {} (normal {} ticks | wave {} ticks)",
            cfg.enabled ? "ON" : "OFF",
            cfg.normalTicks,
            cfg.waveTicks
        );
    }

    void resetRuntime() {
        auto& st = S();
        st.rawDown = false;
        st.handledDown = false;
        st.pendingDelay = -1;
        st.lastBufferedDown = false;
        st.releasePending = false;
        st.spamCounter = 999;
        st.rapidClicks = 0;
        st.failsafeCounter = 0;
        st.frameCounter = 0;
        st.simulating = false;
        st.injecting = false;
        st.abortSim = false;
        st.ghostDied = false;
        st.budget = 700;
        st.gameSpeedApplied = 0.f;
    }

    // -------------------------------------------------------------------------
    //  Ghost players
    // -------------------------------------------------------------------------

    namespace {
        int iconOf(PlayerObject* player) {
            if (!player) return 0;
            if (player->m_isDart) return 7;
            if (player->m_isRobot) return 4;
            if (player->m_isSpider) return 5;
            if (player->m_isSwing) return 6;
            if (player->m_isBird) return 3;
            if (player->m_isBall) return 2;
            if (player->m_isShip) return 1;
            return 0;
        }

        void copyGhostState(PlayerObject* ghost, PlayerObject* player) {
            if (!ghost || !player) return;

            ghost->copyAttributes(player);

            ghost->m_isShip = player->m_isShip;
            ghost->m_isBird = player->m_isBird;
            ghost->m_isBall = player->m_isBall;
            ghost->m_isDart = player->m_isDart;
            ghost->m_isRobot = player->m_isRobot;
            ghost->m_isSpider = player->m_isSpider;
            ghost->m_isSwing = player->m_isSwing;
            ghost->m_isUpsideDown = player->m_isUpsideDown;
            ghost->m_isSideways = player->m_isSideways;
            ghost->m_isGoingLeft = player->m_isGoingLeft;
            ghost->m_isOnGround = player->m_isOnGround;
            ghost->m_isOnSlope = player->m_isOnSlope;
            ghost->m_wasOnSlope = player->m_wasOnSlope;
            ghost->m_isPlatformer = player->m_isPlatformer;
            ghost->m_playerSpeed = player->m_playerSpeed;
            ghost->m_vehicleSize = player->m_vehicleSize;
            ghost->m_gravityMod = player->m_gravityMod;
            ghost->m_yVelocity = player->m_yVelocity;
            ghost->m_platformerXVelocity = player->m_platformerXVelocity;
            ghost->m_lastPosition = player->m_lastPosition;

            ghost->setPosition(player->getPosition());
            ghost->setRotation(player->getRotation());
            ghost->setScaleX(player->getScaleX());
            ghost->setScaleY(player->getScaleY());
            ghost->m_isDead = false;
        }

        void applyButton(PlayerObject* player, bool down) {
            if (!player) return;
            if (down) {
                player->pushButton(PlayerButton::Jump);
            } else {
                player->releaseButton(PlayerButton::Jump);
            }
        }

        void clearCollisionLogs(PlayerObject* player) {
            if (!player) return;
            if (player->m_collisionLogTop) player->m_collisionLogTop->removeAllObjects();
            if (player->m_collisionLogBottom) player->m_collisionLogBottom->removeAllObjects();
            if (player->m_collisionLogLeft) player->m_collisionLogLeft->removeAllObjects();
            if (player->m_collisionLogRight) player->m_collisionLogRight->removeAllObjects();
        }
    }

    void destroyGhosts() {
        auto& st = S();

        if (st.ghost1) {
            st.ghost1->removeFromParent();
            st.ghost1->release();
            st.ghost1 = nullptr;
        }
        if (st.ghost2) {
            st.ghost2->removeFromParent();
            st.ghost2->release();
            st.ghost2 = nullptr;
        }
        if (st.holder) {
            st.holder->removeFromParent();
            st.holder->release();
            st.holder = nullptr;
        }

        resetRuntime();
    }

    void createGhosts(PlayLayer* playLayer) {
        destroyGhosts();

        auto& st = S();
        if (!playLayer || !playLayer->m_objectLayer) return;

        // The ghosts live in an invisible container, so their trails and
        // particles are never shown.
        auto* holder = CCLayer::create();
        if (!holder) return;
        holder->setVisible(false);
        holder->retain();
        playLayer->m_objectLayer->addChild(holder);
        st.holder = holder;

        auto make = [&](PlayerObject*& out, PlayerObject* source, int index) {
            out = PlayerObject::create(index, iconOf(source), playLayer, holder, true);
            if (!out) return;
            out->retain();
            out->setVisible(false);
            out->setOpacity(0);
            holder->addChild(out);
        };

        make(st.ghost1, playLayer->m_player1, 1);
        make(st.ghost2, playLayer->m_player2, 2);

        if (!st.ghost1) {
            log::warn("[InputBuffer] could not create the ghost player, the buffer stays inactive");
        }
    }

    // -------------------------------------------------------------------------
    //  Trajectory simulation
    // -------------------------------------------------------------------------

    int getSurvivalTicks(
        PlayLayer* playLayer,
        PlayerObject* p1,
        PlayerObject* p2,
        bool wave,
        int base,
        bool down,
        int maxTicks
    ) {
        auto& st = S();

        auto* ghost1 = st.ghost1;
        auto* ghost2 = (wave && p2) ? st.ghost2 : nullptr;

        if (!ghost1 || !playLayer || maxTicks <= 0) return maxTicks;

        st.simulating = true;
        st.abortSim = false;
        st.ghostDied = false;

        copyGhostState(ghost1, p1);
        if (ghost2) copyGhostState(ghost2, p2);

        // The state the button had before the buffered input is applied.
        if (base >= 1) {
            applyButton(ghost1, !down);
            if (ghost2) applyButton(ghost2, !down);
        }

        float dt = st.simDt;
        if (!(dt > 0.001f)) dt = 0.25f;

        int survived = 0;
        const int total = base + maxTicks;

        for (int tick = 0; tick < total; ++tick) {
            if (--st.budget < 0) {
                st.abortSim = true;
                break;
            }

            if (tick == base) {
                applyButton(ghost1, down);
                if (ghost2) applyButton(ghost2, down);
            }

            ghost1->update(dt);
            ghost1->updateRotation(dt);
            if (ghost2) {
                ghost2->update(dt);
                ghost2->updateRotation(dt);
            }

            if (st.abortSim) break;

            clearCollisionLogs(ghost1);
            playLayer->checkCollisions(ghost1, dt, false);
            if (st.ghostDied) break;

            if (ghost2) {
                clearCollisionLogs(ghost2);
                playLayer->checkCollisions(ghost2, dt, false);
                if (st.ghostDied) break;
            }

            if (tick >= base) survived++;
        }

        st.simulating = false;

        return survived;
    }

    // -------------------------------------------------------------------------
    //  Input helpers
    // -------------------------------------------------------------------------

    void fireButton(PlayLayer* playLayer, bool down) {
        if (!playLayer) return;

        auto& st = S();

        st.injecting = true;
        st.rawDown = down;

        playLayer->handleButton(down, 1, true);

        // The second player only gets the input when the wave player is not
        // the one that is being controlled.
        if (isWave(playLayer) && (!playLayer->m_player2 || !playLayer->m_player2->m_isDart)) {
            playLayer->handleButton(down, 1, false);
        }

        st.injecting = false;
    }

    void failsafeSync(PlayLayer* playLayer) {
        auto& st = S();
        if (!playLayer) return;

        if (st.handledDown == st.rawDown) return;

        if (st.failsafeCounter > 3) {
            st.handledDown = st.rawDown;
            return;
        }

        st.failsafeCounter++;
        st.injecting = true;
        st.handledDown = st.rawDown;

        playLayer->handleButton(st.rawDown, 1, true);
        if (isWave(playLayer) && (!playLayer->m_player2 || !playLayer->m_player2->m_isDart)) {
            playLayer->handleButton(st.rawDown, 1, false);
        }

        st.injecting = false;
    }

}

// ---------------------------------------------------------------------------
//  Mod entry points
// ---------------------------------------------------------------------------

$on_mod(Loaded) {
    ib::applySettings();
    log::info("[InputBuffer] v4.7 port LOADED");
}

// ---------------------------------------------------------------------------
//  Hooks
// ---------------------------------------------------------------------------

class $modify(IBPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool dontCreateObjects) {
        auto ret = PlayLayer::init(level, dontCreateObjects);
        if (ret) {
            ib::resetRuntime();
            ib::createGhosts(this);
        }
        return ret;
    }

    void onQuit() {
        ib::destroyGhosts();
        PlayLayer::onQuit();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        auto& st = ib::S();

        if (!this->m_isPracticeMode) {
            st.gameSpeedApplied = 0.f;
            st.gameSpeedPlayer = nullptr;
            return;
        }

        float speed = st.cfg.gameSpeed;
        if (speed == st.gameSpeedApplied && st.gameSpeedPlayer == this->m_player1) return;

        st.gameSpeedApplied = speed;
        st.gameSpeedPlayer = this->m_player1;

        if (this->m_player1) {
            this->m_player1->updateTimeMod(speed, true);
        }
        if (this->m_gameState.m_isDualMode && this->m_player2) {
            this->m_player2->updateTimeMod(speed, true);
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        auto& st = ib::S();

        if (st.simulating && (player == st.ghost1 || player == st.ghost2)) {
            // The ghost hit something: the simulation stops here.
            st.ghostDied = true;
            return;
        }

        if (st.cfg.noclip && this->m_isPracticeMode && player == this->m_player1) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }
};

class $modify(IBGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto& st = ib::S();

        if (button != 1 || st.simulating || st.injecting) {
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        auto* playLayer = PlayLayer::get();
        if (!playLayer || static_cast<GJBaseGameLayer*>(playLayer) != static_cast<GJBaseGameLayer*>(this)) {
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        bool wave = ib::isWave(playLayer);

        if (!isPlayer1) {
            // The second player is only touched when it is not the wave player.
            bool dart = wave && playLayer->m_player2 && playLayer->m_player2->m_isDart;
            if (dart || wave) {
                return GJBaseGameLayer::handleButton(down, button, isPlayer1);
            }
            // While an input is waiting to be fired the second player waits too.
            if (st.pendingDelay >= 0 || st.pendingDelay == -2) {
                return;
            }
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        st.rawDown = down;

        if (!st.cfg.enabled) {
            st.handledDown = down;
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        auto* p1 = this->m_player1;
        auto* p2 = wave ? this->m_player2 : nullptr;

        if (!p1 || p1->m_isHidden || p1->m_isDead || (p2 && p2->m_isDead)) {
            st.pendingDelay = -1;
            st.releasePending = false;
            st.handledDown = down;
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        // An input is already waiting to be fired: only remember the new state.
        if (st.pendingDelay >= 0) {
            st.releasePending = down != st.lastBufferedDown;
            st.spamCounter = 0;
            return;
        }

        if (st.pendingDelay == -2) {
            st.pendingDelay = -1;
            st.releasePending = false;
            bool wanted = !st.lastBufferedDown;
            if (st.handledDown != wanted) {
                ib::fireButton(playLayer, wanted);
            }
        }

        // Spam guard: three clicks in a row are let through untouched.
        if (st.spamCounter < st.cfg.spamInterval) {
            st.spamCounter = 0;
            if (st.cfg.spamGuard && st.rapidClicks >= 2) {
                st.handledDown = down;
                return GJBaseGameLayer::handleButton(down, button, isPlayer1);
            }
            st.rapidClicks++;
        } else {
            st.spamCounter = 0;
        }

        bool dart1 = p1->m_isDart;
        bool dart2 = p2 && p2->m_isDart;
        bool anyDart = dart1 || dart2;

        // Outside of wave only the press is buffered, the release of the
        // robot still has to be delayed as well.
        if (!anyDart && !down) {
            bool robot = p1->m_isRobot || (p2 && p2->m_isRobot);
            if (!robot) {
                st.handledDown = down;
                return GJBaseGameLayer::handleButton(down, button, isPlayer1);
            }
        }

        int limit = anyDart ? st.cfg.waveTicks : st.cfg.normalTicks;
        if (limit <= 0) {
            st.handledDown = down;
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        // The mini waves are simulated with a shorter horizon.
        bool mini = p1->m_vehicleSize < 1.f;
        if (!mini && dart2) mini = p2->m_vehicleSize < 1.f;

        int horizon = mini ? ib::kHorizonMini : (anyDart ? ib::kHorizonWave : ib::kHorizonNormal);

        if (st.budget < horizon * 2) {
            st.handledDown = down;
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        int frame0 = ib::getSurvivalTicks(playLayer, p1, p2, wave, 0, down, horizon);

        // The input is safe right away, nothing has to be delayed.
        if (frame0 >= horizon) {
            st.handledDown = down;
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        int bestDelay = 0;
        int bestResult = frame0;

        for (int delay = 1; delay <= limit; ++delay) {
            if (st.budget < horizon) break;

            int result = ib::getSurvivalTicks(playLayer, p1, p2, wave, delay, down, horizon);

            if (result >= horizon) {
                bestDelay = delay;
                bestResult = result;
                break;
            }

            if (result > bestResult + ib::kMinImprovement) {
                bestDelay = delay;
                bestResult = result;
            }
        }

        if (bestDelay < 1) {
            st.handledDown = down;
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        log::info(
            "[InputBuffer] SAVED +{} ticks ({:.2f} frames @ 60fps) {} (Frame0: {} -> Delayed: {}) [Limit: {}]",
            bestDelay,
            bestDelay * ib::kTickFactor,
            down ? "PRESS" : "RELEASE",
            frame0,
            bestResult,
            limit
        );

        st.pendingDelay = bestDelay;
        st.lastBufferedDown = down;

        // The input is swallowed here, it is sent to the game later on.
    }

    void gameEventTriggered(GJGameEvent event, int p1, int p2) {
        if (ib::S().simulating) return;
        GJBaseGameLayer::gameEventTriggered(event, p1, p2);
    }

    void toggleDualMode(GameObject* object, bool dual, PlayerObject* player, bool noEffects) {
        if (ib::S().simulating) return;
        GJBaseGameLayer::toggleDualMode(object, dual, player, noEffects);
    }

    void toggleFlipped(bool p1, bool p2) {
        if (ib::S().simulating) return;
        GJBaseGameLayer::toggleFlipped(p1, p2);
    }
};

class $modify(IBPlayerObject, PlayerObject) {
    void updateTimeMod(float speed, bool noEffects) {
        // The time mod must not change while the trajectory is simulated.
        if (ib::S().simulating) return;
        PlayerObject::updateTimeMod(speed, noEffects);
    }

    void update(float dt) {
        auto& st = ib::S();

        if (st.simulating) {
            return PlayerObject::update(dt);
        }

        auto* playLayer = PlayLayer::get();
        if (!playLayer || playLayer->m_player1 != this) {
            return PlayerObject::update(dt);
        }

        // The trajectory is simulated with a quarter of the frame time, so one
        // tick is 1/240 s.
        st.simDt = dt * ib::kTickFactor;

        st.budget = std::min(st.budget, ib::kBudgetMax) + ib::kBudgetRefill;

        st.frameCounter++;
        if (st.frameCounter >= 30) {
            st.frameCounter = 0;
            if (st.failsafeCounter > 0) st.failsafeCounter--;
        }

        if (!st.cfg.enabled) {
            return PlayerObject::update(dt);
        }

        if (st.spamCounter <= 998) st.spamCounter++;

        bool dead = this->m_isDead;
        if (!dead && ib::isWave(playLayer) && playLayer->m_player2 && playLayer->m_player2->m_isDead) {
            dead = true;
        }

        if (dead) {
            st.pendingDelay = -1;
            st.releasePending = false;
            if (st.handledDown != st.rawDown) {
                ib::failsafeSync(playLayer);
            }
        } else if (st.pendingDelay == 0) {
            ib::fireButton(playLayer, st.lastBufferedDown);
            if (st.releasePending) {
                st.pendingDelay = -2;
                st.releasePending = false;
            } else {
                st.pendingDelay = -1;
            }
        } else if (st.pendingDelay > 0) {
            st.pendingDelay--;
        } else if (st.pendingDelay == -1) {
            ib::failsafeSync(playLayer);
        } else if (st.pendingDelay == -2) {
            ib::fireButton(playLayer, !st.lastBufferedDown);
            st.releasePending = false;
            st.pendingDelay = -1;
        }

        PlayerObject::update(dt);
    }
};

class $modify(IBEffectObject, EffectGameObject) {
    void triggerObject(GJBaseGameLayer* layer, int uniqueID, std::vector<int> const* remapKeys) {
        if (ib::S().simulating) return;
        EffectGameObject::triggerObject(layer, uniqueID, remapKeys);
    }
};

class $modify(IBGameObject, GameObject) {
    void playShineEffect() {
        auto& st = ib::S();
        auto* object = static_cast<GameObject*>(this);
        if (st.simulating && (object == st.ghost1 || object == st.ghost2)) return;
        GameObject::playShineEffect();
    }
};

class $modify(IBPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        ib::addPauseButton(this);
    }
};
