#pragma once

#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <array>

using namespace geode::prelude;

enum class SettingType {
    Toggle,
    Text,
    Percent,
    PercentToggle,
    Select,
    Speed,
    Color,
    Scale,
    CelesteColor,
    CelesteFastToggle,
    SpeechBubbleSelect,
    Folder,
    Empty,
    MonologueSelect,
    AmongUsColor,
};
...