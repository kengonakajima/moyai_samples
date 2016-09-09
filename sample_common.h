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

#endif
