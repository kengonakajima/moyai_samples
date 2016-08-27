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

bool g_use_vsync = 1;

MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
Camera *g_camera;

Texture *g_bmpfont_atlas;
TileDeck *g_bmpfont_deck;


SoundSystem *g_sound_system;



int g_last_render_cnt ;

#define HEADLESS_SERVER_PORT 22222
RemoteHead *g_rh;

GLFWwindow *g_window;

bool g_game_done = false;

Keyboard *g_keyboard;
Mouse *g_mouse;
Pad *g_pad;


static const int SCRW=1024, SCRH=640;


class Ship;
Ship *g_myship;




//////////////////////////

class Beam : public Prop2D {
public:
    Vec2 v;    
    Beam(Vec2 loc, Vec2 v) : Prop2D(), v(v) {
        setLoc(loc);
        setDeck(g_base_deck);
        setUVRot(1);
        setXFlip(1);            
        setIndex(ATLAS_BEAM);
        g_main_layer->insertProp(this);        
    }
    virtual bool prop2DPoll(double dt) {
        loc += v*dt;
        return true;
    }
};

class Ship : public Prop2D {
public:
    double shoot_at;
    
    Ship() : Prop2D(), shoot_at(0) {
        setIndex( ATLAS_MYSHIP );
        setScl(48);
        setLoc(0,0);
        setDeck(g_base_deck);
        g_main_layer->insertProp(this);
    }
    virtual bool prop2DPoll(double dt) {
        Vec2 padvec;
        g_pad->getVec( &padvec );
        float speed = 240;
        Vec2 dloc = padvec * dt * speed;
        loc += dloc;

        setIndex( ATLAS_MYSHIP + ((poll_count/4)%2) );
        
        return true;
    }
    void tryShoot() {
        float interval = 0.1;
        if(accum_time>shoot_at+interval) {
            shoot();
            shoot_at = accum_time;
        }
    }
    void shoot() {
        float beam_speed = 1000;
        new Beam(loc+Vec2(0,20),Vec2(0,beam_speed)); // center

        float fan_interval = M_PI/16.0;
        for(int i=1;i<4;i++) {
            float rad = M_PI/2.0f + fan_interval*(float)i;
            new Beam(loc+Vec2(0,20),Vec2(::cos(rad),::sin(rad)).normalize(beam_speed));
            rad = M_PI/2.0f - fan_interval*(float)i;
            new Beam(loc+Vec2(0,20),Vec2(::cos(rad),::sin(rad)).normalize(beam_speed));
        }

    }
};

////////////////////////////

void gameUpdate(void) {
    glfwPollEvents();            
    g_pad->readKeyboard(g_keyboard);
    
    static double last_print_at = 0;
    static int frame_counter = 0;
    static int total_frame = 0;

    static double last_t=now();
    double t = now();
    double dt = t - last_t;
    last_t = t;
    double loop_start_at = t;
    
    frame_counter ++;
    total_frame ++;    

    if( g_keyboard->getKey('C') ) {
        Image *img = new Image();
        img->setSize( SCRW*RETINA, SCRH*RETINA );
        double st = now();
        g_moyai_client->capture(img);
        double et = now();
        bool ret = img->writePNG("_captured.png");
        double et2 = now();
        print("screen capture time:%f,%f", et-st,et2-et);
        
#if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE)
        assert(ret);
#endif        
        print("captured in _captured.png");
        delete img;
    }


    if( g_keyboard->getKey( 'Q') ) {
        print("Q pressed");
        g_game_done = true;
        return;
    }

    if( g_keyboard->getKey( 'Y' ) ) {
        g_moyai_client->batch_list.dump();
    }
    if( g_keyboard->getKey( GLFW_KEY_SPACE ) ) {
        g_myship->tryShoot();
    }

    if( g_mouse->getButton(0) ) {
        Vec2 cp = g_mouse->getCursorPos();
        print("mouse button 0 %f,%f", cp.x, cp.y );
    }
    

    int cnt;
    cnt = g_moyai_client->poll(dt);

    if(last_print_at == 0){
        last_print_at = t;
    } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d render:%d drawbatch:%d\n", frame_counter, cnt, g_last_render_cnt, g_moyai_client->batch_list.used  );
        frame_counter = 0;
        last_print_at = t;
    }

    if(g_rh) g_rh->heartbeat();

    // need sleep even when using vsync, because GLFW don't wait vsync when window is background
    double loop_end_at = now();
    double loop_time = loop_end_at - loop_start_at;
    double ideal_frame_time = 1.0f / 60.0f;
    if(loop_time < ideal_frame_time ) {
        double to_sleep_sec = ideal_frame_time - loop_time;
        int to_sleep_msec = (int) (to_sleep_sec*1000);
        if( to_sleep_msec > 0 ) {
            sleepMilliSec(to_sleep_msec);
        }
    }
}




void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}
void fbsizeCallback( GLFWwindow *window, int w, int h ) {
    print("fbsizeCallback: %d,%d",w,h);
	glViewport(0, 0, w, h);
}

void keyboardCallback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    g_keyboard->update( key, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    g_mouse->updateButton( button, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    g_mouse->updateCursorPosition( x,y);
}
void onConnectCallback( RemoteHead *rh, Client *cl) {
    print("onConnectCallback clid:%d",cl->id);
}
void onRemoteKeyboardCallback( Client *cl, int kc, int act, int modshift, int modctrl, int modalt ) {
    g_keyboard->update(kc,act,modshift,modctrl,modalt);
}
void onRemoteMouseButtonCallback( Client *cl, int btn, int act, int modshift, int modctrl, int modalt ) {
    g_mouse->updateButton( btn, act, modshift, modctrl, modalt );
}
void onRemoteMouseCursorCallback( Client *cl, int x, int y ) {
    g_mouse->updateCursorPosition(x,y);
}
void gameInit( bool headless_mode, bool enable_spritestream, bool enable_videostream ) {

    print("gameInit: headless_mode:%d spritestream:%d videostream:%d", headless_mode, enable_spritestream, enable_videostream );

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    

    g_sound_system = new SoundSystem();
    
    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, "danmaku", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    //    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(g_use_vsync); // set 1 to use vsync. Use 0 for fast screen capturing and headless
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    // controls
    g_keyboard = new Keyboard();
    glfwSetKeyCallback( g_window, keyboardCallback );
    g_mouse = new Mouse();
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );

    glfwSetFramebufferSizeCallback( g_window, fbsizeCallback );
    g_pad = new Pad();

    g_moyai_client = new MoyaiClient(g_window, SCRW, SCRH );
    
    if( headless_mode ) {
        Moyai::globalInitNetwork();
        g_rh = new RemoteHead();
        if( g_rh->startServer(HEADLESS_SERVER_PORT) == false ) {
            print("headless server: can't start server. port:%d", HEADLESS_SERVER_PORT );
            exit(1);
        }
        if( enable_spritestream ) g_rh->enableSpriteStream();
        if( enable_videostream ) g_rh->enableVideoStream(SCRW*RETINA,SCRH*RETINA,3);
        
        g_moyai_client->setRemoteHead(g_rh);
        g_rh->setTargetMoyaiClient(g_moyai_client);
        g_sound_system->setRemoteHead(g_rh);
        g_rh->setTargetSoundSystem(g_sound_system);
        g_rh->setOnKeyboardCallback(onRemoteKeyboardCallback);
        g_rh->setOnMouseButtonCallback(onRemoteMouseButtonCallback);
        g_rh->setOnMouseCursorCallback(onRemoteMouseCursorCallback);
        g_rh->setOnConnectCallback(onConnectCallback);
    }    

    g_viewport = new Viewport();
    g_viewport->setSize(SCRW*RETINA,SCRH*RETINA); // set actual framebuffer size to output
    g_viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

    g_main_layer = new Layer();
    g_moyai_client->insertLayer(g_main_layer);
    g_main_layer->setViewport(g_viewport);

    g_camera = new Camera();
    g_camera->setLoc(0,0);
    g_main_layer->setCamera(g_camera);
    
    // atlas
    g_base_atlas = new Texture();
    g_base_atlas->load("./images/base1024.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,42,24,24 );

    // Objects

    g_myship = new Ship();
    
}


void gameRender() {
    g_last_render_cnt = g_moyai_client->render();
}
void gameFinish() {
    glfwTerminate();
}




#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)

int main(int argc, char **argv )
{
    bool headless_mode=false, enable_videostream=false, enable_spritestream=true;
    for(int i=0;;i++) {
        if(!argv[i])break;
        if(strcmp(argv[i], "--headless") == 0 ) headless_mode = true;
        if(strcmp(argv[i], "--videostream") == 0 ) enable_videostream = true;
        if(strcmp(argv[i], "--skip-spritestream") == 0 ) enable_spritestream = false;
    }
        
    gameInit(headless_mode, enable_spritestream, enable_videostream);
    
    while( !glfwWindowShouldClose(g_window) && (!g_game_done) ){
        gameUpdate();       
        gameRender();
    }
    gameFinish();
    print("program finished");
    return 0;
}
#endif
