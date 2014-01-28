
// declare screen buffer var
int WIDTH=256, HEIGHT=256, *pixel ;
int fSIZE=(WIDTH*HEIGHT) ;

// include some usefull lib :P
#include "./Gdl/Gdl.hpp"			// my own graphic lib
#include "./myMix'r/myMixr.h"		// my poor&small sound mixer, for playing samples onto minifmod output..
#include "./s3lib/s3m.hpp"			// my own s3m parser...
#include "./xml.ib/xml.hpp"			// my own xml parser

// include game resource, yet in header & not in resource :D
// headers was created with gm4kr & mbin2h, two of my utils

#include "./gfx/s1font.h"			// a font ...
#include "./gfx/flashfont.h"		// a turtle on a tree
#include "./gfx/greenfont.h"		// another turtle, without tree
#include "./gfx/tsetBox.h"			// tileset, for message box corner
#include "./xm/effect.s3m.h"		// a small s3m who contain only samples, used for game sfx
#include "./gfx/case.h"				// game array tileset
#include "./gfx/caseboom.h"			// boom ?!
#include "./gfx/caseboom2.h"
#include "./gfx/marteau.h"			// hammer anim
#include "./gfx/chfont.h"			// big 0 to 9 char, to draw time left
#include "./gfx/cursor.h"			// red rotating cursor (who mark curent tile)
#include "./gfx/cursorSelected.h"	// the same, but in small & who mark all selected tiles
#include "./gfx/bombe.h"			// to tag a tile :)
#include "./gfx/flower.h"			// a sun ?
#include "./gfx/gomme.h"			// eraser anim
#include "./gfx/snow2.h"			// hey it's christmas !
#include "./gfx/note.h"				// music note gfx
#include "./gfx/songChoice.h"		// song choice button
#include "./xm/zikmu.rar.h"			// some included music in a rar file
#include "./xm/empty.xm.h"			// an empty xm, for no music song ..
#include "./gfx/zsmall.h"
#include "./gfx/pzcursor.h"
#include "./gfx/choosePz.h"
#include "./gfx/chooseNick.h"

int * choosePz ;
int * chooseNick ;

// im-ho-tep rulez

//#include "./gfx/icon.h"			//	avatar gfx, shi-fu-mi powered
//#include "./gfx/banana.h"			// yeah man !! roots banana pawa inside !

void drawBox(int x,int y,int sx,int sy,int tsize, int** tset,int bgcl)
{	// is blit pos unspecified, center box
		if(x<0)	x = (WIDTH - sx*tsize)>>1;
		if(y<0)	y = (HEIGHT - sy*tsize)>>1;
	// is box in screen ?
		if(x<0 || x+sx*tsize >= WIDTH) return ;
		if(y<0 || y+sy*tsize >= HEIGHT) return ;

	// box is in screen, now apply a fade with screen and bg color
	int *scr = xy2scr(x+(tsize>>2),y+(tsize>>2));

	int bgclr= ((bgcl>>16)&0xff)>>1;
	int bgclg=  ((bgcl>>8)&0xff)>>1;
	int bgclb=       (bgcl&0xff)>>1;
	
	for(int cy=y+(tsize>>2);cy<y+sy*tsize-(tsize>>2);cy++)
		{	int cx = tsize*(sx-1)+(tsize>>1);
			int *color = scr; scr += WIDTH ;
			while(cx--)
			{	int r = (((*color)>>16)&0xff)+bgclr ;
				int g = (((*color)>>8) &0xff)+bgclg ;
				int b = ((*color)&0xff)+bgclb      ;
				r>>=1;	g>>=1;	b>>=1;
				*color++ = (r<<16)|(g<<8)|b ;
			};
		};

	// screen under box is faded, now draw the box itself
	scr = xy2scr(x,y);	// not use a classic xy blit, direct use screen adress
	int xleft = sx-2, yleft=sy-2 ;
	// draw up left corner
		udrawGfm(tset[0],scr); scr += tsize ;
	// draw middle up border
		while(xleft--) { udrawGfm(tset[4],scr); scr += tsize ; }
	// draw up right corner
		udrawGfm(tset[1],scr);
	// draw left & right borders
		scr += tsize*(WIDTH-sx+1) ;
		{	u32 addValue  = tsize*WIDTH;
			u32 saddValue = tsize*(sx-1);
			while(yleft--)
				{	udrawGfm(tset[5],scr); // left
					udrawGfm(tset[7],scr+saddValue); // right
					scr += addValue ;
				};
		}
	// draw down left corner
		udrawGfm(tset[2],scr); scr += tsize ;
		xleft = sx-2 ;
	// draw middle down borders
		while(xleft--) { udrawGfm(tset[6],scr); scr += tsize ; }
	// draw down right corner
		udrawGfm(tset[3],scr);
}

int **mfont[] = { s0font,flashfont,greenfont }; // an array for fonts

#define zikNb 1

unsigned char *mzIk[] = { 0 } ;	// an array for muzik
		int mzIkSze[] = { 0 } ;	// and for their size

int stzIk = 0 ; /* start musik number */

