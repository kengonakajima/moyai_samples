#include "sample_common.h"


bool g_use_vsync = 1;

MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Layer *g_effect_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
Texture *g_ascii_atlas;
TileDeck *g_ascii_deck;
Camera *g_camera;


SoundSystem *g_sound_system;
Sound *g_shoot_sound;
Sound *g_beamhit_sound;
Sound *g_enemydie_sound;
Sound *g_shipdie_sound;
Sound *g_charge_sound;
Sound *g_shootbig_sound;
Sound *g_machine_explode_sound;

int g_last_render_cnt ;

#define HEADLESS_SERVER_PORT 22222
RemoteHead *g_rh;

GLFWwindow *g_window;

bool g_game_done = false;


Keyboard *g_keyboards[2]; // Duel uses 2 keyboards
Pad *g_pad;
Mouse *g_mouse;

void (*g_game_connect_callback)(RemoteHead *rh, Client *cl);
void (*g_remote_keyboard_callback)(Client *cl, int kc, int act);

const int SCRW=800, SCRH=450;

void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}
void fbsizeCallback( GLFWwindow *window, int w, int h ) {
    print("fbsizeCallback: %d,%d",w,h);
#ifndef __linux__
    glViewport(0, 0, w, h);
#endif    
}


void localKeyboardCallback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    g_keyboards[0]->update( key, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    g_mouse->updateButton( button, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    g_mouse->updateCursorPosition( x,y);
}
void onConnectCallback( RemoteHead *rh, Client *cl) {
    print("onConnectCallback clid:%d",cl->id);
    if(g_game_connect_callback) g_game_connect_callback(rh,cl);
}

void onRemoteKeyboardCallback( Client *cl, int kc, int act, int modshift, int modctrl, int modalt ) {
    int kbd_index = cl->id % 2;
    //    print("onRemoteKeyboardCallback: ind:%d kc:%d",kbd_index,kc);
    g_keyboards[kbd_index]->update(kc,act,modshift,modctrl,modalt);
    if(g_remote_keyboard_callback) g_remote_keyboard_callback(cl,kc,act);
}
void onRemoteMouseButtonCallback( Client *cl, int btn, int act, int modshift, int modctrl, int modalt ) {
    g_mouse->updateButton( btn, act, modshift, modctrl, modalt );
}
void onRemoteMouseCursorCallback( Client *cl, int x, int y ) {
    g_mouse->updateCursorPosition(x,y);
}

bool sampleCommonDone() {
    return glfwWindowShouldClose(g_window) ||g_game_done;
}

void sampleCommonInit(int argc, char **argv, const char *title ) {
    bool headless_mode=false, enable_videostream=false, enable_spritestream=false;
    for(int i=0;;i++) {
        if(!argv[i])break;
        if(strcmp(argv[i], "--videostream") == 0 || strcmp(argv[i],"--vs")==0 ) {
            headless_mode = true;
            enable_videostream = true;
        }
        if(strcmp(argv[i], "--spritestream") == 0 || strcmp(argv[i],"--ss")==0 ) {
            headless_mode = true;
            enable_spritestream = true;
        }
    }
    if( headless_mode && enable_spritestream==false && enable_videostream == false ) {
        print("headless mode with no stream setting. add --videostream or --spritestream");
        exit(1);
    }
    print("sampleCommonInit: headless_mode:%d spritestream:%d videostream:%d title:%s", headless_mode, enable_spritestream, enable_videostream, title );


#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    


    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, title, NULL, NULL );
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
    g_keyboards[0] = new Keyboard();
    g_keyboards[1] = new Keyboard();    
    glfwSetKeyCallback( g_window, localKeyboardCallback );
    g_mouse = new Mouse();
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );

    glfwSetFramebufferSizeCallback( g_window, fbsizeCallback );
    g_pad = new Pad();

    g_moyai_client = new MoyaiClient(g_window, SCRW, SCRH );

    // sounds
    g_sound_system = new SoundSystem();
    g_shoot_sound = g_sound_system->newSound( "./sounds/shoot.wav");
    g_enemydie_sound = g_sound_system->newSound( "./sounds/enemydie.wav");
    g_beamhit_sound = g_sound_system->newSound( "./sounds/beamhit.wav");
    g_shipdie_sound = g_sound_system->newSound( "./sounds/shipdie.wav");
    g_charge_sound = g_sound_system->newSound( "./sounds/charge.wav");
    g_shootbig_sound = g_sound_system->newSound( "./sounds/shootbig.wav");
    g_machine_explode_sound = g_sound_system->newSound( "./sounds/machine_explo.wav");

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

    g_effect_layer = new Layer();
    g_moyai_client->insertLayer(g_effect_layer);
    g_effect_layer->setViewport(g_viewport);

    g_camera = new Camera();
    g_camera->setLoc(0,0);
    g_main_layer->setCamera(g_camera);
    g_effect_layer->setCamera(g_camera);

    // atlas
    g_base_atlas = new Texture();
    g_base_atlas->load("./images/base1024.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,42,24,24 );

    g_ascii_atlas = new Texture();                                                                                           
    g_ascii_atlas->load( "./images/asciibase256.png" );                                                                      
    g_ascii_deck = new TileDeck();                                                                               
    g_ascii_deck->setTexture(g_ascii_atlas);
    g_ascii_deck->setSize( 32,32, 8,8 );
}

int getFirstClientIndex() {
    if(!g_rh)return 0;
    Client *cl = g_rh->getFirstClient();
    if(cl) return cl->id % 2; else return 0;
}
    
void sampleCommonUpdate() {
    glfwPollEvents();    
    g_pad->readKeyboard(g_keyboards[getFirstClientIndex()]);
    
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

    if( g_keyboards[0]->getKey('C') ) {
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


    if( g_keyboards[0]->getKey( 'Q') ) {
        print("Q pressed");
        g_game_done = true;
        return;
    }

    if( g_keyboards[0]->getKey( 'Y' ) ) {
        g_moyai_client->batch_list.dump();
    }

    if( g_mouse->getButton(0) ) {
        Vec2 cp = g_mouse->getCursorPos();
        //        print("mouse button 0 %f,%f", cp.x, cp.y );
    }

    ////

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
void sampleCommonRender() {
    g_last_render_cnt = g_moyai_client->render();
}

void sampleCommonFinish() {
    glfwTerminate();
}



StatusLine::StatusLine() : Prop2D() {
    setScl(24);
    cg = new CharGrid(40,1);
    cg->setDeck(g_ascii_deck);
    addGrid(cg);
}
