#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include "../../core/config.hpp"
#include "../../core/replayEngine.hpp"
#include "../../core/gui.hpp"
#include "../../core/keybinds.hpp"
#include "../../core/utils.hpp"
#include "../../interface/imgui/widgetH.hpp"

static float left_over = 0.f;

#if defined(GEODE_IS_WINDOWS) || defined (GEODE_IS_ANDROID64)
static int expectedTicks = 0;
#endif

#if defined (GEODE_IS_MACOS)
static float expectedTicks = 0;
#endif

static uintptr_t offset = 0;
static uintptr_t deltaOffset = 0;

#ifdef GEODE_IS_WINDOWS 
$execute {
    offset = GDH::Utils::PatternScan(geode::base::get(), 0x250000, "F3 0F 10 86 ? ? ? ? F3 44 0F 10 1D");
}
#endif

#ifdef GEODE_IS_ANDROID64
$execute {
    offset = GDH::Utils::PatternScan(geode::base::get(), 0x900000, "AB 19 60 1E 0A 10 62 1E 6A 09 6A 1E");
}
#endif

#ifdef GEODE_IS_INTEL_MAC
$execute {
    offset = GDH::Utils::PatternScan(geode::base::get(), 0x1600000, "0F 28 ? F3 0F 5D 83 ? ? ? ? F3 0F ? ? F2 0F 10");
    deltaOffset = GDH::Utils::PatternScan(geode::base::get(), 0x1600000, "11 11 11 11 11 11 71 3F");
}
#endif

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64) || defined(GEODE_IS_INTEL_MAC)
class $modify(ReplayCCScheduler, cocos2d::CCScheduler) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Invisible").findHackByName("TPS");   
        hack.setCheating(true);     
        hack.setEarlyInit(false);

        auto &config = Config::get();
        float value = config.get<float>("invisible.tps::value", 240.f); 
        config.set<float>("invisible.tps::value", std::clamp(value, 10.f, 240.f));

        hack.setHandler([](bool enabled) {
            if (offset == 0) {
                geode::log::error("Couldn't find the offset for the TPS bypass");
                return;
            }

            #ifdef GEODE_IS_WINDOWS 
            auto expectedTicksAddr = (uintptr_t)&expectedTicks;
            std::vector<uint8_t> patchBytes;
            
            // mov rax, expectedTicks (48 B8 + 8 byte address)
            patchBytes.push_back(0x48);
            patchBytes.push_back(0xB8);
            for (int i = 0; i < 8; i ++) {
                patchBytes.push_back((expectedTicksAddr >> (i * 8)) & 0xFF);
            }

            // mov r11d, [rax] (44 8B 18)
            patchBytes.push_back(0x44);
            patchBytes.push_back(0x8B);
            patchBytes.push_back(0x18);

            for (int i = 0; i < 54; i++) {
                patchBytes.push_back(0x90); // NOP
            }
            #endif

            #ifdef GEODE_IS_ANDROID64
            std::vector<uint8_t> patchBytes;

            auto push_u32 = [&patchBytes](uint32_t insn) {
                patchBytes.push_back(insn & 0xFF);
                patchBytes.push_back((insn >> 8) & 0xFF);
                patchBytes.push_back((insn >> 16) & 0xFF);
                patchBytes.push_back((insn >> 24) & 0xFF);
            };

            uint64_t addr = reinterpret_cast<uint64_t>(&expectedTicks);
            uint16_t u0 = (addr >> 0)  & 0xFFFF;
            uint16_t u1 = (addr >> 16) & 0xFFFF;
            uint16_t u2 = (addr >> 32) & 0xFFFF;
            uint16_t u3 = (addr >> 48) & 0xFFFF;

            // mov x9, &expectedTicks
            push_u32(0xD2800009 | (u0 << 5));
            push_u32(0xF2A00009 | (u1 << 5));
            push_u32(0xF2C00009 | (u2 << 5));
            push_u32(0xF2E00009 | (u3 << 5));

            // ldr w0, [x9]
            push_u32(0xB9400120);

            // b +0x10 
            push_u32(0x14000004);
            #endif

            #ifdef GEODE_IS_INTEL_MAC
            auto expectedTicksAddr = reinterpret_cast<uintptr_t>(&expectedTicks);
            std::vector<uint8_t> patchBytes;

            // movabs rax, expectedTicksAddr
            patchBytes.push_back(0x48);
            patchBytes.push_back(0xB8);
            for (int i = 0; i < 8; i++) {
                patchBytes.push_back((expectedTicksAddr >> (i * 8)) & 0xFF);
            }

            // movss xmm0, dword ptr [rax]
            patchBytes.push_back(0xF3);
            patchBytes.push_back(0x0F);
            patchBytes.push_back(0x10);
            patchBytes.push_back(0x00);

            // jmp rel32 (0x36)
            patchBytes.push_back(0xE9);
            int32_t rel = 0x36;
            patchBytes.push_back(rel & 0xFF);
            patchBytes.push_back((rel >> 8) & 0xFF);
            patchBytes.push_back((rel >> 16) & 0xFF);
            patchBytes.push_back((rel >> 24) & 0xFF);

            // nop padding
            for (int i = 0; i < 54; i++) {
                patchBytes.push_back(0x90);
            }
            #endif

            static auto result = geode::Mod::get()->patch((void*)(offset), patchBytes);
            static auto patch = result.isErr() ? nullptr : result.unwrap();
            
            if (patch) {
                (void)patch->toggle(enabled);
            }

            #ifdef GEODE_IS_INTEL_MAC
            if (deltaOffset != 0) {
                static geode::Patch* deltaPatch = nullptr;

                if (enabled) {
                    auto& config = Config::get();
                    float tps = config.get<float>("invisible.tps::value", 240.f);
                    double value = 1.0 / static_cast<double>(tps);

                    std::vector<uint8_t> deltaBytes(sizeof(double));
                    std::memcpy(deltaBytes.data(), &value, sizeof(double));

                    if (!deltaPatch) {
                        auto dRes = geode::Mod::get()->patch(reinterpret_cast<void*>(deltaOffset), deltaBytes);
                        if (dRes.isOk()) {
                            deltaPatch = dRes.unwrap();
                            (void)deltaPatch->enable();
                        } else {
                            geode::log::error("failed to patch delta for intel mac: {}", dRes.unwrapErr());
                        }
                    } else {
                        (void)deltaPatch->updateBytes(deltaBytes);
                        (void)deltaPatch->enable();
                    }
                } else if (deltaPatch) {
                    (void)deltaPatch->disable();
                }
            } else {
                geode::log::error("couldn't find the deltaOffset for intel mac");
            }
            #endif
        });
    }
};
#endif

#if defined(GEODE_IS_ANDROID32) || defined(GEODE_IS_ARM_MAC)
#include "../../core/recorder/recorder.hpp"

class $modify(ReplayCCScheduler, cocos2d::CCScheduler) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Invisible").findHackByName("TPS");   
        hack.setCheating(true);
        hack.addHookPtr(self.getHook("cocos2d::CCScheduler::update").unwrap());
    }

    void update(float dt) {
        auto &engine = GDH::ReplayEngine::get();
        auto &config = Config::get();
        auto& recorder = GDH::Recorder::get();

        if (recorder.isRecording()) {
            return CCScheduler::update(dt);
        }

        float tps_value = config.get<float>("invisible.tps::value", 240.f);
        float new_dt = 1.f / tps_value;

        unsigned times = static_cast<int>((dt + left_over) / new_dt);  
        if (dt <= 0.f)
            return CCScheduler::update(dt);

        if (!config.get<bool>("invisible.lock_delta::real_time", true))
            return CCScheduler::update(new_dt);

        auto start = std::chrono::high_resolution_clock::now();
        using namespace std::literals;

        for (unsigned i = 0; i < times; ++i) {
            CCScheduler::update(new_dt);

            if (std::chrono::high_resolution_clock::now() - start > 33.333ms) {         
                times = i + 1;
                break;
            }
        }

        left_over = std::max(0.f, left_over + dt - new_dt * times);
    }
};

#endif

class $modify(ReplayGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        // TPS Bypass
        auto& gui = GDH::Gui::get();
        auto& tps = gui.getWindow("Invisible").findHackByName("TPS");       
        tps.setCheating(true); 
        tps.addHookPtr(self.getHook("GJBaseGameLayer::getModifiedDelta").unwrap());
        #if defined(GEODE_IS_INTEL_MAC)
        tps.addHookPtr(self.getHook("GJBaseGameLayer::update").unwrap());
        #endif
    }

    #if defined(GEODE_IS_INTEL_MAC)
    double calculateModifiedDelta(float dt) {
        if (m_resumeTimer > 0) {
            --m_resumeTimer;
            dt = 0.0f;
        }
        
        auto& config = Config::get();
        auto tps = config.get<float>("invisible.tps::value", 240.f);
        auto fixed_dt = 1.0 / static_cast<double>(tps);

        auto timestep = std::min(static_cast<double>(m_gameState.m_timeWarp), 1.0) * fixed_dt;
        auto total_dt = dt + m_extraDelta;
        auto steps = std::round(total_dt / timestep);
        auto new_dt = steps * timestep;

        if (new_dt > 0.0) {
            float v6 = static_cast<float>(new_dt) * tps;
            expectedTicks = std::max(1.f, std::round(v6 / std::min(m_gameState.m_timeWarp, 1.f)));
        }

        return new_dt;
    }

    void update(float dt) {
        calculateModifiedDelta(dt);
        GJBaseGameLayer::update(dt);
    }
    #endif

    double getModifiedDelta(float dt) {
        if (m_resumeTimer > 0)
        {
            --m_resumeTimer;
            dt = 0.0;
        }
        
        auto& config = Config::get();
        auto tps = config.get<float>("invisible.tps::value", 240.f);
        auto fixed_dt = 1.0 / static_cast<double>(tps);

        auto timestep = std::min(static_cast<double>(m_gameState.m_timeWarp), 1.0) * fixed_dt;
        auto total_dt = dt + m_extraDelta;
        auto steps = std::round(total_dt / timestep);
        auto new_dt = steps * timestep;
        m_extraDelta = total_dt - new_dt;

        #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64)
        if (new_dt > 0.0) {
            // float v6 = new_dt * 60.f;
            // expectedTicks = std::max(1.f, std::round(v6 / std::min(m_gameState.m_timeWarp, 1.f) * 4.f));

            float v6 = new_dt * tps;
            expectedTicks = std::max(1.f, std::round(v6 / std::min(m_gameState.m_timeWarp, 1.f)));
        }
        #endif

        return static_cast<double>(new_dt);
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        auto& engine = GDH::ReplayEngine::get();
        auto& config = Config::get();
        if (engine.mode == state::play && config.get<bool>("engine::ignore_inputs", true)) return;
        
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
        engine.handle_button(down, button, isPlayer1);

        if (config.get<bool>("level.dual_clicks", false) && m_level->m_twoPlayerMode && m_gameState.m_isDualMode) {
            bool invert = config.get<bool>("level.dual_clicks::invert_click", false);
            
            if (engine.mode == state::record) {
                engine.customHandleButton(this, invert ? !down : down, static_cast<PlayerButton>(button), !isPlayer1);
                engine.handle_button(invert ? !down : down, button, !isPlayer1);
            }
            else {
                GJBaseGameLayer::handleButton(invert ? !down : down, button, !isPlayer1);
            }
        }
    }
    
    // void processCommands(float dt, bool isHalfTick, bool isLastTick) {
    //     GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    // }
        
    void processQueuedButtons(float dt, bool clearInputQueue) {
        auto &engine = GDH::ReplayEngine::get();
        engine.inc_frame();
        
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);
        
        if (!m_levelEndAnimationStarted) {
            engine.handle_commands(this);
            engine.handle_update(this);
        }
    }
};

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto& engine = GDH::ReplayEngine::get();
        if (engine.get_actions_size() == 0) {
            engine.replay_name = fmt::format("{}", level->m_levelName);
        }

        return true;
    }
};

class $modify(EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

        auto& engine = GDH::ReplayEngine::get();
        if (engine.get_actions_size() == 0) {
            engine.replay_name = fmt::format("{}", level->m_levelName);
        }

        return true;
    }
};

class $modify(ReplayPauseLayer, PauseLayer) {
    void keyDown(cocos2d::enumKeyCodes key, double timestamp) {
        auto& engine = GDH::ReplayEngine::get();
        if (engine.mode == state::record && key == cocos2d::KEY_Space) {
            this->schedule(schedule_selector(ReplayPauseLayer::checkSpaceUp));
            return;
        }
        PauseLayer::keyDown(key, timestamp);
    }

    void checkSpaceUp(float dt) {
        auto& kb = GDH::Keybinds::get();
        
        if (!kb.isKeyDown(cocos2d::KEY_Space)) {
            this->unschedule(schedule_selector(ReplayPauseLayer::checkSpaceUp));
            this->onResume(nullptr);
        }
    }
};

class $modify(ReplayPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& tps = gui.getWindow("Invisible").findHackByName("TPS");       
        tps.addHookPtr(self.getHook("PlayLayer::updateProgressbar").unwrap());
        tps.addHookPtr(self.getHook("PlayLayer::destroyPlayer").unwrap());
        tps.addHookPtr(self.getHook("PlayLayer::levelComplete").unwrap());
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        
        auto& engine = GDH::ReplayEngine::get();
        if (engine.get_actions_size() == 0) {
            engine.replay_name = fmt::format("{}", level->m_levelName);
        }

        return true;
    }

    void resetLevel() override {
        auto &config = Config::get();
        auto &engine = GDH::ReplayEngine::get();

        left_over = 0;
        engine.set_frame(0);

        PlayLayer::resetLevel();
        
        engine.handle_reset();  

        if (engine.mode == state::continue_mode && !m_isPracticeMode) {
            togglePracticeMode(true);
            #ifdef GEODE_IS_DESKTOP
            ImGuiH::AddPopup("Continue Feature: Switching to practice mode. Don't forget to set checkpoints!");
            #endif
        }
    }

    void playEndAnimationToPos(cocos2d::CCPoint pos) {
        PlayLayer::playEndAnimationToPos(pos);
        geode::queueInMainThread([]() {
            auto& engine = GDH::ReplayEngine::get();
            if (engine.mode == state::record) {
                engine.mode = state::disable;
                engine.setupHacks(state::disable);
            }
        });
    }
    
    void playPlatformerEndAnimationToPos(cocos2d::CCPoint pos, bool instant) {
        PlayLayer::playPlatformerEndAnimationToPos(pos, instant);
        geode::queueInMainThread([]() {
            auto& engine = GDH::ReplayEngine::get();
            if (engine.mode == state::record) {
                engine.mode = state::disable;
                engine.setupHacks(state::disable);
            }
        });
    }

    void pauseGame(bool unfocused) {
        if (!this->canPauseGame() || AppDelegate::get()->m_gamePaused) {
            return PlayLayer::pauseGame(unfocused);
        }

        auto& engine = GDH::ReplayEngine::get();
        if (engine.mode == state::record) engine.handle_reset(true);

        PlayLayer::pauseGame(unfocused);
    }

    int recalculateProgress() {
        auto& config = Config::get();
        float value = config.get<float>("invisible.tps::value", 240.f);

        auto timestamp = m_level->m_timestamp;
        auto currentProgress = m_gameState.m_currentProgress;

        if (timestamp > 0 && value != 240.f) {
            double progress = GDH::Utils::getRealProgress(this);
            m_gameState.m_currentProgress = (timestamp * progress * 0.02);
        }
        return currentProgress;
    }

    void updateProgressbar() {
        auto recurrentProgress = recalculateProgress();
        PlayLayer::updateProgressbar();
        m_gameState.m_currentProgress = recurrentProgress;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) override {
        auto recurrentProgress = recalculateProgress();
        PlayLayer::destroyPlayer(player, object);
        m_gameState.m_currentProgress = recurrentProgress;
    }

    void levelComplete() {
        auto& config = Config::get();
        float value = config.get<float>("invisible.tps::value", 240.f);

        auto old_timestamp = m_gameState.m_commandIndex;
        if (value != 240.f) {
            auto ticks = static_cast<unsigned int>(std::round(m_gameState.m_levelTime * 480));
            m_gameState.m_commandIndex = ticks;
        }
        PlayLayer::levelComplete();
        m_gameState.m_commandIndex = old_timestamp;
    }
};

class $modify(ReplayLevelEditorLayer, LevelEditorLayer) {
    void onPlaytest() {
        GDH::ReplayEngine::get().handle_reset();
        LevelEditorLayer::onPlaytest();
    }
};