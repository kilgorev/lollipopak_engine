//
//  component.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/25/26.
//

#include "component.h"

Component::Component()
    : componentRef(nullptr),
      type(""),
      hasStart(false),
      hasUpdate(false),
      hasLateUpdate(false) {}

bool Component::isEnabled() const {
    if (!componentRef) return false;
    return (*componentRef)["enabled"].cast<bool>();
}
