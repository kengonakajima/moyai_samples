#include "sample_common.h"

////////////

#define FIELDSIZE 512
int g_field[FIELDSIZE][FIELDSIZE]; // [y][x]

int getCell(int x, int y) {
    return g_field[y][x];
}

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

        loadCell();
    }
    void loadCell() {
        for(int y=0;y<SZ;y++) {
            for(int x=0;x<SZ;x++) {
                int cell = getCell(chx*SZ+x,chy*SZ+y);
                g->set(x,y,cell);
            }
        }
    }
    virtual bool prop2DPoll(double dt) {
        return true;
    }
};

/////////
class PC : public Char {
public:
    PC() : Char(CHARTYPE_PC) {
        setDeck(g_base_deck);
        setIndex(ATLAS_PC_RED_BASE);
        setScl(48);
        g_main_layer->insertProp(this);
        setPriority(10000);
    }
    virtual bool charPoll(double dt) {

        Vec2 padvec;
        g_pad->getVec( &padvec );
        float speed = 700;
        Vec2 dloc = padvec * dt * speed;
        loc += dloc;
        if( dloc.x<0) setXFlip(false);
        if( dloc.x>0 ) setXFlip(true);
        return true;
    }
};


/////////




#define CHUNKNUM (FIELDSIZE/Chunk::SZ)
Chunk *g_chunks[CHUNKNUM][CHUNKNUM]; // [y][x]

PC *g_pc;

//////
Chunk *allocateChunk(int chx, int chy) {
    Chunk *chk = new Chunk(chx,chy);
    return chk;
}

void initField() {
    for(int y=0;y<FIELDSIZE;y++) {
        for(int x=0;x<FIELDSIZE;x++) {
            if( range(0,1) < 0.05 ) g_field[y][x] = ATLAS_GROUND_ROCK;
            if(y==0||y==(FIELDSIZE-1)||x==0||(x==FIELDSIZE-1) ) g_field[y][x] = ATLAS_GROUND_BLOCK; else g_field[y][x] = Grid::GRID_NOT_USED;
        }
    }
}
void scrollInit() {

    initField();
    
    g_pc = new PC();
    g_pc->setLoc(100,100);
    
    for(int chy=0;chy<CHUNKNUM;chy++){
        for(int chx=0;chx<CHUNKNUM;chx++) {
            g_chunks[chy][chx]=NULL;
        }
    }
    allocateChunk(0,0);
}
void scrollUpdate() {
    g_camera->setLoc(g_pc->loc);
}

SAMPLE_COMMON_MAIN_FUNCTION(scrollInit,scrollUpdate, "scroll");
