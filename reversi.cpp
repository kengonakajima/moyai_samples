#include "sample_common.h"


class Board;
Board *g_board;
StatusLine *g_statusline;

int g_turn; // mod2=0:black, =1:white

#define IS_BLACK_TURN(n) ( (n%2) == 0 )

/////////////////////
class Board : public Prop2D {
public:
    Grid *bg;
    Grid *fg;
    static const int BLACK=0, WHITE = 1;
    Board() : Prop2D() {
        setScl(48);
        bg = new Grid(8,8);
        addGrid(bg);
        fg = new Grid(8,8);
        addGrid(fg);

        setDeck(g_base_deck);
        setLoc( -8*48/2,-8*48/2);

        // initial
        setPiece(3,3,BLACK);
        setPiece(4,4,BLACK);
        setPiece(3,4,WHITE);
        setPiece(4,3,WHITE);        

        // bg
        for(int i=0;i<8;i++) {
            for(int j=0;j<8;j++) {
                bg->set(i,j,ATLAS_GREEN_BG);
            }
        }
        g_main_layer->insertProp(this);
    }
    void setPiece(int x,int y,int col) {
        fg->set(x,y,col==WHITE?ATLAS_PIECE_WHITE:ATLAS_PIECE_BLACK);        
    }
};

void updateStatusLine() {
    CharGrid *cg = g_statusline->GetCharGrid();
    cg->printf(0,0,Color(1,1,1,1), IS_BLACK_TURN(g_turn) ? "NEXT: BLACK" : "NEXT: WHITE" );    
}


void reverseInit() {
    g_board = new Board();

    g_statusline = new StatusLine();
    g_statusline->setLoc(SCRW/2-300,SCRH/2-100);
    g_effect_layer->insertProp(g_statusline);

    updateStatusLine();
}
void reverseUpdate() {
    if( g_mouse->getToggled(0) ) {
        g_mouse->clearToggled(0);
        Vec2 mpos = g_mouse->getCursorPos();
        mpos -= Vec2(SCRW/2,-SCRH/2);
        mpos.y = SCRH-mpos.y;
        Vec2 relloc = mpos - g_board->loc;
        int bx = relloc.x / 48;
        int by = relloc.y / 48;
        print("mouse 0 press:%f,%f bpos:%d,%d",mpos.x,mpos.y, bx,by);

        if(IS_BLACK_TURN(g_turn)) {
            g_board->setPiece(bx,by,Board::BLACK);
        } else {
            g_board->setPiece(bx,by,Board::WHITE);
        }
        g_turn++;
        updateStatusLine();        
    }
}


SAMPLE_COMMON_MAIN_FUNCTION(reverseInit,reverseUpdate);
