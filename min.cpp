#include "sample_common.h"

void minimumInit() {
    Prop2D *p = new Prop2D(g_base_deck,ATLAS_MYSHIP,48,Vec2(0,0));
    g_main_layer->insertProp(p);
}
void minimumUpdate() {
}

SAMPLE_COMMON_MAIN_FUNCTION( minimumInit, minimumUpdate, "minimum");
