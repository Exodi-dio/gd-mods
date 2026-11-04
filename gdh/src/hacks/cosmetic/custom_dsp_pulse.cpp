#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <imgui-cocos.hpp>
#include <fmod.hpp>
#include <atomic>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include "../../core/config.hpp"
#include "../../core/gui.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

class AudioAnalyser {
public:
    static AudioAnalyser& get() {
        static AudioAnalyser instance;
        return instance;
    }

    void init() {
        if (m_dsp) return;

        auto fmod = FMODAudioEngine::sharedEngine();
        if (!fmod || !fmod->m_system) return;

        FMOD_DSP_DESCRIPTION desc = {};
        std::snprintf(desc.name, sizeof(desc.name), "GDH Audio Analyser");
        desc.numinputbuffers = 1;
        desc.numoutputbuffers = 1;

        desc.read = [](FMOD_DSP_STATE* dsp, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels) -> FMOD_RESULT {
            auto& self = AudioAnalyser::get();
            unsigned int totalSamples = length * static_cast<unsigned int>(*outchannels);

            std::memcpy(outbuffer, inbuffer, totalSamples * sizeof(float));

            if (!self.m_attached.load(std::memory_order_relaxed)) return FMOD_OK;

            float sum = 0.0f;
            for (unsigned int i = 0; i < totalSamples; ++i) {
                float s = inbuffer[i];
                sum += s * s;
            }

            float invCount = 1.0f / static_cast<float>(totalSamples);
            float rms = std::sqrt(sum * invCount);

            if (self.m_useNormalization.load(std::memory_order_relaxed)) {
                float maxVol = self.m_maxVolume.load(std::memory_order_relaxed);
                maxVol = rms > maxVol ? rms : std::max(0.01f, maxVol - 0.0001f);
                self.m_maxVolume.store(maxVol, std::memory_order_relaxed);

                float normalized = std::clamp(rms / maxVol, 0.0f, 1.0f);
                float current = self.m_volume.load(std::memory_order_relaxed);
                self.m_volume.store(current + (normalized - current) * 0.3f, std::memory_order_relaxed);
            } else {
                float current = self.m_volume.load(std::memory_order_relaxed);
                self.m_volume.store(current + (rms - current) * 0.3f, std::memory_order_relaxed);
            }

            return FMOD_OK;
        };

        fmod->m_system->createDSP(&desc, &m_dsp);
    }

    void attach() {
        init();
        if (m_attached.load(std::memory_order_relaxed) || !m_dsp) return;

        auto fmod = FMODAudioEngine::sharedEngine();
        if (fmod && fmod->m_backgroundMusicChannel) {
            fmod->m_backgroundMusicChannel->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, m_dsp);
            m_attached.store(true, std::memory_order_relaxed);
        }
    }

    void detach() {
        if (!m_attached.load(std::memory_order_relaxed) || !m_dsp) return;

        auto fmod = FMODAudioEngine::sharedEngine();
        if (fmod && fmod->m_backgroundMusicChannel) {
            fmod->m_backgroundMusicChannel->removeDSP(m_dsp);
        }
        m_attached.store(false, std::memory_order_relaxed);
        m_volume.store(0.0f, std::memory_order_relaxed);
        m_maxVolume.store(0.1f, std::memory_order_relaxed);
    }

    inline float getVolume() const { return m_volume.load(std::memory_order_relaxed); }
    inline bool isAttached() const { return m_attached.load(std::memory_order_relaxed); }
    inline void setUseNormalization(bool value) { m_useNormalization.store(value, std::memory_order_relaxed); }

private:
    AudioAnalyser() = default;
    FMOD::DSP* m_dsp = nullptr;
    std::atomic<float> m_volume{0.0f};
    std::atomic<float> m_maxVolume{0.1f};
    std::atomic<bool> m_useNormalization{true};
    std::atomic<bool> m_attached{false};
};

GUI_HACK_CREATE("Cosmetic", "Custom DSP Pulse", "Replaces game pulse with live audio DSP volume", false);

class $modify(CustomPulsePlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Custom DSP Pulse");

        hack.addHookPtr(self.getHook("PlayLayer::updateVisibility").unwrap());

        hack.setHandler([](bool enabled) {
            if (enabled) {
                AudioAnalyser::get().attach();
            } else {
                AudioAnalyser::get().detach();
            }
        });

        hack.setCustomWindowImGui([
            sensKey = hack.formatAdditionalSetting("sensitivity"),
            minKey = hack.formatAdditionalSetting("min_threshold"),
            normKey = hack.formatAdditionalSetting("normalize")
        ]{
            ImGuiWidgetConfig::Checkbox("Auto-Gain Normalization", normKey, true);
            ImGuiWidgetConfig::DragFloat("##PulseSens", sensKey, 0.1f, 0.0f, 10.0f, 1.5f, "Sensitivity: %.2f");
            ImGuiWidgetConfig::DragFloat("##PulseMin", minKey, 0.01f, 0.0f, 1.0f, 0.15f, "Min Pulse: %.2f");
        });

        hack.setCustomWindowCocos([
            sensKey = hack.formatAdditionalSetting("sensitivity"),
            minKey = hack.formatAdditionalSetting("min_threshold"),
            normKey = hack.formatAdditionalSetting("normalize")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigToggle("Auto-Gain Normalization", normKey, true);
            popup->addConfigFloatInput("Sensitivity", sensKey, 0.0f, 10.0f, 1.5f);
            popup->addConfigFloatInput("Min Pulse", minKey, 0.0f, 1.0f, 0.15f);
        });
    }

    void updateVisibility(float dt) {
        auto& analyser = AudioAnalyser::get();
        auto& config = Config::get();

        if (!analyser.isAttached()) {
            analyser.attach();
        }

        analyser.setUseNormalization(config.get<bool>("cosmetic.custom_dsp_pulse::normalize", true));

        float rawVolume = analyser.getVolume();
        float sensitivity = config.get<float>("cosmetic.custom_dsp_pulse::sensitivity", 1.5f);
        float minThreshold = config.get<float>("cosmetic.custom_dsp_pulse::min_threshold", 0.15f);

        float customPulse = minThreshold + (rawVolume * sensitivity);

        auto fmod = FMODAudioEngine::sharedEngine();
        fmod->m_pulse1 = customPulse;
        fmod->m_pulse2 = customPulse;

        if (m_audioEffectsLayer) {
            m_audioEffectsLayer->m_audioScale = customPulse;
        }

        PlayLayer::updateVisibility(dt);
    }
};