#include <Geode/Geode.hpp>
#include "macro.hpp"
#include "global.hpp"

using namespace geode::prelude;

class $modify(PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        
        auto& g = Global::get();
        
        
        if (g.state == state::playing) {
            int frame = Global::getCurrentFrame();
            
        }
    }
};

class $modify(PlayerObject) {
    void pushButton(PlayerButton button) {
        PlayerObject::pushButton(button);
        
        auto& g = Global::get();
        if (g.state == state::recording) {
            int frame = Global::getCurrentFrame();
            bool p2 = this == PlayLayer::get()->m_player2;
            Macro::recordAction(frame, static_cast<int>(button), p2, true);
        }
    }
    
    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);
        
        auto& g = Global::get();
        if (g.state == state::recording) {
            int frame = Global::getCurrentFrame();
            bool p2 = this == PlayLayer::get()->m_player2;
            Macro::recordAction(frame, static_cast<int>(button), p2, false);
        }
    }
};

$execute {
    
    log::info("Macro mod loaded");
}
