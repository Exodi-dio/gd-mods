#include "state.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <fstream>
#include <sstream>

using namespace geode::prelude;

static ModelWeights g_weights;
static bool g_weights_loaded = false;
static bool g_agent_enabled = false;
static bool g_frame_step = false; // if true, pause after each frame
static int g_pending_action = 0; // 0 release, 1 tap
static float g_prev_progress = 0.f;
static bool g_episode_done = false;

// State file paths (for trainer IPC / debugging)
static std::filesystem::path statePath() {
    return Mod::get()->getSaveDir() / "agent_state.json";
}
static std::filesystem::path actionPath() {
    return Mod::get()->getSaveDir() / "agent_action.txt";
}

static void writeState(PlayLayer* pl, const Obs& obs, float reward, bool done) {
    std::ofstream f(statePath());
    if (!f) return;
    f << "{\n";
    f << "  \"player_y\": " << obs.player_y << ",\n";
    f << "  \"player_vy\": " << obs.player_vy << ",\n";
    f << "  \"on_ground\": " << obs.on_ground << ",\n";
    f << "  \"next_obs_x\": " << obs.next_obs_x << ",\n";
    f << "  \"next_obs_y\": " << obs.next_obs_y << ",\n";
    f << "  \"next_obs_h\": " << obs.next_obs_h << ",\n";
    f << "  \"next_obs_type\": " << obs.next_obs_type << ",\n";
    f << "  \"speed\": " << obs.speed << ",\n";
    f << "  \"progress\": " << pl->getCurrentPercent() << ",\n";
    f << "  \"reward\": " << reward << ",\n";
    f << "  \"done\": " << (done ? "true" : "false") << "\n";
    f << "}\n";
}

static int readAction() {
    std::ifstream f(actionPath());
    if (!f) return 0;
    int a = 0;
    f >> a;
    return a == 1 ? 1 : 0;
}

// Build observation from PlayLayer
static Obs buildObs(PlayLayer* pl) {
    Obs o;
    if (!pl || !pl->m_player1) return o;
    auto* p = pl->m_player1;
    o.player_y = p->getPositionY() / 320.f; // normalize roughly
    o.player_vy = p->m_yVelocity / 100.f; // rough
    o.on_ground = p->m_isOnGround ? 1.f : 0.f;
    o.speed = pl->m_gameState.m_levelTime > 0 ? 1.f : 1.f;

    // Find nearest obstacle ahead of player
    float px = p->getPositionX();
    float bestDist = 1e9f;
    float bestY = 0, bestH = 0, bestType = 0;
    if (pl->m_gameState.m_level) {
        // iterate objects via PlayLayer's object arrays if available
        // Fallback: leave defaults; trainer on sim doesn't need this
    }
    // Search children of the game layer for GameObjects ahead
    if (auto* gl = pl->m_gameLayer) {
        for (auto* child : CCArrayExt<CCNode*>(gl->getChildren())) {
            if (auto* go = typeinfo_cast<GameObject*>(child)) {
                float ox = go->getPositionX();
                if (ox > px && ox - px < bestDist && ox - px < 400.f) {
                    bestDist = ox - px;
                    bestY = go->getPositionY();
                    bestH = go->getContentSize().height;
                    bestType = (float)go->m_objectID;
                }
            }
        }
    }
    if (bestDist < 1e8f) {
        o.next_obs_x = bestDist / 400.f;
        o.next_obs_y = bestY / 320.f;
        o.next_obs_h = bestH / 100.f;
        o.next_obs_type = bestType / 1000.f;
    } else {
        o.next_obs_x = 1.f;
    }
    return o;
}

class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool b1) {
        if (!PlayLayer::init(level, b1)) return false;
        g_prev_progress = 0.f;
        g_episode_done = false;
        g_pending_action = 0;
        // Load weights once
        if (!g_weights_loaded) {
            auto wpath = Mod::get()->getSaveDir() / "agent_weights.bin";
            g_weights_loaded = loadWeights(g_weights, wpath);
            log::info("GD Agent: weights loaded = {}", g_weights_loaded);
        }
        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!g_agent_enabled) return;
        if (g_episode_done) return;

        Obs obs = buildObs(this);
        float progress = getCurrentPercent();
        float reward = (progress - g_prev_progress) * 100.f; // progress reward
        g_prev_progress = progress;

        // Apply pending action from trainer (if file present) OR run inference
        int action = 0;
        if (g_weights_loaded) {
            float p = infer(g_weights, obs);
            action = (p > 0.5f) ? 1 : 0;
        } else {
            action = readAction();
        }

        // Apply action
        if (action == 1) {
            if (m_player1 && !m_player1->m_isHolding) {
                handleButton(true, 1, false);
            }
        } else {
            if (m_player1 && m_player1->m_isHolding) {
                handleButton(false, 1, false);
            }
        }

        writeState(this, obs, reward, false);

        if (g_frame_step) {
            // pause the game to let trainer sync; trainer resumes via action file
            // (simple approach: don't auto-resume; trainer writes action then we continue next frame)
        }
    }

    void destroyPlayer(PlayerObject* p, GameObject* obj) {
        PlayLayer::destroyPlayer(p, obj);
        g_episode_done = true;
        Obs obs = buildObs(this);
        writeState(this, obs, -10.f, true);
        log::info("GD Agent: episode done (death) at {}%", getCurrentPercent());
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        g_episode_done = true;
        Obs obs = buildObs(this);
        writeState(this, obs, 100.f, true);
        log::info("GD Agent: level complete!");
    }
};

// Toggle agent on/off via a setting or keybind. For now, enable by default if weights exist.
$execute {
    geode::listenForSettingChanges<bool>("agent_enabled", +[](bool v) { g_agent_enabled = v; });
    geode::listenForSettingChanges<bool>("frame_step", +[](bool v) { g_frame_step = v; });
}
EOF
echo "written"