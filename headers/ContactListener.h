#ifndef contactlistener_h
#define contactlistener_h

#include "box2d/box2d.h"

class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};

#endif /* contactlistener_h */
