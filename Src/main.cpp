#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) 
   $override
   bool init() (
      if (MenuLayer::init()) return false;



      return true:
    )
        }}
