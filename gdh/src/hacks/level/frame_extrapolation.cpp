#include <Geode/Geode.hpp>
#include <Geode/modify/GJEffectManager.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Frame Extrapolation", "Extrapolates frames for visual updates beyond 240fps (unstable, needs testing)", false);

class $modify(FrameExtrapolationGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Frame Extrapolation");
        hack.addHookPtr(self.getHook("GJBaseGameLayer::getModifiedDelta").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::update").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::updateCamera").unwrap());
    }

    struct Fields {
        float tickDuration = 0.f;
        float elapsedTickTime = 0.f;
        double modifiedDelta = 0.0;

        bool isExtrapolatingCamera = false;
        CCPoint savedCameraPos;
    };

    double getModifiedDelta(float dt) {
        auto ret = GJBaseGameLayer::getModifiedDelta(dt);
        m_fields->modifiedDelta = ret;
        return ret;
    }

    void update(float dt) {
        if (dt == 0.f) return GJBaseGameLayer::update(dt);
        
        auto playLayer = PlayLayer::get();
        if (!playLayer || playLayer->m_levelEndAnimationStarted) {
            return GJBaseGameLayer::update(dt);
        }
        
        auto f = m_fields.self();

        if (f->modifiedDelta != 0) {
            f->tickDuration = f->modifiedDelta;
            f->elapsedTickTime = 0.f;
        } else {
            f->elapsedTickTime += dt;
        }

        GJBaseGameLayer::update(dt);

        if (m_playerDied || playLayer->m_levelEndAnimationStarted || isFlipping() || !m_started) return;
        if (f->tickDuration == 0) return;

        float t = std::clamp(f->elapsedTickTime / f->tickDuration, 0.f, 1.f);
        if (t <= 0.f) return;

        extrapolatePlayer(m_player1, t);
        extrapolatePlayer(m_player2, t);

        f->savedCameraPos = m_gameState.m_cameraPosition;
        
        f->isExtrapolatingCamera = true;
        GJBaseGameLayer::updateCamera(FLT_MIN);
        f->isExtrapolatingCamera = false;
        
        interpolateGround(m_groundLayer, t);
        interpolateGround(m_groundLayer2, t);
    }

    void updateCamera(float dt) {
        auto f = m_fields.self();

        if (!f->isExtrapolatingCamera) {
            GJBaseGameLayer::updateCamera(dt);
            return;
        }

        GJBaseGameLayer::updateCamera(dt);

        auto newCameraX = f->savedCameraPos.x - m_gameState.m_cameraPosition.x;
        auto newCameraY = f->savedCameraPos.y - m_gameState.m_cameraPosition.y;
        
        m_gameState.m_cameraStepDiff.x = newCameraX;
        m_gameState.m_cameraStepDiff.y = newCameraY;
    }

    void extrapolatePlayer(PlayerObject* player, float t) {
        if (!player) return;

        float deltaX = player->m_position.x - player->m_lastPosition.x;
        float deltaY = player->m_position.y - player->m_lastPosition.y;
        
        player->setPosition({
            player->m_position.x + (deltaX * t),
            player->m_position.y + (deltaY * t)
        });

        float rotateSpeed = (player->m_isBall && player->m_isBallRotating) ? 1.0f : player->m_rotateSpeed;
        float endRot = (player->m_rotationSpeed * rotateSpeed) / 240.0f;
        float baseRot = player->m_isSideways ? -90.f : 0.f;
        
        player->m_mainLayer->setRotation((endRot * t) + baseRot);
    }

    void interpolateGround(GJGroundLayer* ground, float t) {
        if (!ground) return;
        if (auto batch = ground->getChildByType<CCSpriteBatchNode>(0)) {
            batch->setPositionX(m_gameState.m_cameraStepDiff.x * t);
        }
    }
};