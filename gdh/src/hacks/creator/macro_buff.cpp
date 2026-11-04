#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/ringBuffer.hpp"

GUI_HACK_CREATE("Creator", "Macro Buff", "Auto-places spikes on jump. Highly experimental. Works only on pure jumps! Pads and gravity shifts break the structure. Cube and Robot only", false);

class $modify(MacroBuffGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Macro Buff");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::processCommands").unwrap());
    }

    struct Fields {
        bool cached = false;
        CCPoint spikeOffset;
        CCSize spikeSize;

        RingBuffer<CCRect> player_trail{480};
        RingBuffer<GameObject*> spikes{800};
        int frame = 0;
    };

    void cacheSpike() {
        if (m_fields->cached) return;
        auto lel = LevelEditorLayer::get();
        if (!lel) return;

        auto temp = lel->createObject(8, CCPoint(0.f, 0.f), true);
        if (!temp) return;

        CCRect r = temp->getObjectRect();
        m_fields->spikeOffset = temp->getPosition() - r.origin;
        m_fields->spikeSize = r.size;

        lel->removeObject(temp, true);
        m_fields->cached = true;
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

        auto lel = LevelEditorLayer::get();
        if (!lel || !m_player1) return;

        cacheSpike();
        if (!m_fields->cached) return;

        CCRect playerRect = m_player1->getObjectRect();
        m_fields->player_trail.push(playerRect);

        auto checkIntersects = [](const CCRect& r1, const CCRect& r2) {
            return !(r1.origin.x + r1.size.width <= r2.origin.x ||
                    r2.origin.x + r2.size.width  <= r1.origin.x ||
                    r1.origin.y + r1.size.height <= r2.origin.y ||
                    r2.origin.y + r2.size.height <= r1.origin.y);
        };

        m_fields->spikes.for_each([&](GameObject*& spk) {
            if (!spk) return;

            CCRect currentSpikeRect = spk->getObjectRect();
            bool shouldRemove = false;

            m_fields->player_trail.for_each([&](const auto& trailRect) {
                if (!shouldRemove && checkIntersects(currentSpikeRect, trailRect)) {
                    shouldRemove = true;
                }
            });

            if (shouldRemove) {
                lel->removeObject(spk, true);
                spk = nullptr;
            }
        });
        
        if (m_player1->m_isOnGround) return;

        m_fields->frame++;
        if (m_fields->frame < 5) {
            return;
        }
        m_fields->frame = 0;

        float vy = m_player1->m_yVelocity;

        auto spawnSpikeAt = [&](float originX, float originY) {
            CCPoint targetOrigin(originX, originY);
            CCPoint finalPos = targetOrigin + m_fields->spikeOffset;
            auto obj = lel->createObject(8, finalPos, true);
            if (obj) {
                m_fields->spikes.push(obj);
            }
        };

        if (vy >= 0.f) {
            spawnSpikeAt(
                playerRect.origin.x - m_fields->spikeSize.width - 0.1f,
                playerRect.origin.y + playerRect.size.height + 0.1f
            );
        } else {
            spawnSpikeAt(
                playerRect.origin.x + playerRect.size.width + 0.1f,
                playerRect.origin.y + playerRect.size.height + 0.1f
            );
        }

        if (std::abs(vy) >= 5.0f) {
            if (vy >= 0.f) {
                spawnSpikeAt(
                    playerRect.origin.x + playerRect.size.width + 0.1f,
                    playerRect.origin.y - m_fields->spikeSize.height - 0.1f
                );
            } else {
                spawnSpikeAt(
                    playerRect.origin.x - m_fields->spikeSize.width - 0.1f,
                    playerRect.origin.y - m_fields->spikeSize.height - 0.1f
                );
            }
        }
    }
};

class $modify(MacroBuffEditorLayer, LevelEditorLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Macro Buff");        
        
        hack.addHookPtr(self.getHook("LevelEditorLayer::onPlaytest").unwrap());
    }

    void onPlaytest() {
        LevelEditorLayer::onPlaytest();

        if (auto gjbgl = static_cast<MacroBuffGJBaseGameLayer*>(GJBaseGameLayer::get())) {
            gjbgl->m_fields->player_trail.clear();
            gjbgl->m_fields->spikes.clear();
            gjbgl->m_fields->frame = 0;
        }
    }
};