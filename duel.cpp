#include "sample_common.h"

///////////////////



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
}
void duelUpdate() {
}
SAMPLE_COMMON_MAIN_FUNCTION(duelInit,duelUpdate);
