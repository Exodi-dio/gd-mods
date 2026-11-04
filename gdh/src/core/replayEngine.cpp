#include "replayEngine.hpp"
#include "config.hpp"
#include "gui.hpp"
#include "../interface/imgui/widgetH.hpp"
#include "labels.hpp"

using namespace GDH;

uint64_t ReplayEngine::get_frame_legacy() {
    auto gjbgl = GJBaseGameLayer::get();
    if (gjbgl) {
        auto& config = Config::get();
        return static_cast<uint64_t>(gjbgl->m_gameState.m_levelTime * config.get<float>("invisible.tps::value", 240.f));
    }
    return 0;
}

void ReplayEngine::remove_actions(uint64_t currentFrame, bool pause) {
    // clearing physic frames
    std::erase_if(m_physicFrames, [currentFrame](const physic_data &action) {
        // return action.frame >= currentFrame;
        return action.frame > currentFrame;
    });

    // clearing inputs and auto-releasing it
    std::erase_if(m_inputFrames, [currentFrame](const input_data &action) {
        // return action.frame >= currentFrame;
        return action.frame > currentFrame;
    });

    std::array<bool, 6> released = {true, true, true, true, true, true};
    std::array<bool, 6> checked = {false, false, false, false, false, false};

    for (auto it = m_inputFrames.rbegin(); it != m_inputFrames.rend(); ++it) {
        int button_index = it->button - 1 + (it->isPlayer2 ? 3 : 0);
        if (!checked[button_index]) {
            checked[button_index] = true;
            released[button_index] = !it->down;
            
            if (std::all_of(checked.begin(), checked.end(), [](bool v) { return v; }))
                break;
        }
    }

    if (auto gjbgl = GJBaseGameLayer::get()) {
        gjbgl->m_queuedButtons.clear();
        
        bool swapControls = GameManager::get()->getGameVariable(GameVar::Flip2PlayerControls);
        
        for (int i = 0; i < 3; ++i) {
            if (!released[i]) {
                customHandleButton(gjbgl, false, static_cast<PlayerButton>(i + 1), !swapControls);
                handle_button(false, i + 1, !swapControls);
            }
            if (!released[i + 3]) {
                customHandleButton(gjbgl, false, static_cast<PlayerButton>(i + 1), swapControls);
                handle_button(false, i + 1, swapControls);
            }
        }
        
        if (!pause) {
            auto isJumpButtonPressedForPlayer = [](UILayer* ui, bool player1) -> bool {
                if (player1) {
                    return (ui->m_p1TouchId != -1 || ui->m_p1Jumping);
                } else {
                    return (ui->m_p2TouchId != -1 || ui->m_p2Jumping);
                }
            };

            if (isJumpButtonPressedForPlayer(gjbgl->m_uiLayer, true)) {
                gjbgl->queueButton(1, true, false, 0.0);
            }
    
            if (isJumpButtonPressedForPlayer(gjbgl->m_uiLayer, false)) {
                gjbgl->queueButton(1, true, true, 0.0);
            }
        }
    }
}

void ReplayEngine::handle_update(GJBaseGameLayer* self) {  
    auto& config = Config::get();
    if (!config.get<bool>("engine::accuracy_fix", true)) return;

    auto frame = get_frame();

    if (mode == state::record) {
        bool frameExist = !m_physicFrames.empty() && m_physicFrames.back().frame == frame;
        
        if (!frameExist) {
            auto recordPlayerFrame = [this, frame](auto* player, bool isSecondPlayer) {
                m_physicFrames.push_back({
                    frame, 
                    player->getPositionX(),
                    player->getPositionY(),
                    player->m_yVelocity,
                    isSecondPlayer
                });
            };

            recordPlayerFrame(self->m_player1, false);
            if (self->m_gameState.m_isDualMode)
                recordPlayerFrame(self->m_player2, true);
        }
    }
    else if (mode == state::play || mode == state::continue_mode) {
        while (m_physicIndex < m_physicFrames.size() && frame >= m_physicFrames[m_physicIndex].frame)
        {
            const auto& physicFrame = m_physicFrames[m_physicIndex];
            auto* player = physicFrame.isPlayer2 ? self->m_player2 : self->m_player1;

            player->setPosition({physicFrame.x, physicFrame.y});

            if (config.get<bool>("engine::velocity_fix", false))
                player->m_yVelocity = physicFrame.y_accel;

            m_physicIndex++;
        }
    }
}

void ReplayEngine::handle_commands(GJBaseGameLayer* self) {
    auto frame = get_frame();
    auto& cps = CpsCounter::get();

    if (mode == state::continue_mode) {
        bool reachedEnd = (m_inputIndex >= m_inputFrames.size());
        
        if (reachedEnd || (!m_inputFrames.empty() && frame > m_inputFrames.back().frame)) {
            std::erase_if(m_physicFrames, [frame](const physic_data& action) {
                return action.frame >= frame;
            });

            mode = state::record;

            #ifdef GEODE_IS_DESKTOP
            ImGuiH::AddPopup("Switched to Record mode. Continue recording!");
            #endif
            return;
        }
    }

    if (mode != state::play && mode != state::continue_mode) return;
    
    while (m_inputIndex < m_inputFrames.size() && frame >= m_inputFrames[m_inputIndex].frame)
    {
        bool isPlayer2 = m_inputFrames[m_inputIndex].isPlayer2;
        bool isPlayer1 = !isPlayer2;

        bool swapControls = GameManager::get()->getGameVariable(GameVar::Flip2PlayerControls);

        if (swapControls) {
            isPlayer1 = !isPlayer1;
        }

        #ifdef GEODE_IS_ANDROID
        self->m_allowedButtons.clear();
        #endif

        cps.click(m_inputFrames[m_inputIndex].down, !m_inputFrames[m_inputIndex].isPlayer2);
        
        customHandleButton(self, m_inputFrames[m_inputIndex].down, static_cast<PlayerButton>(m_inputFrames[m_inputIndex].button), isPlayer1);
        m_inputIndex++;
    }
}

void ReplayEngine::handle_reset(bool pause) {
    if (mode == state::record) {
        int lastCheckpointFrame = get_frame();
        remove_actions(lastCheckpointFrame, pause);
    }
    else if (mode == state::play || mode == state::continue_mode) {
        m_physicIndex = 0;
        m_inputIndex = 0;
    }
}

void ReplayEngine::handle_button(bool down, int button, bool isPlayer1) {
    if (mode != state::record)
        return;

    bool swapControls = GameManager::get()->getGameVariable(GameVar::Flip2PlayerControls);

    if (swapControls) {
        isPlayer1 = !isPlayer1;
    }

    auto frame = get_frame();
    bool isPlayer2 = !isPlayer1;

    m_inputFrames.push_back({frame, down, button, isPlayer2});
}

size_t ReplayEngine::get_actions_size() {
    return m_inputFrames.size();
}

size_t ReplayEngine::get_current_index() {
    return m_inputIndex;
}

std::string ReplayEngine::save(const std::string& replay_name) {
    if (replay_name.empty())
        return "Empty macro name is not allowed";

    if (m_inputFrames.empty())
        return "Replay doesn't have actions";

    std::ofstream out(getFolderMacroPath() / std::string(replay_name + ".re4"), std::ios::binary);
    if (!out.is_open()) return "Failed to save Replay";

    out.write("RE4", 3);
    
    auto& config = Config::get();
    float fps = config.get<float>("invisible.tps::value", 240.f);    
    out.write(reinterpret_cast<const char*>(&fps), sizeof(fps));

    uint64_t physicSize = m_physicFrames.size();
    out.write(reinterpret_cast<const char*>(&physicSize), sizeof(physicSize));
    
    for (const auto& p : m_physicFrames) {
        out.write(reinterpret_cast<const char*>(&p.frame), sizeof(p.frame));
        out.write(reinterpret_cast<const char*>(&p.x), sizeof(p.x));
        out.write(reinterpret_cast<const char*>(&p.y), sizeof(p.y));
        out.write(reinterpret_cast<const char*>(&p.y_accel), sizeof(p.y_accel));
        
        uint8_t p2 = p.isPlayer2 ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&p2), 1);
    }

    uint64_t inputSize = m_inputFrames.size();
    out.write(reinterpret_cast<const char*>(&inputSize), sizeof(inputSize));

    for (const auto& i : m_inputFrames) {
        out.write(reinterpret_cast<const char*>(&i.frame), sizeof(i.frame));
        
        uint8_t down = i.down ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&down), 1);
        
        int32_t btn = static_cast<int32_t>(i.button);
        out.write(reinterpret_cast<const char*>(&btn), sizeof(btn));
        
        uint8_t p2 = i.isPlayer2 ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&p2), 1);
    }

    return "Replay Saved";
}

std::string ReplayEngine::load(const std::string& replay_name) {

    std::ifstream in(getFolderMacroPath() / std::string(replay_name + ".re4"), std::ios::binary);
    if (!in.is_open()) return "Failed to open Replay";

    m_physicFrames.clear();
    m_inputFrames.clear();

    char header[3];
    in.read(header, 3);
    if (header[0] != 'R' || header[1] != 'E' || header[2] != '4') {
        return "Invalid Replay format";
    }

    float tps;
    in.read(reinterpret_cast<char*>(&tps), sizeof(tps));

    auto& config = Config::get();
    config.set<float>("invisible.tps::value", tps);
    config.set<float>("invisible.lock_delta::value", tps);

    uint64_t physicSize = 0;
    in.read(reinterpret_cast<char*>(&physicSize), sizeof(physicSize));
    m_physicFrames.resize(physicSize);

    for (auto& p : m_physicFrames) {
        in.read(reinterpret_cast<char*>(&p.frame), sizeof(p.frame));
        in.read(reinterpret_cast<char*>(&p.x), sizeof(p.x));
        in.read(reinterpret_cast<char*>(&p.y), sizeof(p.y));
        in.read(reinterpret_cast<char*>(&p.y_accel), sizeof(p.y_accel));
        
        uint8_t p2 = 0;
        in.read(reinterpret_cast<char*>(&p2), 1);
        p.isPlayer2 = (p2 != 0);
    }

    uint64_t inputSize = 0;
    in.read(reinterpret_cast<char*>(&inputSize), sizeof(inputSize));
    m_inputFrames.resize(inputSize);

    for (auto& i : m_inputFrames) {
        in.read(reinterpret_cast<char*>(&i.frame), sizeof(i.frame));
        
        uint8_t down = 0;
        in.read(reinterpret_cast<char*>(&down), 1);
        i.down = (down != 0);
        
        int32_t btn = 0;
        in.read(reinterpret_cast<char*>(&btn), sizeof(btn));
        i.button = static_cast<int>(btn);
        
        uint8_t p2 = 0;
        in.read(reinterpret_cast<char*>(&p2), 1);
        i.isPlayer2 = (p2 != 0);
    }

    return "Replay Loaded";
}

void ReplayEngine::clear() {
    m_physicFrames.clear();
    m_inputFrames.clear();
}

void ReplayEngine::customHandleButton(GJBaseGameLayer* self, bool down, PlayerButton button, bool player1) {
    auto buttonAction = down ? &PlayerObject::pushButton : &PlayerObject::releaseButton;

    if (GameManager::get()->getGameVariable(GameVar::Flip2PlayerControls)) {
        player1 = !player1;
    }

    if (self->m_levelSettings->m_twoPlayerMode) {
        PlayerObject* player = player1 ? self->m_player1 : self->m_player2;
        (player->*buttonAction)(button);
    } else {
        (self->m_player1->*buttonAction)(button);
        if (self->m_gameState.m_isDualMode) {
            (self->m_player2->*buttonAction)(button);
        }
    }

    self->m_effectManager->playerButton(down, player1);

    if (down) {
        self->m_clicks++;
        if (button == PlayerButton::Jump) {
            self->m_jumping = true;
        }
    }
}

void ReplayEngine::setupHacks(state newMode) {
    auto& gui = GDH::Gui::get();

    struct HackTarget {
        std::string window;
        std::string name;
        bool targetStateForRecord;
        bool targetStateForPlay;
    };

    std::vector<HackTarget> targets = {
        {"Invisible", "TPS", true, true},
        {"Level", "Click Between Steps", false, false},
        {"Level", "Click On Steps", false, false},
        {"Level", "Click Between Frames", false, false},
        {"Level", "Practice Fix",  true, false},
        {"Level", "Respawn Lag Fix", true, false},
        {"Level", "Frame Extrapolation", false, false},
        {"Cosmetic", "No Mirror", true, false}
    };

    if (newMode == state::disable) {
        if (!m_savedHackStates.empty()) {
            for (const auto& target : targets) {
                std::string key = std::string(target.window) + "::" + target.name;
                if (m_savedHackStates.contains(key)) {
                    auto& hack = gui.getWindow(target.window).findHackByName(target.name);
                    bool originalState = m_savedHackStates[key];
                    originalState ? hack.enable() : hack.disable();
                }
            }
            m_savedHackStates.clear();
        }
        return;
    }

    if (m_savedHackStates.empty()) {
        for (const auto& target : targets) {
            std::string key = std::string(target.window) + "::" + target.name;
            auto& hack = gui.getWindow(target.window).findHackByName(target.name);
            m_savedHackStates[key] = hack.getEnabled();
        }
    }

    for (const auto& target : targets) {
        auto& hack = gui.getWindow(target.window).findHackByName(target.name);
        bool shouldEnable = (newMode == state::record || newMode == state::continue_mode) ? target.targetStateForRecord : target.targetStateForPlay;
        shouldEnable ? hack.enable() : hack.disable();
    }
}