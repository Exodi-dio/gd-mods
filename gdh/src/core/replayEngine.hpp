#pragma once
#include <Geode/Geode.hpp>

enum state { disable, record, play, continue_mode };

struct physic_data
{
    uint64_t frame;
    float x;
    float y;
    double y_accel;
    bool isPlayer2;
};

struct input_data
{
    uint64_t frame;
    bool down;
    int button;
    bool isPlayer2;
};
    
namespace GDH {
    class ReplayEngine {
    public:
        static ReplayEngine& get() {
            static ReplayEngine instance; 
            return instance;
        }

        ReplayEngine& operator=(const ReplayEngine&) = delete; 
        ReplayEngine(const ReplayEngine&) = delete;

        state mode = state::disable;
        std::string replay_name;

        uint64_t get_frame() { return m_currentFrame; }
        uint64_t get_frame_legacy();

        void set_frame(uint64_t frame) { m_currentFrame = frame; }
        void inc_frame() { m_currentFrame++; }
        void remove_actions(uint64_t currentFrame, bool pause = false);

        size_t get_actions_size();
        size_t get_current_index();

        std::string save(const std::string& replay_name);
        std::string load(const std::string& replay_name);
        void clear();

        void handle_update(GJBaseGameLayer* self);
        void handle_commands(GJBaseGameLayer* self);

        void handle_reset(bool pause = false);

        void handle_button(bool down, int button, bool isPlayer1);

        std::vector<physic_data>& get_physic_frames() { return m_physicFrames; }
        std::vector<input_data>& get_input_frames() { return m_inputFrames; }

        void customHandleButton(GJBaseGameLayer* self, bool down, PlayerButton button, bool player1);
        void setupHacks(state newMode);
    private:
        ReplayEngine() = default;

        std::vector<physic_data> m_physicFrames;
        size_t m_physicIndex = 0;

        std::vector<input_data> m_inputFrames;
        size_t m_inputIndex = 0;

        std::unordered_map<std::string, bool> m_savedHackStates;

        uint64_t m_currentFrame = 0;
    };
}