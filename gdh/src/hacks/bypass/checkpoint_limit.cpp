#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Checkpoint Limit", "Removes the limit that deletes previous checkpoints after the 50th checkpoint", false);

class $modify(CheckpointLimitPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Checkpoint Limit");        
        
        hack.addHookPtr(self.getHook("PlayLayer::storeCheckpoint").unwrap());
    }

    void storeCheckpoint(CheckpointObject* obj) {        
        m_checkpointArray->addObject(obj);
        PlayLayer::addToSection(obj->m_physicalCheckpointObject);
    }
};