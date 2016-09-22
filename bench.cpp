#include "sample_common.h"

StatusLine *g_statusline;

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
    new Enemy(); // At least one sprite is necessary to send File and TileDeck first
    g_statusline = new StatusLine();
    g_statusline->setLoc(-SCRW/2+50,SCRH/2-50);
    g_effect_layer->insertProp(g_statusline);
}

void benchUpdate() {
    if( g_mouse->getButton(0) ) for(int i=0;i<10;i++) new Enemy();

    CharGrid *cg = g_statusline->getCharGrid();
    cg->printf(0,0,Color(1,1,1,1), "Sprite: %d", g_main_layer->countProps());
}

SAMPLE_COMMON_MAIN_FUNCTION(benchInit,benchUpdate,"bench");
