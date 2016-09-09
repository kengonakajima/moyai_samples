#ifndef SAMPLE_COMMON_H
#define SAMPLE_COMMON_H


#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#include "atlas.h"

#ifndef WIN32
#include <strings.h>
#endif

#if defined(__APPLE__)
#define RETINA 2
#else
#define RETINA 1
#endif

#include "moyai/client.h"



extern Layer *g_main_layer;
extern Layer *g_effect_layer;
extern Texture *g_base_atlas;
extern TileDeck *g_base_deck;

extern Sound *g_shoot_sound;
extern Sound *g_beamhit_sound;
extern Sound *g_enemydie_sound;
extern Sound *g_shipdie_sound;

extern Keyboard *g_keyboard;
extern Mouse *g_mouse;
extern Pad *g_pad;

extern const int SCRW, SCRH;

void sampleCommonInit(int argc, char**argv);
bool sampleCommonDone();
void sampleCommonUpdate();
void sampleCommonRender();
void sampleCommonFinish();


//////////////////////////

typedef enum {
    CHARTYPE_SHIP = 1,
    CHARTYPE_BEAM = 2,
    CHARTYPE_ENEMY = 3,
    CHARTYPE_BULLET = 4,
} CHARTYPE;

class Char : public Prop2D {
public:
    CHARTYPE chartype;
    Vec2 v;
    Char( CHARTYPE t ) : Prop2D(), chartype(t) {
    }
    virtual bool charPoll(double dt) {return true;}
    virtual bool prop2DPoll(double dt) {
        loc += v*dt;
        return charPoll(dt);
    }
    static Char *hitAny(Char *tgt, float r, CHARTYPE hittype ) {
        Char *cur = (Char*)g_main_layer->prop_top;
        while(cur) {
            if( cur->chartype == hittype &&
                cur != tgt &&
                cur->to_clean == false &&
                cur->loc.x > tgt->loc.x-r && cur->loc.x < tgt->loc.x+r &&
                cur->loc.y > tgt->loc.y-r && cur->loc.y < tgt->loc.y+r ) {
                return cur;
            }
            cur = (Char*)cur->next;
        }
        return NULL;
    }
    bool isOutOfScreen() {
        float mgn=50;
        return ( loc.y < -SCRH/2-mgn || loc.y > SCRH/2+mgn || loc.x < -SCRW/2-mgn || loc.x > SCRW/2+mgn );
    }    
};


#define SAMPLE_COMMON_MAIN_FUNCTION(initfunc,updatefunc)  \
int main(int argc, char **argv ) { \
    sampleCommonInit(argc,argv); \
    initfunc(); \
    while( !sampleCommonDone() ) { \
        sampleCommonUpdate(); \
        updatefunc(); \
        sampleCommonRender(); \
    } \
    sampleCommonFinish(); \
    print("program finished"); \
    return 0; \
}



#endif
