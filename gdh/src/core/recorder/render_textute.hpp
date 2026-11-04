#pragma once
#include <Geode/Geode.hpp>
#include <mutex>
#include <vector>
#include <condition_variable>

class RenderTexture {
private:
    unsigned m_width = 0;
    unsigned m_height = 0;
    unsigned m_fbo = 0;
    cocos2d::CCTexture2D* m_texture = nullptr;
    bool m_initialized = false;

public:
    RenderTexture() = default;
    ~RenderTexture();

    bool init(unsigned width, unsigned height);
    void cleanup();

    void capture(cocos2d::CCNode* target_node, std::mutex& lock, std::condition_variable& cv, std::vector<uint8_t>& data, bool& frame_has_data);
};