#ifdef GEODE_IS_WINDOWS

#include <Geode/Geode.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"
#include "discord_rpc.h"

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

enum class DiscordState {
    InMenu,
    InEditor,
    InLevel
};

class DiscordManager {
private:
    bool m_running = false;

    std::thread m_thread;
    int64_t rpcStartTime = 0;

    std::string m_state;
    std::string m_details;
    std::string m_smallKey;
    std::string m_smallText;

    DiscordManager() = default;

    std::string getDifficultyKey(GJGameLevel* m_lvl) {
        if (std::string(m_lvl->m_creatorName) == "Player") return "editor";

        int stars = m_lvl->m_stars;
        int difficulty = m_lvl->getAverageDifficulty();
        int levelID = m_lvl->m_levelID.value();

        if (levelID >= 1 && levelID <= 5004) {
            switch (levelID) {
                case 1: case 2: return "easy";
                case 3: case 4: return "normal";
                case 5: case 6: return "hard";
                case 7: case 8: case 9: return "harder";
                case 10: case 11: case 12: case 13: case 15: case 16: case 21: case 22: return "insane";
                case 17: case 19: return "harder";
                case 14: case 18: case 20: return "demon";
                case 5001: return "hard";
                case 5002: case 5003: case 5004: return "harder";
                case 3001: return "hard";
                default: break;
            }
        }

        if (stars == 0) {
            switch (difficulty) {
                case -1: return "auto";
                case 0: return "na";
                case 1: return "easy";
                case 2: return "normal";
                case 3: return "hard";
                case 4: return "harder";
                case 5: return "insane";
                case 6: return "demon";
                default: break;
            }
        } else if (stars == 10) {
            switch (difficulty) {
                case 1: return "easydemon";
                case 2: return "mediumdemon";
                case 3: return "demon";
                case 4: return "insanedemon";
                case 5: return "extremedemon";
                default: break;
            }
        } else {
            switch (stars) {
                case 1: return "auto";
                case 2: return "easy";
                case 3: return "normal";
                case 4: return "hard";
                case 5: case 6: return "harder";
                case 7: case 8: case 9: return "insane";
                default: break;
            }
        }
        return "na";
    }

    std::string getDifficultyName(const std::string& key) {
        if (key == "editor") return "Building a level";
        if (key == "auto") return "Auto";
        if (key == "easy") return "Easy";
        if (key == "normal") return "Normal";
        if (key == "hard") return "Hard";
        if (key == "harder") return "Harder";
        if (key == "insane") return "Insane";
        if (key == "demon") return "Hard Demon";
        if (key == "easydemon") return "Easy Demon";
        if (key == "mediumdemon") return "Medium Demon";
        if (key == "insanedemon") return "Insane Demon";
        if (key == "extremedemon") return "Extreme Demon";
        return "N/A";
    }

public:
    static DiscordManager& get() {
        static DiscordManager instance;
        return instance;
    }

    DiscordManager(const DiscordManager&) = delete;
    DiscordManager& operator=(const DiscordManager&) = delete;

    void start() {
        if (m_running) return;
        m_running = true;
        rpcStartTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        DiscordEventHandlers handlers;
        std::memset(&handlers, 0, sizeof(handlers));
        Discord_Initialize("1516801396471627887", &handlers, 1, nullptr);

        updatePresence(DiscordState::InMenu);

        m_thread = std::thread([this]() {
            while (m_running) {
                Discord_RunCallbacks();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });
    }

    void stop() {
        if (!m_running) return;
        m_running = false;
        if (m_thread.joinable()) m_thread.join();
        Discord_ClearPresence();
    }

    void updatePresence(DiscordState state, GJGameLevel* level = nullptr, int objectCount = 0) {
        DiscordRichPresence presence;
        std::memset(&presence, 0, sizeof(presence));
        presence.largeImageKey = "game_icon";
        presence.largeImageText = "Geometry Dash";
        presence.startTimestamp = rpcStartTime;

        static const std::unordered_set<int> robtop_levels = {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
            5001, 5002, 5003, 5004, 3001
        };

        if (state == DiscordState::InMenu) {
            m_state = "Browsing Menus";
            m_details = "";
            m_smallKey = "";
            m_smallText = "";
        } else if (state == DiscordState::InEditor) {
            m_state = fmt::format("In Editor ({} objects)", objectCount);
            m_details = level->m_levelName;
            m_smallKey = "editor";
            m_smallText = "Building a level";
        } else if (state == DiscordState::InLevel && level) {
            if (!level->isPlatformer())
                m_state = fmt::format("Playing a level ({}%)", level->getNormalPercent());
            else
                m_state = "Playing a level";

            m_smallKey = getDifficultyKey(level);
            m_smallText = getDifficultyName(m_smallKey);
            
            int levelId = level->m_levelID.value();
            if (levelId == 0) {
                m_details = fmt::format("{} - Playtesting", level->m_levelName);
            } else if (robtop_levels.contains(levelId)) {
                m_details = fmt::format("{} - RobTop", level->m_levelName);
            } else {
                std::string creator = level->m_creatorName.empty() ? "Unknown" : level->m_creatorName;
                m_details = fmt::format("{} - {} ({})", level->m_levelName, creator, levelId);
            }
        }

        presence.state = m_state.c_str();
        if (!m_details.empty()) presence.details = m_details.c_str();
        if (!m_smallKey.empty()) {
            presence.smallImageKey = m_smallKey.c_str();
            presence.smallImageText = m_smallText.c_str();
        }

        Discord_UpdatePresence(&presence);
    }
};

GUI_HACK_CREATE("Core", "Discord RPC", "", false);

class $modify(DiscordRPCPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& hack = GDH::Gui::get().getWindow("Core").findHackByName("Discord RPC");
        hack.setHandler([](bool enabled) { enabled ? DiscordManager::get().start() : DiscordManager::get().stop(); });

        hack.addHookPtr(self.getHook("PlayLayer::init").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::levelComplete").unwrap());
    }

    struct Fields {
        ~Fields() {
            DiscordManager::get().updatePresence(DiscordState::InMenu);
        }
    };

    void onUpdate(float) {
        DiscordManager::get().updatePresence(DiscordState::InLevel, m_level);
    }
    
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        m_fields.self();

        DiscordManager::get().updatePresence(DiscordState::InLevel, level);
        this->schedule(schedule_selector(DiscordRPCPlayLayer::onUpdate), 30.f);

        return true;
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        DiscordManager::get().updatePresence(DiscordState::InLevel, m_level);
    }
};

class $modify(DiscordRPCLevelEditorLayer, LevelEditorLayer) {
    static void onModify(auto& self) {
        auto& hack = GDH::Gui::get().getWindow("Core").findHackByName("Discord RPC");
        hack.addHookPtr(self.getHook("LevelEditorLayer::init").unwrap());
    }

    struct Fields {
        ~Fields() {
            DiscordManager::get().updatePresence(DiscordState::InMenu);
        }
    };

    void onUpdate(float) {
        DiscordManager::get().updatePresence(DiscordState::InEditor, m_level, m_objectCount);
    }

    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;
        m_fields.self();

        DiscordManager::get().updatePresence(DiscordState::InEditor, level, m_objectCount);
        this->schedule(schedule_selector(DiscordRPCLevelEditorLayer::onUpdate), 30.f);

        return true;
    }
};

#endif