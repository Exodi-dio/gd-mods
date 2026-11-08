#pragma once

#include <Geode/loader/Event.hpp>

#include <optional>

namespace bananchikireal::global_demonlist_rank {

enum class PlacementState {
    Loading,
    Listed,
    Unlisted,
    Error,
};

struct PlacementResult {
    int levelID = 0;
    PlacementState state = PlacementState::Loading;
    std::optional<int> placement;
};

class PlacementUpdateEvent final : public geode::Event<
    PlacementUpdateEvent,
    bool(PlacementResult const&)
> {
public:
    using Event::Event;
};

class PlacementCacheEvent final : public geode::Event<
    PlacementCacheEvent,
    bool(int, std::optional<PlacementResult>&)
> {
public:
    using Event::Event;
};

inline std::optional<PlacementResult> getCachedPlacement(int levelID) {
    std::optional<PlacementResult> result;
    PlacementCacheEvent().send(levelID, result);
    return result;
}

} // namespace bananchikireal::global_demonlist_rank
