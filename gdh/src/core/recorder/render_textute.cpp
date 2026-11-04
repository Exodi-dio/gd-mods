#include "render_textute.hpp"
#include "recorder.hpp"

using namespace cocos2d;

RenderTexture::~RenderTexture() {
    cleanup();
}

bool RenderTexture::init(unsigned width, unsigned height) {
    if (m_initialized) {
        cleanup();
    }

    m_width = width;
    m_height = height;

    GLint old_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);

    m_texture = new CCTexture2D();
    if (!m_texture) return false;

    bool success = m_texture->initWithData(
        nullptr, 
        kCCTexture2DPixelFormat_RGBA8888, 
        m_width, 
        m_height, 
        CCSize(static_cast<float>(m_width), static_cast<float>(m_height))
    );

    if (!success) {
        m_texture->release();
        m_texture = nullptr;
        return false;
    }

    m_texture->setAliasTexParameters();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture->getName(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        geode::log::error("framebuffer status is incomplete!");
        glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
        cleanup();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
    m_initialized = true;

    return true;
}

void RenderTexture::cleanup() {
    if (!m_initialized) return;

    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_texture) {
        m_texture->release();
        m_texture = nullptr;
    }
    m_width = 0;
    m_height = 0;
    m_initialized = false;
}

void RenderTexture::capture(cocos2d::CCNode* target_node, std::mutex& lock, std::condition_variable& cv, std::vector<uint8_t>& data, bool& frame_has_data) {
    if (!m_initialized || !target_node) return;

    GLint old_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);

    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    target_node->visit();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    std::unique_lock<std::mutex> u_lock(lock);
    frame_has_data = true;
    
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    u_lock.unlock();
    cv.notify_one();

    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
    CCDirector::sharedDirector()->setViewport();
}