#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <mutex>
#include "../ringBuffer.hpp"

class LogOverlay {
public:
    static LogOverlay& get() {
        static LogOverlay instance;
        return instance;
    }

    void create() {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_buffer.init(15);
        }

        geode::queueInMainThread([]() {
            auto overlay = geode::OverlayManager::get();
            if (!overlay || overlay->getChildByID("h264_logs"_spr)) return;

            auto label = cocos2d::CCLabelBMFont::create("", "GoogleSans.fnt"_spr);
            label->setScale(0.40f);
            label->setAnchorPoint({0.0f, 0.0f});
            label->setPosition({3.0f, 3.0f});
            label->setOpacity(180);
            label->setID("h264_logs"_spr);
            overlay->addChild(label);
        });
    }

    void destroy() {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_buffer.clear();
        }
        geode::queueInMainThread([]() {
            if (auto overlay = geode::OverlayManager::get()) {
                if (auto label = overlay->getChildByID("h264_logs"_spr)) {
                    label->removeFromParent();
                }
            }
        });
    }

    void add_log(const std::string& line) {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_buffer.push(line);
        }

        geode::queueInMainThread([this]() {
            auto overlay = geode::OverlayManager::get();
            if (!overlay) return;
            
            if (auto label = static_cast<cocos2d::CCLabelBMFont*>(overlay->getChildByID("h264_logs"_spr))) {
                std::string full_text;
                {
                    std::lock_guard<std::mutex> lk(m_mutex);
                    m_buffer.for_each([&full_text](const std::string& item) {
                        if (!full_text.empty()) full_text += "\n";
                        full_text += item;
                    });
                }
                label->setString(full_text.c_str());
            }
        });
    }

private:
    RingBuffer<std::string> m_buffer;
    std::mutex m_mutex;
};