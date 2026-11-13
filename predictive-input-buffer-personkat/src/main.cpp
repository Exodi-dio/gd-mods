// Predictive Input Buffer - recreation based on the console log of v4.7
//
// Instead of applying every input (press/release) instantly, the input is queued
// and applied N physics ticks later. N depends on the active stage and on whether
// the player is in wave mode. F6 cycles: STAGE 1 -> 2 -> 3 -> 4 -> OFF.
//
// 1 tick = 1/240 s  (4 ticks = 1 frame @ 60fps, same as the original log)

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <deque>

using namespace geode::prelude;

namespace pib {
    struct Stage { int all; int wave; };
    constexpr Stage STAGES[4] = { {2, 2}, {4, 4}, {8, 8}, {16, 8} };
    constexpr int OFF = 4; // index 4 means the buffer is disabled

    inline int g_stage = 0; // starts on STAGE 1

    inline bool verbose() {
        return Mod::get()->getSettingValue<bool>("verbose-log");
    }

    inline void logStage() {
        if (g_stage == OFF) {
            log::info("[InputBuffer] F6 Cycle -> STAGE 1: OFF (100% Raw Input)");
            return;
        }
        auto s = STAGES[g_stage];
        log::info(
            "[InputBuffer] F6 Cycle -> STAGE {}: {} Ticks All | {} Ticks Wave ({:.1f}f @ 60fps)",
            g_stage + 1, s.all, s.wave, s.all / 4.0f
        );
    }
}

struct PendingInput {
    bool down;
    int button;
    bool player1;
    int remaining; // ticks left before the input is applied
};

namespace pib {
    inline std::deque<PendingInput> g_queue;
    inline bool g_applying = false;
}

class $modify(PIBBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        // Pass through when the buffer is off or when we are re-injecting an input
        if (pib::g_applying || pib::g_stage == pib::OFF) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto* player = isPlayer1 ? m_player1 : m_player2;
        bool isWave = player && player->m_isDart;
        auto stage = pib::STAGES[pib::g_stage];
        int delay = isWave ? stage.wave : stage.all;

        pib::g_queue.push_back({ down, button, isPlayer1, delay });

        if (pib::verbose()) {
            log::info(
                "*** BUFFERED +{} TICKS ({:.2f} frames @ 60fps) {}! [Limit: {}] ***",
                delay, delay / 4.0f, down ? "PRESS" : "RELEASE", delay
            );
        }
    }

    // Runs once per physics step: this is where the tick countdown happens.
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        for (auto it = pib::g_queue.begin(); it != pib::g_queue.end();) {
            it->remaining -= 1;
            if (it->remaining <= 0) {
                pib::g_applying = true;
                GJBaseGameLayer::handleButton(it->down, it->button, it->player1);
                pib::g_applying = false;
                it = pib::g_queue.erase(it);
            } else {
                ++it;
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(PIBPlayLayer, PlayLayer) {
    void resetLevel() {
        // Clear the queue on restart so inputs do not leak between attempts
        pib::g_queue.clear();
        PlayLayer::resetLevel();
    }
};

class $modify(PIBKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_F6) {
            pib::g_stage = (pib::g_stage + 1) % 5;
            pib::logStage();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};

$on_mod(Loaded) {
    log::info("[InputBuffer] v4.7 LOADED (Zero-Lag + Up Arrow Fix)");
    log::info("[InputBuffer] Starting Mode: STAGE 1 (2 All / 2 Wave)");
    log::info("[InputBuffer] Press F6 in-level to cycle: 2/2 -> 4/4 -> 8/8 -> 16/8 -> OFF");
}