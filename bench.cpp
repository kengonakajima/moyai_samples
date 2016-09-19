#include "sample_common.h"

class Enemy : public Prop2D {
public:
    Vec2 v;
    Enemy() {
        setDeck(g_base_deck);
        setScl(24);
        setLoc( range(-500,500),range(-500,500));
        setIndex(ATLAS_ENEMY_ANIMBASE);
        v = Vec2( range(-100,100), range(-100,100) );
        g_main_layer->insertProp(this);
    }
    virtual bool prop2DPoll(double dt) {
        loc += v*dt;
        if( loc.x<-500 || loc.x>500) v.x*=-1;
        if( loc.y<-500 || loc.y>500) v.y*=-1;
        return true;
    }
};

void benchInit() {
}
void benchUpdate() {
    for(int i=0;i<10;i++) new Enemy();
}

SAMPLE_COMMON_MAIN_FUNCTION(benchInit,benchUpdate,"bench");
