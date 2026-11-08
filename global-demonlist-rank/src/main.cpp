#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/utils/web.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <optional>
#include <unordered_map>

#include "../include/GlobalDemonlistRankAPI.hpp"

using namespace geode::prelude;
using namespace bananchikireal::global_demonlist_rank;

namespace {
constexpr char const* API_URL = "https://api.demonlist.org/level/classic/get";
constexpr char const* USER_AGENT = "BANANCHIKIREAL-GlobalDemonlistRank/1.2.1";

// A null rank means that the API confirmed this level is not on the list.
// The cache lives only for the current game session, so placements are refreshed
// after restarting Geometry Dash.
std::unordered_map<int, std::optional<int>> g_rankCache;
std::unordered_map<int, PlacementResult> g_apiState;

void publishPlacementState(
    int levelID,
    PlacementState state,
    std::optional<int> placement = std::nullopt
) {
    auto result = PlacementResult {
        .levelID = levelID,
        .state = state,
        .placement = placement,
    };
    g_apiState.insert_or_assign(levelID, result);
    PlacementUpdateEvent().send(result);
}

std::optional<int> parsePlacement(web::WebResponse const& response) {
    auto json = response.json();
    if (!json.isOk()) {
        return std::nullopt;
    }

    auto& root = json.unwrap();
    auto message = root.get<std::string>("message");
    if (!message.isOk() || message.unwrap() != "success") {
        return std::nullopt;
    }

    auto data = root.get("data");
    if (!data.isOk()) {
        return std::nullopt;
    }

    auto placement = data.unwrap().get<int>("placement");
    if (!placement.isOk() || placement.unwrap() < 1) {
        return std::nullopt;
    }

    return placement.unwrap();
}

$on_mod(Loaded) {
    PlacementCacheEvent().listen([](int levelID, std::optional<PlacementResult>& result) {
        if (auto cached = g_apiState.find(levelID); cached != g_apiState.end()) {
            result = cached->second;
            return ListenerResult::Stop;
        }
        return ListenerResult::Propagate;
    }).leak();
}
}

class $modify(GlobalDemonlistRankLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        TaskHolder<web::WebResponse> request;
        CCNode* placement = nullptr;
        CCSprite* trophy = nullptr;
        CCLabelBMFont* label = nullptr;
        LoadingSpinner* loading = nullptr;
        CCLabelBMFont* error = nullptr;
        CCPoint ingameListRankPosition = CCPointZero;
        int levelID = 0;
    };

    char const* trophyFrameForPlacement(int placement) {
        if (placement == 1) return "rankIcon_1_001.png";
        if (placement <= 10) return "rankIcon_top10_001.png";
        if (placement <= 50) return "rankIcon_top50_001.png";
        if (placement <= 100) return "rankIcon_top100_001.png";
        if (placement <= 200) return "rankIcon_top200_001.png";
        if (placement <= 500) return "rankIcon_top500_001.png";
        if (placement <= 1000) return "rankIcon_top1000_001.png";
        if (placement <= 2500) return "rankIcon_top2500_001.png";
        if (placement <= 5000) return "rankIcon_top5000_001.png";
        if (placement <= 10000) return "rankIcon_top10000_001.png";
        return "rankIcon_all_001.png";
    }

    void layoutPlacement() {
        auto placement = m_fields->placement;
        auto trophy = m_fields->trophy;
        auto label = m_fields->label;
        if (!placement || !trophy || !label) {
            return;
        }

        auto const trophySize = trophy->getScaledContentSize();
        auto const labelSize = label->getScaledContentSize();
        constexpr float gap = 3.f;
        auto const height = std::max(trophySize.height, labelSize.height);
        auto const width = trophySize.width + gap + labelSize.width;

        placement->setContentSize({ width, height });
        trophy->setPosition({ trophySize.width / 2.f, height / 2.f });
        label->setPosition({ trophySize.width + gap + labelSize.width / 2.f, height / 2.f });
    }

    CCNode* ingameListRank() {
        if (auto rank = this->getChildByIDRecursive("adyagd.ingamelistmod/placement-label")) {
            return rank;
        }

        if (!Loader::get()->isModLoaded("adyagd.ingamelistmod")) {
            return nullptr;
        }

        // IngameListMod v1.2.6 assigns its node ID only after a network
        // request. On a cache hit, the same numeric label is created without
        // an ID at this fixed position, so use the position as a fallback.
        auto const expected = m_fields->ingameListRankPosition;
        for (auto child : CCArrayExt<CCNode*>(this->getChildren())) {
            auto label = typeinfo_cast<CCLabelBMFont*>(child);
            if (!label || !label->isVisible()) {
                continue;
            }

            auto const text = std::string(label->getString());
            auto const numeric = !text.empty() && std::all_of(
                text.begin(),
                text.end(),
                [](unsigned char character) { return std::isdigit(character) != 0; }
            );
            auto const position = label->getPosition();
            if (
                numeric &&
                std::abs(position.x - expected.x) <= 1.f &&
                std::abs(position.y - expected.y) <= 1.f
            ) {
                return label;
            }
        }
        return nullptr;
    }

    CCPoint statusPosition(CCNode* node) {
        if (!node || !m_difficultySprite || !m_difficultySprite->getParent()) {
            return CCPointZero;
        }

        auto parent = m_difficultySprite->getParent();
        auto const difficultySize = m_difficultySprite->getScaledContentSize();
        auto lowestY = m_difficultySprite->getPositionY() -
            difficultySize.height * m_difficultySprite->getAnchorPoint().y;

        auto includeNodeBottom = [parent, &lowestY](CCNode* sibling) {
            if (!sibling || !sibling->isVisible()) {
                return;
            }

            auto const localBottom = CCPoint { sibling->getContentSize().width / 2.f, 0.f };
            auto const worldBottom = sibling->convertToWorldSpace(localBottom);
            auto const bottomInParent = parent->convertToNodeSpace(worldBottom);
            lowestY = std::min(lowestY, bottomInParent.y);
        };

        includeNodeBottom(m_starsLabel);
        includeNodeBottom(m_starsIcon);
        for (auto coin : CCArrayExt<CCSprite*>(m_coins)) {
            includeNodeBottom(coin);
        }

        // IngameListMod adds this label asynchronously to LevelInfoLayer. Its
        // trophy has no node ID, but the label reaches lower than the trophy,
        // so placing our row below the label keeps both complete rows apart.
        auto rank = ingameListRank();
        auto hasIngameListRank = rank && rank->isVisible();
        if (hasIngameListRank) {
            includeNodeBottom(rank);
        }

        auto const nodeHeight = node->getScaledContentSize().height;
        return {
            m_difficultySprite->getPositionX(),
            lowestY - nodeHeight / 2.f - 5.f + (hasIngameListRank ? 7.f : 0.f),
        };
    }

    void positionStatusNodes() {
        if (m_fields->placement) {
            m_fields->placement->setPosition(statusPosition(m_fields->placement));
        }
        if (m_fields->loading) {
            m_fields->loading->setPosition(statusPosition(m_fields->loading));
        }
        if (m_fields->error) {
            m_fields->error->setPosition(statusPosition(m_fields->error));
        }
    }

    void refreshCompatibilityLayout(float) {
        positionStatusNodes();
    }

    void showLoading() {
        if (!m_fields->loading) {
            return;
        }

        m_fields->placement->setVisible(false);
        m_fields->error->stopAllActions();
        m_fields->error->setVisible(false);
        m_fields->loading->setVisible(true);
        positionStatusNodes();
    }

    void hideLoading() {
        if (m_fields->loading) {
            m_fields->loading->setVisible(false);
        }
    }

    void showNetworkError() {
        hideLoading();
        if (!m_fields->error) {
            return;
        }

        m_fields->error->stopAllActions();
        m_fields->error->setOpacity(255);
        m_fields->error->setVisible(true);
        positionStatusNodes();
        m_fields->error->runAction(CCSequence::create(
            CCDelayTime::create(.4f),
            CCFadeOut::create(.65f),
            CCHide::create(),
            nullptr
        ));
    }

    void showPlacement(int placement) {
        if (!m_fields->placement || !m_fields->trophy || !m_fields->label) {
            return;
        }

        hideLoading();
        auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(
            trophyFrameForPlacement(placement)
        );
        if (frame) {
            m_fields->trophy->setDisplayFrame(frame);
        }
        m_fields->label->setString(fmt::format("{}", placement).c_str());
        layoutPlacement();
        positionStatusNodes();
        m_fields->placement->setVisible(true);
    }

    void addStatusNodes() {
        if (!m_difficultySprite || !m_difficultySprite->getParent()) {
            return;
        }

        auto parent = m_difficultySprite->getParent();
        auto placement = CCNode::create();
        placement->setID("global-demonlist-placement"_spr);
        placement->setAnchorPoint({ .5f, .5f });
        placement->setVisible(false);

        auto trophy = CCSprite::createWithSpriteFrameName("rankIcon_all_001.png");
        trophy->setID("global-demonlist-trophy"_spr);
        trophy->setScale(.55f);

        auto label = CCLabelBMFont::create("0000", "goldFont.fnt");
        label->setID("global-demonlist-rank-label"_spr);
        label->setScale(.45f);

        placement->addChild(trophy);
        placement->addChild(label);

        auto loading = LoadingSpinner::create(10.f);
        loading->setID("global-demonlist-loading"_spr);
        loading->setVisible(false);

        auto error = CCLabelBMFont::create("X", "bigFont.fnt");
        error->setID("global-demonlist-network-error"_spr);
        error->setColor({ 255, 70, 70 });
        error->setScale(.3f);
        error->setVisible(false);

        auto const zOrder = m_difficultySprite->getZOrder() + 1;
        parent->addChild(placement, zOrder);
        parent->addChild(loading, zOrder);
        parent->addChild(error, zOrder);
        m_fields->placement = placement;
        m_fields->trophy = trophy;
        m_fields->label = label;
        m_fields->loading = loading;
        m_fields->error = error;
        layoutPlacement();
        positionStatusNodes();
    }

    void requestPlacement() {
        auto const levelID = m_fields->levelID;
        if (levelID <= 0) {
            return;
        }

        if (auto cached = g_rankCache.find(levelID); cached != g_rankCache.end()) {
            if (cached->second.has_value()) {
                publishPlacementState(levelID, PlacementState::Listed, *cached->second);
                showPlacement(*cached->second);
            }
            else {
                publishPlacementState(levelID, PlacementState::Unlisted);
            }
            return;
        }

        publishPlacementState(levelID, PlacementState::Loading);
        showLoading();

        auto request = web::WebRequest();
        request
            .header("Accept", "application/json")
            .userAgent(USER_AGENT)
            .timeout(std::chrono::seconds(10));

        auto const url = fmt::format("{}?ingame_id={}", API_URL, levelID);
        m_fields->request.spawn(request.get(url), [this, levelID](web::WebResponse response) {
            // The callback is owned by TaskHolder and is cancelled with this layer.
            if (m_fields->levelID != levelID) {
                return;
            }

            if (response.code() == 404) {
                hideLoading();
                g_rankCache.insert_or_assign(levelID, std::nullopt);
                publishPlacementState(levelID, PlacementState::Unlisted);
                return;
            }

            if (!response.ok()) {
                publishPlacementState(levelID, PlacementState::Error);
                showNetworkError();
                log::warn(
                    "Global Demonlist request for level {} failed with HTTP {}: {}",
                    levelID,
                    response.code(),
                    response.errorMessage()
                );
                return;
            }

            auto placement = parsePlacement(response);
            if (!placement.has_value()) {
                publishPlacementState(levelID, PlacementState::Error);
                showNetworkError();
                log::warn("Global Demonlist returned an unexpected response for level {}", levelID);
                return;
            }

            g_rankCache.insert_or_assign(levelID, placement);
            publishPlacementState(levelID, PlacementState::Listed, *placement);
            showPlacement(*placement);
        });
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) {
            return false;
        }

        if (!level || !Mod::get()->getSettingValue<bool>("enabled")) {
            return true;
        }

        // The Global Demonlist classic endpoint does not rank platformer levels.
        if (level->isPlatformer()) {
            return true;
        }

        m_fields->levelID = static_cast<int>(level->m_levelID);
        if (m_fields->levelID <= 0) {
            return true;
        }

        auto const windowSize = CCDirector::sharedDirector()->getWinSize();
        auto const coinOffset = static_cast<int>(level->m_coins) == 0 ? 17.f : 4.f;
        auto const listOffset = static_cast<int>(level->m_stars) == 10 ? 0.f : 7.f;
        m_fields->ingameListRankPosition = CCPoint {
            windowSize.width / 2.f - 100.f,
            windowSize.height / 2.f + coinOffset + listOffset,
        };

        addStatusNodes();
        schedule(
            schedule_selector(GlobalDemonlistRankLevelInfoLayer::refreshCompatibilityLayout),
            .25f
        );
        requestPlacement();
        return true;
    }
};
