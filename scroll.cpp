#include "sample_common.h"

////////////

#define FIELDSIZE 128
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
    Client *cl;
    Camera *camera;
    Keyboard *keyboard;
    Pad *pad;
    Mouse *mouse;
    Viewport *viewport;
    float zoom_rate;
    PC( Client *cl) : Char(CHARTYPE_PC), cl(cl) {
        assert(cl);
        
        setDeck(g_base_deck);
        setIndex(ATLAS_PC_RED_BASE);
        setScl(48);
        g_main_layer->insertProp(this);
        setPriority(10000);
        

        camera = new Camera(cl);
        g_main_layer->addDynamicCamera(camera);            
        keyboard = new Keyboard();
        pad = new Pad();
        mouse =  new Mouse();
        viewport = new Viewport(cl);
        zoom_rate = 1;
        modZoom(0);
    }
    virtual bool charPoll(double dt) {
        Vec2 padvec;
        pad->getVec( &padvec );
        float speed = 700;
        Vec2 dloc = padvec * dt * speed;
        loc += dloc;
        if( dloc.x<0) setXFlip(false);
        if( dloc.x>0 ) setXFlip(true);
        camera->setLoc(loc); // camera is null when single_screen
        if(keyboard->getKey('Z')) modZoom(0.05);
        if(keyboard->getKey('X')) modZoom(-0.05);
        //        print("%d: %f,%f",id,loc.x,loc.y);
        return true;
    }
    void modZoom(float d) {
        zoom_rate+=d;
        viewport->setScale2D(SCRW*zoom_rate,SCRH*zoom_rate);
    }
};


/////////




#define CHUNKNUM (FIELDSIZE/Chunk::SZ)
Chunk *g_chunks[CHUNKNUM][CHUNKNUM]; // [y][x]



//////
Chunk *allocateChunk(int chx, int chy) {
    Chunk *chk = new Chunk(chx,chy);
    return chk;
}
void setupChunks() {
    for(int chy=0;chy<CHUNKNUM;chy++){
        for(int chx=0;chx<CHUNKNUM;chx++) {
            if(g_chunks[chy][chx]==NULL) g_chunks[chy][chx] = new Chunk(chx,chy);
        }
    }
}

void initField() {
    for(int y=0;y<FIELDSIZE;y++) {
        for(int x=0;x<FIELDSIZE;x++) {
            g_field[y][x] = Grid::GRID_NOT_USED;
            if( range(0,1) < 0.05 ) g_field[y][x] = ATLAS_GROUND_ROCK;
            if(y==0||y==(FIELDSIZE-1)||x==0||(x==FIELDSIZE-1) ) g_field[y][x] = ATLAS_GROUND_BLOCK;
        }
    }
}

void updateZoom() {
    float minzoom = 0.5, maxzoom = 2;
    static float zoom_rate = 1;
    
    if(g_mouse->getButton(0)) {
        zoom_rate -= 0.05;
        if( zoom_rate < minzoom ) zoom_rate = minzoom;
    }
    if(g_mouse->getButton(1)) {
        zoom_rate += 0.05;
        if( zoom_rate > maxzoom ) zoom_rate = maxzoom;
    }
    g_viewport->setScale2D( SCRW / zoom_rate, SCRH / zoom_rate );
    
}

void scrollConnect(RemoteHead *rh, Client *cl) {
    print("scrollConnect clid:%d",cl->id);
    PC *pc = new PC(cl);
    pc->setLoc(200,200);
    g_main_layer->addDynamicCamera(pc->camera);
    g_main_layer->addDynamicViewport(pc->viewport);
}
PC *findPCByClient(Client *cl) {
    Prop *cur = g_main_layer->prop_top;
    while(cur){
        Char *ch=(Char*)cur;
        if(ch->chartype == CHARTYPE_PC ) {
            PC *pc =(PC*)ch;
            if(pc->cl==cl) {
                return pc;
            }
        }
        cur=cur->next;
    }
    return NULL;
}
void scrollRemoteKeyboard(Client *cl, int kc, int act) {
    PC *pc = findPCByClient(cl);
    if(pc) {
        pc->keyboard->update(kc,act,0,0,0);
        pc->pad->readKeyboard(pc->keyboard);
    }
}
void scrollRemoteMouseButton(Client *cl, int btn, int act) {
    PC *pc = findPCByClient(cl);
    if(pc) {
        pc->mouse->updateButton(btn,act,0,0,0);
    }
}

void scrollInit() {
    g_game_connect_callback = scrollConnect;
    g_remote_keyboard_callback = scrollRemoteKeyboard;
    g_main_layer->setCamera(NULL);
    g_effect_layer->setCamera(NULL);
 
    initField();
    
    for(int chy=0;chy<CHUNKNUM;chy++){
        for(int chx=0;chx<CHUNKNUM;chx++) {
            g_chunks[chy][chx]=NULL;
        }
    }
    allocateChunk(0,0);
    float zr=0.2f;
    g_viewport->setScale2D( SCRW/zr, SCRH/zr );
    g_camera->setLoc(SCRW*zr,SCRH*zr);
    setupChunks();
}
void scrollUpdate() {
    
}

SAMPLE_COMMON_MAIN_FUNCTION(scrollInit,scrollUpdate, "scroll");
