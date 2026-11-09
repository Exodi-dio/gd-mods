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

enum class Anim {
    AmongUs,
    Celeste,
    Jumpscare,
    Maro,
    CBFDetected,
    Undertale,
    Ghost,
    Pop,
    ToBeContinued,
    SpeechBubble,
    What,
    NewBest,
    Poof,
    Blood,
    MrHippo,
    FadeOut,
    Minecraft,
    SquidGame,
    SquidGame2,
    FNAF,
    Ambush,
    JeffTheKiller,
    JeffTheKiller2,
    FiveNights,
    PoppyPlaytime,
    Scp173,
    Granny,
    Baldi,
    Sonic,
    Fnaf,
    Fnaf2,
    Fnaf3,
    Fnaf4,
    Fnaf5,
    Fnaf6,
    Fnaf7,
    Fnaf8,
    Fnaf9,
    Fnaf10,
    Fnaf11,
    Fnaf12,
    Fnaf13,
    Fnaf14,
    Fnaf15,
    Fnaf16,
    Fnaf17,
    Fnaf18,
    Fnaf19,
    Fnaf20,
};

struct AnimationSetting {
    std::string id;
    std::string name;
    std::string description;
    SettingType type;
    std::vector<std::string> elements;
};

inline const std::array<AnimationSetting, 8> defaultSettings = {{
    { .id = "speed", .name = "Speed", .description = "The speed at which the animation plays.", .type = SettingType::Speed },
    { .id = "only-after", .name = "Only After", .description = "Play the animation only if you die after a certain percentage.", .type = SettingType::Percent },
    { .id = "probability", .name = "Probability", .description = "The probability that the animation will play.", .type = SettingType::Percent },
    { .id = "prevent-early-restart", .name = "Prevent Early Restart", .description = "Prevent restarting the level while the animation is playing.", .type = SettingType::Toggle },
    { .id = "play-sound-effects", .name = "Play Sound Effects", .description = "Play any sound effects associated with the animation.", .type = SettingType::Toggle },
    { .id = "play-in-practice", .name = "Play in Practice", .description = "Play the animation in practice mode.", .type = SettingType::Toggle },
    { .id = "play-in-testmode", .name = "Play in Testmode", .description = "Play the animation in test mode.", .type = SettingType::Toggle },
    { .id = "only-on-new-best", .name = "Only on New Best", .description = "Play the animation only when achieving a new best.", .type = SettingType::Toggle }
}};

inline const std::unordered_map<int, std::vector<AnimationSetting>> extraSettings = {
    { Anim::AmongUs, std::vector<AnimationSetting>{ { .id = "animation", .name = "Kill Animation", .description = "The Among Us kill animation that plays on death.", .type = SettingType::Select, .elements = { "Random", "Gun", "Knife", "Neck", "Alien" } }, { .id = "colors", .name = "Colors", .description = "The colors of the impostor and victim in the animation.", .type = SettingType::AmongUsColor, .elements = { "Player Colors", "Custom" } } } },
    { Anim::Celeste, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the death/respawn animation on the second player as well.", .type = SettingType::Toggle }, { .id = "explosion-color", .name = "Explosion Color", .description = "The color to use for the explosion animation.", .type = SettingType::CelesteColor }, { .id = "respawn-animation", .name = "Respawn Animation", .description = "Play the Celeste respawn animation.", .type = SettingType::Toggle }, { .id = "shockwave", .name = "Shockwave", .description = "Play the shockwave effect.", .type = SettingType::Toggle }, { .id = "bounce-off", .name = "Bounce Off", .description = "Bounce the player off.", .type = SettingType::Toggle }, { .id = "speed-affects-respawn", .name = "Speed Affects Respawn", .description = "The Speed setting will affect the respawn animation (if enabled) as well.", .type = SettingType::Toggle }, { .id = "fast", .name = "Fast", .description = "Skip the wait time before the explosion effect.", .type = SettingType::CelesteFastToggle }, { .id = "transition", .name = "Transition", .description = "The Celeste transition to use in the animation.", .type = SettingType::Select, .elements = { "None", "Random", "Chapter 1", "Chapter 2", "Chapter 3", "Chapter 4", "Chapter 5", "Chapter 6", "Chapter 7", "Chapter 8" } } } },
    { Anim::Jumpscare, std::vector<AnimationSetting>{ { .id = "static", .name = "Static", .description = "Stop the image from zooming in.", .type = SettingType::Toggle }, { .id = "disable-flashing", .name = "Disable Flashing", .description = "Stop the image from flashing repeatedly.", .type = SettingType::Toggle }, { .id = "ignore-volume", .name = "Ignore Volume", .description = "Ignore the game's SFX volume.", .type = SettingType::Toggle }, { .id = "fill-screen", .name = "Fill Screen", .description = "Make the image fill the screen (may crop the image).", .type = SettingType::Toggle }, { .id = "images-folder", .name = "Images Folder", .description = "Folder containing the jumpscare images; one will be chosen at random.", .type = SettingType::Folder, .folder = Mod::get()->getSaveDir() / "jumpscare-images" }, { .id = "sounds-folder", .name = "Sounds Folder", .description = "Folder containing the jumpscare sounds; one will be chosen at random.", .type = SettingType::Folder, .folder = Mod::get()->getSaveDir() / "jumpscare-sounds" } } },
    { Anim::Maro, std::vector<AnimationSetting>{ { .id = "use-nes-colors", .name = "Use NES Colors", .description = "Uses the colors from the original NES.", .type = SettingType::Toggle }, { .id = "use-maro-sprite", .name = "Use Mario Sprite", .description = "Uses the sprite of Mario instead of a pixel-art version of the player.", .type = SettingType::Toggle }, { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::CBFDetected, std::vector<AnimationSetting>{ { .id = "use-level-name", .name = "Use Level Name", .description = "Use the name of the level you're currently playing instead of Nullscapes.", .type = SettingType::Toggle }, { .id = "include-loser", .name = "Include Loser! text", .description = "In later versions of the level, the text got changed to remove \"Loser\", this setting brings it back.", .type = SettingType::Toggle }, { .id = "yellow-color", .name = "Yellow Color", .description = "The red text was yellow at some point, this setting brings it back.", .type = SettingType::Toggle } } },
    { Anim::Undertale, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Ghost, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Pop, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::ToBeContinued, std::vector<AnimationSetting>{ { .id = "yellow-shader", .name = "Yellow Shader", .description = "Enables the yellow filter you usually see in these memes.", .type = SettingType::Toggle } } },
    { Anim::SpeechBubble, std::vector<AnimationSetting>{ { .id = "show-ui", .name = "Show UI", .description = "", .type = SettingType::Toggle }, { .id = "scale-y", .name = "Scale Y", .description = "", .type = SettingType::Scale }, { .id = "flip-x", .name = "Flip X", .description = "", .type = SettingType::Toggle }, { .id = "flip-y", .name = "Flip Y", .description = "", .type = SettingType::Toggle }, { .id = "speech-bubble", .name = "Speech Bubble", .description = "The speech bubble image that will be used by default when dying.", .type = SettingType::SpeechBubbleSelect, .elements = { "Random" } }, { .id = "", .name = "Speech Bubbles Folder", .description = "Folder containing the speech bubble images.", .type = SettingType::Folder, .folder = Mod::get()->getSaveDir() / "speech-bubbles" }, { .id = "", .name = "Images Folder", .description = "Folder containing the images you have saved.", .type = SettingType::Folder, .folder = Mod::get()->getSaveDir() / "screenshots" } } },
    { Anim::What, std::vector<AnimationSetting>{ { .id = "target-p2", .name = "Target Player 2", .description = "Zoom into the second player if possible.", .type = SettingType::Toggle }, { .id = "top-text", .name = "Top Text", .description = "", .type = SettingType::Text }, { .id = "bottom-text", .name = "Bottom Text", .description = "", .type = SettingType::Text } } },
    { Anim::NewBest, std::vector<AnimationSetting>{ { .id = "play-in-platformer", .name = "Play in platformer", .description = "New best in platformer (?).", .type = SettingType::Toggle }, { .id = "use-custom-percent", .name = "Use Custom Percent", .description = "Show a constant % value instead of where you actually died.", .type = SettingType::PercentToggle }, { .type = SettingType::Empty } } },
    { Anim::Poof, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Blood, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle }, { .id = "blood-spatter", .name = "Blood Spatter", .description = "Show the blood spatter on screen.", .type = SettingType::Toggle } } },
    { Anim::MrHippo, std::vector<AnimationSetting>{ { .id = "monologue", .name = "Monologue", .description = "What Mr. Hippo will yap about.", .type = SettingType::MonologueSelect, .elements = { "Random", "Monologue 1", "Monologue 2", "Monologue 3", "Monologue 4" } } } },
    { Anim::FadeOut, std::vector<AnimationSetting>{ { .id = "slow-down", .name = "Slow Down", .description = "Slow down the game along with the sound.", .type = SettingType::Toggle }, { .id = "desaturate", .name = "Desaturate", .description = "Slightly desaturate the colors for more immersion.", .type = SettingType::Toggle } } },
    { Anim::Minecraft, std::vector<AnimationSetting>{ { .id = "no-overlay", .name = "No Overlay", .description = "Disables the \"You Died!\" overlay, leaving only the player animation.", .type = SettingType::Toggle } } },
    { Anim::SquidGame, std::vector<AnimationSetting>{ { .id = "red-light-green-light", .name = "Red Light, Green Light", .description = "Play the Red Light Green Light animation.", .type = SettingType::Toggle } } },
    { Anim::SquidGame2, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::FNAF, std::vector<AnimationSetting>{ { .id = "jumpscare", .name = "Jumpscare", .description = "Play the FNAF jumpscare animation.", .type = SettingType::Toggle }, { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Ambush, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::JeffTheKiller, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::JeffTheKiller2, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::FiveNights, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::PoppyPlaytime, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Scp173, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Granny, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Baldi, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Sonic, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf2, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf3, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf4, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf5, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf6, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf7, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf8, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf9, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf10, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf11, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf12, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf13, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf14, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf15, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf16, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf17, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf18, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf19, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
    { Anim::Fnaf20, std::vector<AnimationSetting>{ { .id = "second-player", .name = "Second Player", .description = "Play the animation on the second player as well.", .type = SettingType::Toggle } } },
};
