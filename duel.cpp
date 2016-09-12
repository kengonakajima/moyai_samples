#include "sample_common.h"

///////////////////
class PC;
PC *g_pcs[2];

class PC : public Char {
public:
    int direction; // -1:Left faced, 1:Right faced
    bool jumping;
    PC(int faction) : Char(CHARTYPE_PC) {
        setDeck(g_base_deck);
        setIndex( factionToBaseIndex(faction));
        setScl(48);
        g_main_layer->insertProp(this);
        direction = (faction==0) ? 1:0;
        jumping = false;
    }
    virtual bool charPoll(double dt) {
        setXFlip( direction==1 );

        float ymin = -48-24;
        if(loc.y < ymin) {
            loc.y = ymin;
            v.y = 0;
            onLand();
        } else {
            float gravity = 1500;
            v.y -= gravity * dt;
        }
        return true;
    }
    int factionToBaseIndex(int faction) {
        return faction==0?ATLAS_PC_RED_BASE:ATLAS_PC_BLUE_BASE;
    }
    void tryJump() {
        if( jumping == false ) {
            jumping = true;
            v.y = 500;
        }
    }
    void onLand() {
        jumping = false;
    }
};


void duelInit() {
    Prop2D *ground = new Prop2D();
    Grid *g = new Grid(24,6);
    for(int j=0;j<g->height;j++) {
        for(int i=0;i<g->width;i++) {
            g->set(i,j,ATLAS_GROUND_BLOCK);
        }
    }
    ground->addGrid(g);
    ground->setDeck(g_base_deck);
    ground->setScl(48);
    ground->setLoc(-12*48,-8*48);
    g_main_layer->insertProp(ground);

    
    g_pcs[0] = new PC(0);
    g_pcs[0]->setLoc( -SCRW/2 + 100, 0 );
    g_pcs[1] = new PC(1);
    g_pcs[1]->setLoc( SCRW/2 - 100, 0 );    
    
}
void duelUpdate() {
    if( g_keyboard->getKey(' ')) {
        g_pcs[0]->tryJump();
    }
}
SAMPLE_COMMON_MAIN_FUNCTION(duelInit,duelUpdate);
