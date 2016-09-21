#include "sample_common.h"

///////////////////
class Chunk : public Prop2D {
public:
    static const int SZ=8;
    Grid *g;
    int chx,chy;
    Chunk( int chx, int chy ) : Prop2D(),chx(chx),chy(chy) {
        g = new Grid(SZ,SZ);
        addGrid(g);
        g->setDeck(g_base_deck);
        setScl(48);
        setLoc(chx*SZ*48,chy*SZ*48);
        g_main_layer->insertProp(this);

        init();
    }
    void init() {
        for(int y=0;y<SZ;y++) {
            for(int x=0;x<SZ;x++) {
                if(x==y) {
                    g->set(x,y,ATLAS_GROUND_BLOCK);
                } else {
                    g->set(x,y,ATLAS_GROUND_ROCK);
                }
                
            }
        }
    }
    virtual bool prop2DPoll(double dt) {
        return true;
    }
};

/////////



#define FIELDSIZE 512
#define CHUNKNUM (FIELDSIZE/Chunk::SZ)
Chunk *g_chunks[CHUNKNUM][CHUNKNUM]; // [y][x]

//////
Chunk *allocateChunk(int chx, int chy) {
    Chunk *chk = new Chunk(chx,chy);
    return chk;
}

void scrollInit() {
    for(int chy=0;chy<CHUNKNUM;chy++){
        for(int chx=0;chx<CHUNKNUM;chx++) {
            g_chunks[chy][chx]=NULL;
        }
    }
    allocateChunk(0,0);
}
void scrollUpdate() {
    
}

SAMPLE_COMMON_MAIN_FUNCTION(scrollInit,scrollUpdate, "scroll");
