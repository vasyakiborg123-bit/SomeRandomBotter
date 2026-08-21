#pragma once

#include "macro.hpp"

enum state {
    none,
    recording,
    playing
};

struct Global {
    state state = state::none;
    Macro macro;
    int frameOffset = 0;
    bool tpsEnabled = false;
    float tps = 240.f;
    size_t currentInputIndex = 0;  // для воспроизведения
    
    static Global& get() {
        static Global instance;
        return instance;
    }
    
    static int getCurrentFrame() {
        PlayLayer* pl = PlayLayer::get();
        if (!pl) return 0;
        
        auto& g = Global::get();
        int frame = static_cast<int>(pl->m_gameState.m_levelTime * 240.f);
        frame++;
        frame -= g.frameOffset;
        if (frame < 0) return 0;
        return frame;
    }
};
