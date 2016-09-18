#include "sample_common.h"


class Ship;
Ship *g_myship;



///////////////////////

class Beam : public Char {
public:
    Beam(Vec2 loc, Vec2 to_v) : Char(CHARTYPE_BEAM) {
        v=to_v;
        setLoc(loc);
        setDeck(g_base_deck);
        setUVRot(1);
        setXFlip(1);            
        setIndex(ATLAS_BEAM);
        g_main_layer->insertProp(this);        
    }
    virtual bool charPoll(double dt) {
        if(isOutOfScreen())return false;
        return true;
    }
};

///////////////

class Ship : public Char {
public:
    double shoot_at;
    
    Ship() : Char(CHARTYPE_SHIP), shoot_at(0) {
        setIndex( ATLAS_MYSHIP );
        setScl(48);
        setLoc(0,0);
        setDeck(g_base_deck);
        g_main_layer->insertProp(this);
    }
    virtual bool charPoll(double dt) {
        Vec2 padvec;
        g_pad->getVec( &padvec );
        float speed = 240;
        Vec2 dloc = padvec * dt * speed;
        loc += dloc;
        if( loc.x<-SCRW/2+20 )loc.x = -SCRW/2+20;
        if( loc.x>SCRW/2-20 )loc.x = SCRW/2-20;
        if( loc.y<-SCRH/2+20 )loc.y = -SCRH/2+20;
        if( loc.y>SCRH/2-20) loc.y = SCRH/2-20;

        setIndex( ATLAS_MYSHIP + ((poll_count/4)%2) );

        if( Char::hitAny(this,10,CHARTYPE_BULLET) ||
            Char::hitAny(this,10,CHARTYPE_ENEMY) ) {
            g_shipdie_sound->play();
            loc = Vec2(0,0);
        }
         
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
        g_shoot_sound->play();
    }
};

class Bullet : public Char {
public:
    Bullet(Vec2 loc, Vec2 to_v) : Char(CHARTYPE_BULLET) {
        v = to_v;
        setIndex( ATLAS_ENEMY_BULLET );
        setScl(48);
        setLoc(loc);
        setDeck(g_base_deck);
        g_main_layer->insertProp(this);
    }
    virtual bool charPoll(double dt) {
        if( isOutOfScreen() ) { return false; }        
        return true;
    }
};

class ParticleEffect : public Prop2D {
public:
    Vec2 v;
    double clean_at;
    ParticleEffect(Vec2 loc,Vec2 iniv) : Prop2D() {
        float m = 200;
        v = iniv*0.3 + Vec2( range(-m,m), range(-m,m));
        clean_at = range(0.1,0.4);
        setLoc(loc);
        setDeck(g_base_deck);
        setIndex(ATLAS_BEAM_PARTICLE);
        setScl(range(24,48));
        setColor( Color(1,1,1,range(0.2,1)));
        g_effect_layer->insertProp(this);
    }
    virtual bool prop2DPoll(double dt) {
        loc += v*dt;
        if(accum_time>clean_at)return false;
        return true;
    }
    static void create( int n, Vec2 loc, Vec2 v) {
        for(int i=0;i<n;i++) {
            new ParticleEffect(loc,v);        
        }
    }        
};



class Enemy : public Char {
public:
    bool mad;
    int hp;
    double turn_at;
    double to_shoot_at;
    Enemy( Vec2 loc ) : Char(CHARTYPE_ENEMY), mad(false), hp(10), turn_at(0), to_shoot_at(0) {
        v.y = -200;
        setLoc(loc);
        setDeck(g_base_deck);
        setIndex(ATLAS_ENEMY_ANIMBASE);
        setScl(48);
        g_main_layer->insertProp(this);
    }
    virtual bool charPoll(double dt) {
        Char *hitchar = Char::hitAny(this,30,CHARTYPE_BEAM);
        if(hitchar) {
            hitchar->to_clean = true;
            ParticleEffect::create(irange(5,10),hitchar->loc,hitchar->v);
            //            print("hit! this:%p hit:%p hp:%d",this, hitchar,hp);
            g_beamhit_sound->play();
            hp--;
            mad = true;
            to_shoot_at = accum_time + range(0.1,0.3);
            if(hp<0) {
                g_enemydie_sound->play();
                return false;
            }
            setColor(Color(1,0.5,0.5,1));
        }
        if(turn_at<accum_time-0.5) {
            turn_at = accum_time;            
            if(mad) {
                float m = 300;
                v = Vec2(range(-m,m),range(-m,m)) ;                
            } else {
                Vec2 to_myship = g_myship->loc - loc;
                v = to_myship.normalize(150);
            }
        }
        if( mad && accum_time > to_shoot_at ) {
            to_shoot_at = accum_time + range(0.1,0.2);
            int n = (hp < 3) ? 5 : 1;
            for(int i=0;i<n;i++) {
                new Bullet(loc,loc.to(g_myship->loc+Vec2(range(-100,100),range(-100,100))).normalize(200));
            }
        }
        
        if( isOutOfScreen() ) { return false; }
        return true;
    }
};


////////////////////////////

void danmakuInit() {
    // Objects
    g_myship = new Ship();
}

void danmakuUpdate() {
    if( g_keyboards[0]->getKey( GLFW_KEY_SPACE ) ) {
        g_myship->tryShoot();
    }
    ////
    static double last_pop_at = 0;
    if( last_pop_at < g_myship->accum_time -1) {
        last_pop_at = g_myship->accum_time;
        int n = range(0,1) > 0.9 ? 5 : 1;
        for(int i=0;i<n;i++) new Enemy( Vec2( range(-SCRW/2,SCRW/2), SCRH/2) );
    }
}


SAMPLE_COMMON_MAIN_FUNCTION( danmakuInit, danmakuUpdate, "danmaku" );
