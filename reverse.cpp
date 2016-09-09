#include "sample_common.h"




/////////////////////
class Board : public Prop2D {
public:
    Grid *bg;
    Grid *fg;
    Board() : Prop2D() {
        setScl(48);
        bg = new Grid(8,8);
        addGrid(bg);
        fg = new Grid(8,8);
        addGrid(fg);

        setDeck(g_base_deck);
        setLoc( -8*48/2,-8*48/2);

        // initial
        fg->set(3,3,ATLAS_PIECE_BLACK);
        fg->set(4,4,ATLAS_PIECE_BLACK);        
        fg->set(3,4,ATLAS_PIECE_WHITE);
        fg->set(4,3,ATLAS_PIECE_WHITE);

        // bg
        for(int i=0;i<8;i++) {
            for(int j=0;j<8;j++) {
                bg->set(i,j,ATLAS_GREEN_BG);
            }
        }
        
        
        g_main_layer->insertProp(this);
    }
};

Board *g_board;
void reverseInit() {
    g_board = new Board();
    
}
void reverseUpdate() {
}


SAMPLE_COMMON_MAIN_FUNCTION(reverseInit,reverseUpdate);
