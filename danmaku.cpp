#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>



#ifndef WIN32
#include <strings.h>
#endif

#if defined(__APPLE__)
#define RETINA 2
#else
#define RETINA 1
#endif

#include "moyai/client.h"

MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
Camera *g_camera;

Texture *g_bmpfont_atlas;
TileDeck *g_bmpfont_deck;

ColorReplacerShader *g_replacer_shader;




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

    // replace white to random color
    g_replacer_shader->setColor( Color(0xF7E26B), Color( range(0,1),range(0,1),range(0,1),1), 0.02 );

    if( g_keyboard->getKey( 'Q') ) {
        print("Q pressed");
        g_game_done = true;
        return;
    }

    if( g_keyboard->getKey( 'Y' ) ) {
        g_moyai_client->batch_list.dump();
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
    
    double loop_end_at = now();
    double loop_time = loop_end_at - loop_start_at;
    double ideal_frame_time = 1.0f / 60.0f;
    if(loop_time < ideal_frame_time ) {
        double to_sleep_sec = ideal_frame_time - loop_time;
        int to_sleep_msec = (int) (to_sleep_sec*1000);
        if( to_sleep_msec > 0 ) sleepMilliSec(to_sleep_msec);
    }
}

void qstest(){
    SorterEntry tosort[5];
    tosort[0].val = 9;
    tosort[0].ptr = (void*)"aho";
    tosort[1].val = 5;
    tosort[1].ptr = (void*)"hoo";
    tosort[2].val = 1;
    tosort[2].ptr = (void*)"mog";
    tosort[3].val = 8;
    tosort[3].ptr = (void*)"tek";
    tosort[4].val = 10;
    tosort[4].ptr = (void*)"pak";
    
    quickSortF(tosort,0,5-1);
    for(int i=0;i<5;i++){
        print("val:%f %s",tosort[i].val, (char*)tosort[i].ptr );
    }
    assert( tosort[0].val == 1 );
    assert( strcmp( (char*)tosort[0].ptr, "mog" ) == 0 );
    assert( tosort[4].val == 10 );
    assert( strcmp( (char*)tosort[4].ptr, "pak" ) == 0 );    
    
}

void optest(){
    Vec2 a(1,2);
    Vec2 b(2,3);
    Vec2 c = a + b;
    assert( c.x == 3 );
    assert( c.y == 5 );
    assert( c == Vec2(3,5) );
    assert( c != Vec2(3,4) );
    assert( c != Vec2(3.1,5) );
    assert( !c.isZero() );
    c = Vec2(0,0);
    assert( c.isZero() );
    
    Vec2 d = a - b;
    assert( d.x == -1 );
    assert( d.y == -1 );

    Vec2 e = a * 2;
    assert( e.x == 2 );
    assert( e.y == 4 );

    Vec2 f(2,3);
    f *= 2;
    assert( f.x == 4);
    assert( f.y == 6);

    Vec2 g(2,4);
    g /= 2;
    assert( g.x == 1);
    assert( g.y == 2);

    Vec2 h(2,3);
    h += Vec2(1,2);
    assert( h.x == 3);
    assert( h.y == 5);    

    Vec2 k(2,3);
    k -= Vec2(5,5);
    assert( k.x == -3);
    assert( k.y == -2);    

    print("optest done");    
}

void comptest() {
    char buf[] = "hogehogefugafugahogefugapiyopiyo";
    char zipped[1024];
    char inflated[1024];
    int zipped_len = memCompressSnappy( zipped, sizeof(zipped), buf, (int)strlen(buf) );
    int inflated_len = memDecompressSnappy( inflated, sizeof(inflated), zipped, zipped_len );
    inflated[inflated_len] = '\0';
    print("snappy: %d bytes to %d byte", inflated_len, zipped_len );
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
    qstest();
    optest();
    comptest();

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
    g_window =  glfwCreateWindow( SCRW, SCRH, "demo2d", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    //    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(0); // set 1 to use vsync. Use 0 for fast screen capturing and headless
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

    // shader    
    g_replacer_shader = new ColorReplacerShader();
    if( !g_replacer_shader->init() ){
        print("can't initialize shader");
        exit(1);
    }

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

#if 0
    {
        Texture *sss = new Texture();
        sss->load( "./assets/dragon8.png" );
        for(int y=0;y<sss->image->height;y++) {
            for(int x=0;x<sss->image->width;x++) {
                Color c = sss->image->getPixel(x,y);
                prt( "%d ", (int)(c.r*255) );
            }
            prt("\n");
        }
        return 0;
    }
#endif

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
    g_base_deck->setSize(32,32,8,8 );
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
