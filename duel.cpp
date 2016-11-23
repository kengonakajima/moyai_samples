#include "sample_common.h"

///////////////////



class PC;
PC *g_pcs[2];

class Beam : public Char {
public:
    int faction;
    int power;
    Beam(int faction,Vec2 iniloc, Vec2 iniv, int power ) : Char(CHARTYPE_BEAM), faction(faction), power(power) {
        if(power>150)power=150;
        setLoc(loc);
        setLoc(iniloc);
        v = iniv;
        setDeck(g_base_deck);
        setIndex(ATLAS_BEAM);
        float s = 16 + power;
        if(s>120)s=120;
        print("power:%d scl:%f",power,s);
        setScl(s);
        if(iniv.x<0) setXFlip(true);
        g_main_layer->insertProp(this);        
    }
    virtual bool charPoll(double dt);
};


class ChargeParticle : public Prop2D {
public:
    Char *tgt;
    ChargeParticle(Vec2 loc,Char *tgt) : Prop2D(), tgt(tgt) {
        setLoc(loc);
        setDeck(g_base_deck);
        setIndex(ATLAS_BEAM_PARTICLE);
        setColor( Color(1,1,1,range(0.5,1)));
        setScl( range(16,48));
        g_effect_layer->insertProp(this);
    }
    virtual bool prop2DPoll(double dt) {
        Vec2 d = tgt->loc - loc;
        loc += d * dt *5;
        if( d.len() < 12 )return false;
        return true;
    }
};

class PC : public Char {
public:
    int faction;
    int direction; // -1:Left faced, 1:Right faced
    bool jumping;
    double charge_sound_at;
    int charge_count;
    double invincible_until;
    Vec2 tmpv;
    static const int MAXCHARGE=30;
    
    PC(int faction,int xdir) : Char(CHARTYPE_PC), faction(faction), charge_sound_at(0), charge_count(0), invincible_until(2) {
        setDeck(g_base_deck);
        setIndex( factionToBaseIndex(faction));
        setScl(48);
        g_main_layer->insertProp(this);
        direction = (faction==0) ? 1:0;        
        jumping = false;
        initLoc();
        walk(xdir);
    }
    void initLoc() {
        setLoc( (SCRW/2 - 200) * (faction==0?-1:1), 0 );    
    }
    virtual bool charPoll(double dt) {
        setXFlip( direction==1 );

        float ymin = -48-24;
        if(loc.y < ymin) {
            loc.y = ymin;
            v.y = 0;
            onLand();
        } else {
            float gravity = 1500;
            v.y -= gravity * dt;
        }
        if( invincible_until > accum_time ) {
            float a = (poll_count/5)%2;
            setColor(Color(1,1,1,a) );
        } else {
            setColor(Color(1,1,1,1));
        }
        int ind = factionToBaseIndex(faction);
        if(v.x!=0) {
            ind += (poll_count/4)%2;
        }
        setIndex(ind);

        // 
        loc += tmpv * dt;
        tmpv *= 0.8;

        float mgn = 20; 
        if( loc.x < -SCRW/2-mgn || loc.x > SCRW/2+mgn ) {
            initLoc();
            v*=0;
            tmpv*=0;
            invincible_until = accum_time+2;
        }
        if(v.x!=0) {
            int hit_pc_ind = faction ? 0 : 1;
            if(g_pcs[hit_pc_ind]->loc.len(loc)<30) {
                g_pcs[hit_pc_ind]->addKnockBack(Vec2(v.x*0.3,0));
                tmpv = -v;
            }
        }
        

       
        return true;
    }
    int factionToBaseIndex(int faction) {
        return faction==0?ATLAS_PC_RED_BASE:ATLAS_PC_BLUE_BASE;
    }
    void charge() {
        float r = 100;
        Vec2 dif( range(-1,1),range(-1,1));
        Vec2 at = loc+dif.normalize( range(r*0.5,r));
        new ChargeParticle(at,this);
        if( charge_sound_at < accum_time - 0.1 ) {
            g_charge_sound->play(0.4);
            charge_sound_at = accum_time;
        }
        charge_count++;
    }
    void tryJump() {
        if( jumping == false ) {
            jumping = true;
            if( charge_count<MAXCHARGE) {
                v.y = 250;
            } else {
                v.y = 500;
            }
        }
    }
    void onLand() {
        jumping = false;
    }
    void tryShoot() {
        if(charge_count>0) {
            Vec2 iniv(direction*500,0);
            new Beam(faction,loc,iniv,100);
            charge_count=0;
            g_shootbig_sound->play();
        }
    }
    void walk(int xdir ) { // xdir: -1:left 1:right
        v.x = xdir * 300;
        if(xdir<0) direction = -1;
        if(xdir>0) direction = 1;
    }
    void addKnockBack(Vec2 k) {
        tmpv += k;
    }
};


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

    
    g_pcs[0] = new PC(0,1);
    g_pcs[1] = new PC(1,-1);
    
}
void duelUpdate() {
    for(int i=0;i<2;i++) {
        if( g_keyboards[i]->getKey('W')) g_pcs[i]->tryJump();
        if( g_keyboards[i]->getKey(' ')) {
            if( g_pcs[i]->charge_count>=PC::MAXCHARGE) {
                g_pcs[i]->tryShoot();
            }
        }
        if( g_pcs[i]->charge_count<PC::MAXCHARGE) g_pcs[i]->charge();

        int xd=0;
        if( g_keyboards[i]->getKey('A')) {
            xd=-1;
        } else if( g_keyboards[i]->getKey('D')) {
            xd=1;
        } else {
            xd=0;
        }
        g_pcs[i]->walk(xd);
    }

    // Automation
    if(!g_rh) {
        if( range(0,1000)<20) g_pcs[1]->tryJump();
        if( range(0,1000)<30 && g_pcs[1]->charge_count>=PC::MAXCHARGE) g_pcs[1]->tryShoot();
    }
}


bool Beam::charPoll(double dt) {
    int hit_pc_ind = faction==0?1:0;
            
    if(g_pcs[hit_pc_ind]->loc.len(loc) < 20) {
        g_enemydie_sound->play();
        Vec2 kbv;
        kbv.x = (v.x>0?1.0f:-1.0f) * (power * power)/6;
        g_pcs[hit_pc_ind]->addKnockBack(kbv);
        return false;
    }

    // Hit on beams each other
    Prop *cur = g_main_layer->prop_top;
    while(cur) {
        Char *ch = (Char*)cur;
        if(ch!=this&& ch->chartype==CHARTYPE_BEAM && ch->loc.len(loc) < 30) {
            ch->to_clean = to_clean = true;
            g_machine_explode_sound->play();
            break;
        }
        cur=cur->next;
    }
    
    return true;
}


SAMPLE_COMMON_MAIN_FUNCTION(duelInit,duelUpdate, "duel");
