#include "sample_common.h"




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

Board *g_board;
void reverseInit() {
    g_board = new Board();
    
}
void reverseUpdate() {
}


SAMPLE_COMMON_MAIN_FUNCTION(reverseInit,reverseUpdate);
