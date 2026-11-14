#pragma once

#include <Geode/Geode.hpp>

#include <string>

namespace ib {

    // Runtime configuration, mirrored from the mod settings.
    struct Config {
        bool enabled = true;
        int normalTicks = 2;
        int waveTicks = 2;
        bool spamGuard = true;
        int spamInterval = 20;
        bool noclip = false;
        float gameSpeed = 1.f;
        int autoClose = 15;
        std::string buttonCorner = "Bottom right";
        float buttonScale = 0.7f;
    };

    struct State {
        Config cfg;

        // Raw input state.
        bool rawDown = false;
        bool handledDown = false;

        // Buffered input state machine.
        // -1: idle, -2: a release is pending, >= 0: ticks left before the input is fired.
        int pendingDelay = -1;
        bool lastBufferedDown = false;
        bool releasePending = false;

        // Spam guard.
        int spamCounter = 999;
        int rapidClicks = 0;

        // Failsafe synchronisation.
        int failsafeCounter = 0;
        int frameCounter = 0;

        // Game speed that is currently applied to the player (practice mode).
        float gameSpeedApplied = 0.f;
        PlayerObject* gameSpeedPlayer = nullptr;

        // Trajectory simulation.
        bool simulating = false;
        bool injecting = false;
        bool abortSim = false;
        bool ghostDied = false;
        int budget = 700;
        float simDt = 1.f / 240.f;
        cocos2d::CCLayer* holder = nullptr;
        PlayerObject* ghost1 = nullptr;
        PlayerObject* ghost2 = nullptr;
    };

    State& S();

    void applySettings();

    // Reads the settings into the runtime config without logging.
    void syncConfig();

    void createGhosts(PlayLayer* playLayer);
    void destroyGhosts();
    void resetRuntime();

    // Simulates the trajectory of the player and returns how many ticks it survives
    // after `base` ticks have been simulated with the requested button state.
    int getSurvivalTicks(
        PlayLayer* playLayer,
        PlayerObject* p1,
        PlayerObject* p2,
        bool wave,
        int base,
        bool down,
        int maxTicks
    );

    // Sends a button state to the game without the buffer touching it.
    void fireButton(PlayLayer* playLayer, bool down);

    // Re-sends the raw input when the game state and the raw input disagree.
    void failsafeSync(PlayLayer* playLayer);

    // True while the game is in wave mode (dual mode with a second player).
    bool isWave(PlayLayer* playLayer);

    // Opens / refreshes the Buffer menu.
    void openMenu();
    void refreshMenu();

    // Adds the "Buffer" button to the pause menu.
    void addPauseButton(PauseLayer*);

}
