
/*
	  picros's
	  qe
	s'racros

		#gni# ?!

	yet you finally escape to this f**ki*g title :)
*/

//#define _debug

#include "tinyptc.h"			// the framebuffer TinyPTC, by Gaffer
#include "../lib/minifmod.h"	// the xm replayer minifmod, by Firelight Technologies
#include "minifmodio.cpp"		// xm lib, i/o callback code
#include <windows.h>

#include "./Gdl/Gdl.hpp"

char * saveFile=0 ;
u32 saveFileSize=0;
int curentPlayer=0;
char*curentPlayerName=0;

void playS3mSample(int splNb,int chn=255,int period=0,int octave=4,int volume=64,int differed=0,int maxTime=0xffff);
void updateSaveFile(void);

#include "./defineSomeThing.h"	// some thing declared and define ...

struct anim *fleur, *cur, *scur, *mapcur, *hammer, *gom, *pen, *boom=0, *currentCursor;//, *banane; // declare some pointer to anim
int cursorx=10, cursory=30 ;
int mouseIsInArray = 0 ;
int timeLeft = 0;
int timeLeftDraw = 0 ;
int canLoseTime = 0;
int paused=0;
u32 canClick=0;
void drawBoom(void);

#include "../easyp.lvl.h" // the 40 10*10 levels of easy mode, from the game boy's game..

void showSomeSnow(void);
void playARandomSong(void);

int readLastSong=0; // read previous song flag
int deadSong=0;		// no song flag

// return -1 on error, else return option choosed, argument is only one string who contain all options text separated by a '|' ---> "this|is|an|exemple" for a 4 entry menu    sorry not title for now :P
int drawMenu(char*entry,const char * entryStr = ".%s.",const char * separator=0,int**font0=*mfont,int**font1=mfont[1],void(*customBlit)(void)=0);

// 2 vars to manage bg color change
int colorChange = 0 ;//42+rand()%42 ;
int bgColor = 0 ;

// a small hack to draw an 'choose your name' picture in drawMenu
u32 drawChooseNick = 0 ;

void setPausedState(int state)
{	static int onPauseVolume = 64 ;
	if(!state)
	{	// to seen a fade when quit pause
		colorChange = 64+(rand()%32);
		bgColor = 0 ;
	//	onPauseVolume = FMUSIC_setWantedVolume(24);
	}//	else FMUSIC_setWantedVolume(onPauseVolume);
	paused = state ;
}

int keyUp(int k)
{	static int lastkey[256];
	if(k>255) { memset(lastkey,0xff,256*4); return 0; }
	int up=0;
	up = (key[k] && lastkey[k]==0);
	lastkey[k] = key[k] ;
	return up;
}

#define killKeyFront() keyUp(0xffff)

char * lvlString = "entering level  -r043v" ;

// some string to insult this bad player :D
#define deadlvlStringNb 21
char * deadlvlString[] = {	"damn !!",
							"no luck...",
							"increase your brain!",
							"try another day",
							"it is bad!",
							"outch!",
							"why ?!!",
							"maybe another time",
							"doh!",
							"no comment ...",
							"retry needed...",
							"you much better sleep ..",
							"loser one time loser every time",
							"fudes !!!",
							"i cant call you brainMaster..",
							"any better player ?",
							"have you read rules ?!",
							"search GOOD block!!",
							"too hard for you ?",
							"make better !",
							"brain before click"
						};

char * curentLvlString ;
int			 lvlStringPos = 0xffff ;
int			 lvlStringDec = 0 ;
#define		 lvlStringOffset 15

char * sngString ;
char * sngName   ;
int			 sngStringPos = -10 ;// = WIDTH*2 ;
u32			 sngStringWay = 0 ; // 0 up, 1 down

void resetSongTitlePos(void)
{	sngStringWay = 0 ;
	sngStringPos = -10;
}

void launchSongTitlePosDown(void)
{	sngStringWay = 666 ;
}

FMUSIC_MODULE *mod[zikNb] ; MEMFILE zk[zikNb] ;

char  * savePath ;

void iniZik(void)
{  FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell, extraMix, playARandomSong, playARandomSong);
if( !((mzIk[0])&&(mzIkSze[0])) ) return;
  for(int n=0;n<zikNb;n++)
   {	zk[n].pos=0 ; zk[n].length=mzIkSze[n]; zk[n].data=mzIk[n];
		mod[n] = FMUSIC_LoadSong((char*)&(zk[n]), NULL);
   };
   FMUSIC_PlaySong(mod[stzIk]);
   char * name = (char*)mzIk[stzIk] ;
   name += 16 ;

	memcpy(sngName,name,21);
	//sngName[20]=0;
   //strcpy(sngName,name);
	sngName[21]=0;
	sprintf(sngString,"%s",sngName);
	resetSongTitlePos();
}

struct puzzleFile
{	char * file;
	char * name;
	char * author;
	int    numberOfPuzzle;
	int  * puzzleSize;
	char **puzzles; 
};

inline int char2int(char * n)
{	register int nb = *n++, temp	;
		nb -= 0x30  ;
		//nb *= 10	;
		nb <<= 1	;
		temp = nb	;
		temp <<= 2	;
		nb += temp	;
		nb += *n	;
		nb -= 0x30	;
	return nb		;
}

char *	currentMap ;
int		currentMapSizeX ;
int		currentMapSizeY ;
char *	maskedMap=0 ;

struct puzzleFile * loadPuzzleFile(u8*file)//const char*path)
{	struct puzzleFile * pzfile = (struct puzzleFile *)allocAndAddFree(sizeof(struct puzzleFile));
	pzfile->file   = (char*)file ;//(char*)loadFile(path);
	pzfile->name   = pzfile->file + 13 ; (pzfile->name)[19]=0;
	pzfile->author = pzfile->file + 33 ; (pzfile->author)[14]=0;
	pzfile->numberOfPuzzle = char2int((pzfile->file)+48);
	pzfile->puzzleSize = (int*)allocAndAddFree(pzfile->numberOfPuzzle*12); // sizex 4o, sizey 4o, pointer to puzzle 4o
	int index = 50 ;
	char * curentPz = (char*)(pzfile->puzzleSize);
	curentPz+= (pzfile->numberOfPuzzle)<<3 ;
	pzfile->puzzles = (char**)curentPz;
	curentPz = (pzfile->file)+50+((pzfile->numberOfPuzzle)<<2);
	int i=0,j=0,k,l;
#ifdef _debug
	printl("%s\nthere is %i puzzles write by %s\n",pzfile->name,pzfile->numberOfPuzzle,pzfile->author);
#endif
	for(i=0;i<(pzfile->numberOfPuzzle)<<1;)
	{		(pzfile->puzzles)[j++] = curentPz ;
			k = char2int(pzfile->file+index);
			(pzfile->puzzleSize)[i++] = k ;
			#ifdef _debug
				printl("\n puzzle %i   \t%i*",j,k);
			#endif
			l = k ; index+=2;
			k = char2int(pzfile->file+index);
			index+=2;
			l *= k ;  curentPz += l ;
			(pzfile->puzzleSize)[i++] = k ;
			#ifdef _debug
				printl("%i",k);
			#endif
	};
	return pzfile;
}

const char *hexa="%x",*decimal="%i",*ddecimal="%2i";

/***********************/
/* ../~ game option .. */

u8 option[] = {1,1,1,1,255,64,16,1} ; // default options ...

#define infoWay			(option[0])		// 1:classic	0:inverted
#define autoLoadLastPz	(option[1])		// 1:yes		0:no
#define	arrayTheme		(option[2])		// 0 1 or 2
#define	difficult		(option[3])		// 0:tapette	1:human			2:killer
#define songToPlay		(option[4])		// 0:no song	n:song n		255:random
#define songVolume		(option[5])		// from 0 to 128
#define tileSize		(option[6])		// 10 16 or 32
#define playerIsALamer	(option[7])		// enable hint ?

/***********************/

void resizeWindowedWindows(void)
{	oldW = 50 + ((currentMapSizeX>>1)+currentMapSizeX)*10 ;
	oldH = 35 + ((currentMapSizeY>>1)+currentMapSizeY)*10 ;
	resetScreenSize();
}

void resizeWindows(void)
{	WIDTH  = 50 + ((currentMapSizeX>>1)+currentMapSizeX)*10 ;
	HEIGHT = 35 + ((currentMapSizeY>>1)+currentMapSizeY)*10 ;
	resetScreenSize();
}

void resizeWindowsToGame(void)
{	int oldw, oldh ;
	oldh = HEIGHT  ;
	oldw = WIDTH   ;
	resizeWindows();
	resetScreenSize();
	if(oldh == HEIGHT && oldw == WIDTH) return;
	ptc_close(0);
	ptc_open(0,&WIDTH,&HEIGHT);
}

void showInfo(char*start, int way, int dpos, int pos, int **font)
{	char *end = start, *ptr = start ;
	int continuous=0;
	
	int nb = currentMapSizeY ;

	if(way!='h') { nb=currentMapSizeX; end+=currentMapSizeX-1 ;  }
	else		   end+=currentMapSizeX*(currentMapSizeY-1) ;
	
	if(infoWay) ptr=end;

	while( nb-- )
	{ if(*ptr=='0' || !*ptr) {	if(continuous)	{	if(continuous>9) continuous+=7 ; // if >9 show value in hexa
										if(way=='h')	drawGfm(font[16+continuous],dpos,pos);
											else		drawGfm(font[16+continuous],pos,dpos);
										pos-=10; continuous=0;
									}
				} else continuous++;
			
			// yet too lasy to unroll
		if(infoWay) {	if(way!='h') --ptr; else ptr -= currentMapSizeX;	}
		else		{	if(way!='h') ++ptr; else ptr += currentMapSizeX;	}
	
	};

	if(continuous)	{	if(continuous>9) continuous+=7 ; // if value >9 show with letter
						if(way=='h')	drawGfm(font[16+continuous],dpos,pos);
							else		drawGfm(font[16+continuous],pos,dpos);
					}
}

/*  listen this great piece of sound .. 'ty hji oe' at badloop.com
		perfect to play picros's 		*/

#define numberOfCaseBoom 100
int		 caseBoomNumber = 0 ;
int		*caseBoomEnable = 0 ;
int		*caseBoomX=0, *caseBoomY=0;
struct	anim *caseBoom = 0 ;

int mapNb=0 ; // the map counter, first map is 0

struct puzzleFile * puzzle ;
int goodBlocInCurrentMap ;
int youLose;

// return pointer in saveFile, where are stored player stat (try & best time)
u16 * retrevePlayerStat(u32 player, u32 map)
{	u16 * ptr = (u16*)(saveFile+4) ;
	u32 playerNb = *ptr++ ; if(player >= playerNb) return 0 ;
	u32 mapNb	 = *ptr++ ; if(map >= mapNb) return 0 ;
	ptr += playerNb*15;//+= (playerNb<<4)-playerNb ; // 
	//if(*ptr++ != ) printlr("bad check !!! : %2s .. %2s .. %2s",ptr[-2],ptr[-1],*ptr);
	//else printlr("good check !!!");
	//ptr++ ;
	ptr += map*(1+playerNb*2) ; // set on good puzzle
	if(*ptr != *(u16*)"**") printlr("bad check !!! : %2s",*ptr);
	ptr += player*2 ; // set on good player
	return ++ptr ;
}

void loadMap(u32 nb, u32 opt=1)
{	currentMap = (puzzle->puzzles)[nb] ;
	int *size = puzzle->puzzleSize ;
	size += (nb*2) ;// nb-=3 ;
	currentMapSizeX = *size++ ;
	currentMapSizeY = *size ;
	youLose=0;
	caseBoomNumber = 0 ;
	timeLeft = (30<<16); // time is 30 minuts at start
	memset(caseBoomEnable,0,numberOfCaseBoom*4*3);
	memset(maskedMap,0,1024);
	goodBlocInCurrentMap=0 ;
	for(int n=0;n<currentMapSizeX*currentMapSizeY;n++) // count number of good bloc in the level..
		if(currentMap[n] && currentMap[n]!='0') goodBlocInCurrentMap++ ;
	if(!opt) return ;
	u16 * tryNb = retrevePlayerStat(curentPlayer,nb);
	if(tryNb) *tryNb = (*tryNb) + 1 ;
	writeFile(savePath,saveFile,saveFileSize);
	/*printlr("update try number of level %i for player %i",nb,curentPlayer);
	if(tryNb) printlr("it's now %i try",*tryNb);
	else log("internal error");*/
	// update try number in the save file
/*	char * ptr = saveFile ;	u16 playerNb = *(u16*)(saveFile+4);
	ptr += 8 + playerNb*30 + nb*(2+4*playerNb);
	ptr+=2 ; ptr += (4*curentPlayer) ; ptr++ ;
	u16*tryNb = (u16*)ptr ;	*tryNb = *tryNb + 1 ;
	writeFile(savePath,saveFile,saveFileSize);*/
}

/* masked map tileset..
	0 masked
	1 masked and marked as bad
	2 unmasked and good
	3 unmasked and bad
  */

#define xspace 10
#define yspace 10
int xinfopos=0, yinfopos=0 ;
u32 lineCursor = 0xff, colonCursor = 0xff ;

void showMapInfo()
{	static int oldW=0;
	if(oldW!=WIDTH)
		{	xinfopos = (WIDTH  - (currentMapSizeX>>1)*10)>>1 ;
			yinfopos = (HEIGHT - (currentMapSizeY>>1)*10)>>1 ;
			oldW=WIDTH;
		}
	int cursorx, cursory ;
	if(mouseIsInArray)
		{	cursorx = mousex - xinfopos ;
			cursorx /= xspace ; // yet not really fast :|
			cursory = mousey - yinfopos ;
			cursory /= yspace ;
		}
	if(!mouseIsInArray)
	{	for(int x=0;x<currentMapSizeY;x++)
			showInfo(currentMap+currentMapSizeX*x,0,yinfopos+yspace*x,xinfopos-xspace,*mfont);

		for(int y=0;y<currentMapSizeX;y++)
			showInfo(currentMap+y,'h',xinfopos+xspace*y,yinfopos-yspace,*mfont);

		for(x=0;x<currentMapSizeX;x++)
			for(y=0;y<currentMapSizeY;y++)
			{	register int tmp ;
				tmp = maskedMap[y*currentMapSizeX+x] ;
				drawGfm(zcase[tmp],xinfopos+x*xspace-1,yinfopos+y*yspace-1) ;
			};
	} else {
		for(int x=0;x<currentMapSizeY;x++)
			showInfo(currentMap+currentMapSizeX*x,0,yinfopos+yspace*x,xinfopos-xspace,mfont[2*(x==cursory)]);

		for(int y=0;y<currentMapSizeX;y++)
			showInfo(currentMap+y,'h',xinfopos+xspace*y,yinfopos-yspace,mfont[2*(y==cursorx)]);

		for(x=0;x<currentMapSizeX;x++)
			for(y=0;y<currentMapSizeY;y++)
			{	register int tmp ;
				tmp = maskedMap[y*currentMapSizeX+x] ;
				drawGfm(zcase[tmp],xinfopos+x*xspace-1,yinfopos+y*yspace-1) ;
			};
		
		playAnim(&cur,xinfopos+cursorx*xspace-1,yinfopos+cursory*yspace-1,0);
	}
}

/* why this fucking title ?  picros's - pqr aei - s'racro
     sorry..  */

char string[8]; // a string,    on the beatch ?

#define startColor 0xef00ff

//extern int * data2Gfm(unsigned char *data);

#include "./unrarlib.h"			// unrarlib, for read and uncompress rar v2 archive

//extern int inFullScreen; // is the game in full screen mode ?

extern void playAnim(struct anim **b, int x, int y, int way) ;

void iniCaseBoom(void)
{	if(caseBoomEnable)	free(caseBoomEnable);
	if(caseBoom)		free(caseBoom) ;
	if(boom)			free(boom) ;
	caseBoomNumber = 0 ;
	
	switch(arrayTheme)
	{	case 0	: boom = setAnim( caseboom,8,60,1); break ;
		default	: boom = setAnim(_caseboom,8,60,1); break ;
	}
	
	caseBoomEnable = (int*)allocAndAddFree(numberOfCaseBoom*4*3); // group into only one alloc ..
	memset(caseBoomEnable,0,numberOfCaseBoom*4*3);
	caseBoomX = caseBoomEnable ;
	caseBoomX +=  numberOfCaseBoom ;
	caseBoomY = caseBoomX ;
	caseBoomY +=  numberOfCaseBoom ;
	
	caseBoom = (struct anim*)allocAndAddFree(numberOfCaseBoom * sizeof(struct anim));
	//if(!boom) return ;
	for(int n=0;n<numberOfCaseBoom;n++)
		memcpy(&(caseBoom[n]),boom,sizeof(struct anim));
}

void foundFreeBoom(int *nb)
{	for(int n=0;n<numberOfCaseBoom;n++)
		if(caseBoomEnable[n] == 0){ *nb = n ; return ; }
		showMsg("use a rand boom",0);
	*nb = rand()%numberOfCaseBoom ; // else if there is not a free anim, use a rand ..
	if(caseBoomEnable[*nb]) caseBoomNumber-- ;
}

void addBoom(int tilePos)
{	int nb,y=0;
	if(tilePos < 0 || tilePos >= currentMapSizeX*currentMapSizeY) return ;
	//if(maskedMap[tilePos]>1) return ;
	foundFreeBoom(&nb);
	if(caseBoomEnable[nb]) showMsg("foundFreeBoom error :|",0);
	while(tilePos >= currentMapSizeX) { tilePos -= currentMapSizeX ; y++ ; }
	if(y>currentMapSizeY) return ;
		// tilePos now get x coordonate
	caseBoomX[nb] = (tilePos*xspace) + xinfopos - 7 ; // compute pos in screen
	caseBoomY[nb] =		  (y*yspace) + yinfopos - 6 ;
	caseBoomEnable[nb] = 1 ;
	caseBoom[nb].curentFrm = 0 ;
	caseBoom[nb].lastTime = tick ;
	caseBoomNumber++;
}
 
void drawBoom(void)
{	int boomDrawed=0;
	if(!caseBoomNumber) return ;
	struct anim * tmp ;
	int nb = numberOfCaseBoom ;
	for(int n=0;n<numberOfCaseBoom;n++)
		if(caseBoomEnable[n])
			{	tmp = &(caseBoom[n]) ;
				//if(tmp->curentFrm < tmp->frmNumber)
				{	playAnim(&tmp,caseBoomX[n],caseBoomY[n]);
					if(tmp->curentFrm >= tmp->frmNumber -1)
					{	caseBoomEnable[n] = 0 ; caseBoomNumber-- ; tmp->curentFrm = 255 ; }
				}	if(++boomDrawed >= nb) return ;
			}
}

void doesShowCursor(void)
{	if(!paused && mouseIsInArray) SetCursor(0) ;
}

int isMouseInArray(void)
{		if(mousex > xinfopos && mousex < xinfopos+currentMapSizeX*xspace)
			if(mousey > yinfopos && mousey < yinfopos+currentMapSizeY*yspace)
					return 1 ;
		return 0 ;
}

u32 sfxVolume = 64 ;

int unmask(int tilePos)
{	if(maskedMap[tilePos]>1) return 0 ; // need to be masked to unmask :)
	// reset anim to broke tile with the hammer !
		if(canLoseTime) { currentCursor = hammer ; resetAnim(&hammer); }
	// add boom animate
		addBoom(tilePos); 
	// does we broke a good tile ?
		if(currentMap[tilePos] && currentMap[tilePos] != '0')
			{				maskedMap[tilePos] = 2 ; // set as unmasked and good
							if(goodBlocInCurrentMap) goodBlocInCurrentMap-- ;
							if(!goodBlocInCurrentMap && !youLose)
									{	// we just win
											canClick=0; // disable click
										// save progression into save file
										updateSaveFile();
									}
							return 1 ;
			}	else	{	maskedMap[tilePos] = 3 ; // set as unmasked and bad
							// remove some time ..
							if(canLoseTime)
							{	static int losePawa=0, lastTLeft=0 ;
								if(losePawa<8) losePawa<<=1;
								if(timeLeft > lastTLeft) losePawa=1; // reset lose value
								if(timeLeft>>16 >= losePawa)
									timeLeft = ((timeLeft)&0xffff) + (((timeLeft>>16)-losePawa)<<16) ;
								else timeLeft = 0 ;
								if(timeLeft <= 0)
									{	timeLeft=0; // game over sfx
										if(!youLose)
										{	static char * olddeadlvlstr = 0 ; // to don't have two time the same string
											playS3mSample(0,255,0,4,sfxVolume, 0,8);
											playS3mSample(0,255,2,4,sfxVolume, 8,8);
											playS3mSample(0,255,4,4,sfxVolume,16,8);
											playS3mSample(0,255,8,4,sfxVolume,24,8);
											playS3mSample(0,255,0,5,sfxVolume,32,8);
											playS3mSample(0,255,4,5,sfxVolume,40,8);

											playS3mSample(0,255,0,4,sfxVolume, 0,12);
											playS3mSample(0,255,2,4,sfxVolume, 8,12);
											playS3mSample(0,255,4,4,sfxVolume,16,12);
											playS3mSample(0,255,8,4,sfxVolume,24,12);
											playS3mSample(0,255,0,5,sfxVolume,32,12);
											playS3mSample(0,255,4,5,sfxVolume,40,12);
											do	{ curentLvlString = deadlvlString[rand()%deadlvlStringNb] ;
												} while(olddeadlvlstr == curentLvlString);
											olddeadlvlstr = curentLvlString ;
											lvlStringPos = WIDTH+10 ; // update and launch info text
											lvlStringDec = 0 ;
											youLose=1;	canClick=0;
										}	
									} else { // lose time sfx
										playS3mSample(0,255,0,4,sfxVolume, 0,10);
										playS3mSample(0,255,8,3,sfxVolume-16,3,10);
									}
								lastTLeft = timeLeft ;
							}
						}
		return 0 ;
}

inline void switch2anim(struct anim **a, struct anim *d)
{  *a=d ; if(d!=hammer) resetAnim(a); else d->curentFrm = 3; }

char * mouse2tile(void)
{	int cursorx = mousex - xinfopos ;	cursorx /= xspace ;
	int cursory = mousey - yinfopos ;	cursory /= yspace ;
	return &(maskedMap[cursory*currentMapSizeX+cursorx]);
}

void rightclick(char *curentTile, int value=0xff) 
{	if(value == 0xff || *curentTile > 1) value = *curentTile ;
	switch(value)
		{	case 0 : /* masked */
				*curentTile = 1 ;	switch2anim(&currentCursor,pen); // mark the tile as bad
			break;
			case 1 : /* marked as bad */
				*curentTile = 0 ; switch2anim(&currentCursor,gom); // unmark the tile
			break ;
			default : return ;
		}
}

int leftclick(char *curentTile) 
{	switch(*curentTile)
		{	case 0 : /* masked */
				return unmask(curentTile-maskedMap);	// or unmask tile (left click)
			break;
			default : return 1 ;
		}
}

void drawGame(void);

void onclick(int value, int button) /* button : 0 for left (broke) and 1 for right (mark as bad) */
{	if(paused || !mouseIsInArray) return ;
	if(!button) leftclick(mouse2tile());
	else		rightclick(mouse2tile());
}


void onmove(void)
{	mouseIsInArray = isMouseInArray();
	doesShowCursor();
}

void animSwitch(struct anim **b);

void animSwitch(struct anim **b)
{	if(*b == gom || *b == pen) switch2anim(b,hammer);
}

ArchiveList_struct	*songList=0,*lst=0 ;

int startClickX=255, startClickY=255 ;

void showCursor(void)
{	if(!mouseIsInArray) return ;	// show cursor only if we are in array
	
	if(startClickX != 255 && startClickY != 255) // draw selection, if there is one
		{	int x = xinfopos+startClickX*xspace+2 ;
			int y = yinfopos+startClickY*yspace+1 ;
			playAnim(&scur,x,y,0);
			int cursorx, cursory,n,m=0 ;
			cursorx = mousex - xinfopos ;	cursorx /= xspace ;
			cursory = mousey - yinfopos ;	cursory /= yspace ;
			int ysame = (startClickY == cursory) ;
			int	xsame = (startClickX == cursorx) ;
			if(xsame && ysame) goto end ;
			if(ysame) // show selected line
			{	n=startClickX-cursorx ; if(n<0) n = -n ; m=n;
				if(startClickX > cursorx)	while(n--) { x-=xspace ; playAnim(&scur,x,y,0); }
					else					while(n--) { x+=xspace ; playAnim(&scur,x,y,0); }
			} else if(xsame){	// or selected colon
								n=startClickY-cursory ; if(n<0) n = -n ; m=n;
								if(startClickY > cursory)	while(n--) { y-=xspace ; playAnim(&scur,x,y,0); }
									else					while(n--) { y+=xspace ; playAnim(&scur,x,y,0); }
							} //else	startClickX=startClickY=255 ;
				if(m++) if(m<10) drawGfm(chfont[m],mousex,mousey);
					//drawInt(m,10,30,decimal,0,*mfont); // show selected size
		}
	
	end:
		if(currentCursor == hammer) playAnim(&currentCursor,mousex-4 ,mousey-28,0); // hammer
		 else	if(currentCursor == pen)	playAnim(&currentCursor,mousex-22,mousey-10,0); // bombe
				 else	playAnim(&currentCursor,mousex-16,mousey-4,0); // gomme
}

int fadeSpeed = 13;//16 ;
void applyFade(void)
{	int *p = pixel, n=HEIGHT*WIDTH ;
	while(n--) // the fade
       { if(*p==0xffffff) *p=0; // to not work with white color
		 if(*p) {
           int r = ((*p)>>16)&0xff ;
           int g = ((*p)>>8) &0xff ;
           int b = (*p)&0xff       ;
           if(r) { if(r>fadeSpeed) r-=(fadeSpeed>>1) ; else r-- ; }
		   if(g) { if(g>fadeSpeed) g-=fadeSpeed ; else g-- ; }
		   if(b) { if(b>21 && b>fadeSpeed) b-=(fadeSpeed>>1) ; else { if(b>1) b-=2 ; else b=0; } }
           *p++ = (r<<16)|(g<<8)|b ;
         } else p++ ;
       };
}

u32 sendSomeColor = 1 ;

void drawSongTitle(void)
{	if(sngStringPos != 0xffff)
	{	int h = HEIGHT-sngStringPos ;
		static u8 flip = 0 ;
		drawGfm(*note,mprints(-1,h,sngString)-16,h-4);
		//	else drawGfm(*note,mprints(-1,h,"untitled")-16,h-4);
		//drawText(sngString,sngStringPos,HEIGHT-16,0,*mfont);
		if(!sngStringWay) // up
		{	// up text, or launch wait if max is reatched
			if(++flip == 2)
			{	if(sngStringPos > 16) sngStringWay++ ;
				 else sngStringPos++;
				flip=0;
			}
		} else { // wait or down
				if(++sngStringWay > 300)
				{	if(++flip == 2)
					{	if(--sngStringPos < -10)
							{ sngStringPos = 0xffff; sngStringWay=0 ; }
						flip=0;
					}
				}
		};
	}
}

void onFocus(void);

void drawSongButton(void)
{	for(u32 n=0;n<3;n++) drawGfm(songChoice[n],WIDTH-25+8*n,4);
	static int oldMouse = 0 ;
		if(mousex > (WIDTH-26) && mousex < WIDTH) // end click on song button
			if(mousey > 4 && mousey < 12)
				{	int x = mousex - (WIDTH-26);
					x >>= 3 ; if(x<3) drawGfm(songChoice[x+3],WIDTH-25+8*x,4);
					
					if(!mouseLeftState && oldMouse)
					  switch(x)
						{	case 2 :
								launchSongTitlePosDown();
								FMUSIC_setVolumeUpdateSpeed(10);
								FMUSIC_setWantedVolume(0);
							break ;

							case 0 :
								launchSongTitlePosDown();
								FMUSIC_setVolumeUpdateSpeed(10);
								FMUSIC_setWantedVolume(0);
								readLastSong=1;
							break ;

							case 1 :
								static char noSound=0 ;	noSound^=1;
								launchSongTitlePosDown();
								FMUSIC_setVolumeUpdateSpeed(10);
								FMUSIC_setWantedVolume(0);

								deadSong=readLastSong=0;
								if(noSound)	deadSong=1;
								 else		readLastSong=1;
							break ;
						};
					oldMouse = mouseLeftState;
					return ;
				}

		if(!mouseLeftState && oldMouse)	
			if(paused){ SetCursor(0); onFocus(); } // any click will exit pause
	oldMouse = mouseLeftState;
}

void drawGame(void)
{	// global key command
	if(key[kup]) // volume up
		{	FMUSIC_SlideVolume(1);
			if(sngStringPos == 0xffff) resetSongTitlePos(); else sngStringWay=0;
		}

	if(key[kdown]) // volume down
		{	if(sngStringPos == 0xffff) resetSongTitlePos(); else sngStringWay=0;
			FMUSIC_SlideVolume(0);
		}

	if(keyUp(kleft)) // another song
		{	launchSongTitlePosDown();
			FMUSIC_setVolumeUpdateSpeed(10);
			FMUSIC_setWantedVolume(0);
			readLastSong=1;
		}

	if(keyUp(kright)) // previous song
		{	launchSongTitlePosDown();
			FMUSIC_setVolumeUpdateSpeed(10);
			FMUSIC_setWantedVolume(0);
		}

	if(keyUp(kesc)) // edit puzzle
		{	// 

		}

	if(paused) { /*showSomeSnow();*/ applyFade(); }

		static int*wave=0;
		if(!wave)
		{	wave = (int*)allocAndAddFree(2048*4);
			for(int n=0;n<1024;n++)
			{	int red = 0x6b+n; if(red>0xff) red = 0xff;
				wave[n] = (red<<16)|(0x2b<<8)|0xc6 ;
			};
		}

		if(!paused)
		{	for(int c=0;c<WIDTH*HEIGHT;c+=2)
				{	int b = (c%128)+(rand()%64);//(128+rand()%2) ;
					//if(++d>8) d=0 ;
					pixel[c] = (b<<16)|(b<<8)|b ;
				};

			for(c=1;c<WIDTH*HEIGHT;c++)
				{	//u32 z = pixel[c] ;
					//if(z==0xffffff)
					pixel[c] = (c%colorChange) + bgColor ;
					c++ ;
				};
		}

		if(paused)
		if(curentMixBf)
		{	int*color = (int*)curentMixBf,x,y=HEIGHT>>1;
			if(color)
			{	static int oldWidth = 0 ; static int wratio,hratio ;
				if(WIDTH!=oldWidth)	{	wratio = (curentMixSz<<15)/WIDTH;
										if(HEIGHT<255)	hratio = (127<<16)/((HEIGHT>>1) + 100);
										else			hratio = (((HEIGHT>>1)-100)<<16)/127;	
										oldWidth=WIDTH;
									}
					for(int n=0;n<WIDTH;n++)
						{	x = (n*wratio)>>16 ;
							x = (color[x]>>23) ;
							x = (x*hratio)>>16 ;
							x = y+x ; if(x<0)x=0; else if(x>=HEIGHT) x=HEIGHT-1;
							ligne(n,y,n,x,wave);
						};
			}
		}

		if(!paused)
		if(curentMixBf) // show music, vertical way
		{	int*color = (int*)curentMixBf;
			if(color)
			{	static int oldWidth = 0 ; static int wratio,hratio ;
				if(WIDTH!=oldWidth)	{	hratio = (curentMixSz<<15)/(HEIGHT-60);
										wratio = (32<<16)/(127);
										oldWidth=WIDTH;
									}
			
					int * ypos = &(pixel[26+54*WIDTH]) ;
					for(int n=0;n<HEIGHT-60;n++)
						{	register int size ;
							size = (n*hratio)>>16 ;  // select good value in output wave
							size = color[size]>>24 ; // take only wanted part..
							size = (size*wratio)>>16 ;	 // stretch value to output wanted zone..
							if(size>0)
									for(int z=0;z<size;z++) ypos[z] = wave[(z<<2)+64] ;	
							else	for(int z=0;z>size&&z>-26;z--) ypos[z] = wave[((-z)<<2)+64] ;
							//else	
							//memcpy(ypos,wave,size*4);
							ypos += WIDTH;
							//ligne(n,y,n,x,wave);
						};
			}
		}

	if(paused)
	{	showSomeSnow();
		/* draw & manage info & song text .. */
			if(lvlStringPos != 0xffff)
			{	drawText(curentLvlString+lvlStringDec,lvlStringPos,20,0,*mfont);
				if(curentLvlString[lvlStringDec]==0) lvlStringPos = 0xffff ; // stop draw if flower destruct all of the text ^^
				else if(--lvlStringPos < -8) { lvlStringPos+=8 ; lvlStringDec++; }
			}

		if(keyUp(kspace)) setPausedState(0);

		drawSongTitle();
		drawSongButton();
		//drawInt(FMUSIC_GetTime(),10,HEIGHT-32,decimal,0,*mfont);
		//drawInt(FMUSIC_GetProgress(),100,HEIGHT-32,decimal,0,*mfont);
		
		//prints(130,HEIGHT-32,"%i/%i",retreve('o'),retreve(0));
FMUSIC_GetProgress();
		return ;
	} else { FMUSIC_GetProgress();	showSomeSnow(); }// applyFade(); }

	//memset(pixel,0xff,(WIDTH*HEIGHT)*2);
	//memset(&pixel[(WIDTH*HEIGHT)>>1],0,(WIDTH*HEIGHT)*2);

/* blit the background */

	//if(sendSomeColor)

	if(!colorChange)
	{	colorChange = 96-(rand()%32);
		//sendSomeColor = 0 ;
	}

	{ int r,g,b,c=0 ; static int nflip=0;

		r = (bgColor>>16)&0xff ;
		g = (bgColor>> 8)&0xff ;
		b = bgColor & 0xff	 ;

		if(r<255) ++r;
		if(g<255) ++g;

		if(nflip) { if(b<255) ++b ; } nflip^=1;
/*		
		if(g<255) ++g;
		else  if(r<255) ++r;
*/
		if(r<255) ++r;
		else  if(g<255) ++g;
				else  if(b<255) ++b ;
						else c=1;

		if(!c)
		{	//if(r+g+b > 512) bgColor = 0 ;	else
				bgColor = (r<<16)|(g<<8)|b ;
		}
		else	{	bgColor = 0;
					colorChange += (4+(rand()%24)) ;
					if(colorChange<64 || colorChange>128) colorChange=96-(rand()%42);
				}
		//prints(60,10,"%i.%i.%i %i",r,g,b,colorChange);
	}

	int *scr = pixel ;
	int bgcl = 0x169fbc ;
	int bgclr= ((bgcl>>16)&0xff)>>1;
	int bgclg=  ((bgcl>>8)&0xff)>>1;
	int bgclb=       (bgcl&0xff)>>1;

	for(int cy=0;cy<HEIGHT;cy++)
		{	int cx = WIDTH ;
			int *color = scr; scr += WIDTH ;
			while(cx--)
			{	int r = (((*color)>>16)&0xff)+bgclr ;
				int g = (((*color)>>8) &0xff)+bgclg ;
				int b = ((*color)&0xff)+bgclb      ;
				r>>=1;	g>>=1;	b>>=1;
				*color++ = (r<<16)|(g<<8)|b ;
			};
		};

/* show game array .. */
	showMapInfo();

/* draw explosions */
	drawBoom();
/* draw & manage info & song text .. */
	if(lvlStringPos != 0xffff)
	{	drawText(curentLvlString+lvlStringDec,lvlStringPos,20,0,*mfont);
		if(curentLvlString[lvlStringDec]==0) lvlStringPos = 0xffff ; // stop draw if flower destruct all of the text ^^
		else if(--lvlStringPos < 32) { lvlStringPos+=8 ; lvlStringDec++; }
	}

	drawSongTitle();
	drawSongButton();

/* draw flower ^__^ */
	playAnim(&fleur,12,10);
/* draw & manage time left */
	drawInt(timeLeftDraw>>16 ,8,40,ddecimal,0,chfont,'0');
	drawInt(timeLeftDraw&0xff,28,40,ddecimal,0,chfont,'0');

	if((timeLeftDraw&0xff) < 10) drawGfm(*chfont,28,40);
		// does we need to show the time separator ?
	static int drawSeparator = 0 ;
	if(drawSeparator) drawGfm(chfont[10],23,40);
	/* down time left .. */
	static int lastDrawTime=0 ;
	tick = GetTickCount();
	if(timeLeft && tick - lastDrawTime > 500)
		{	drawSeparator ^= 1 ;
			if(drawSeparator)
			{	if(!(timeLeft&0xffff)) // if second counter is at 0
					{			timeLeft = (((timeLeft>>16)-1)<<16) + 59 ; // down about 1 minut
					}	else	if(timeLeft>0) timeLeft-- ;
			}	lastDrawTime = tick ;
		}

	/* made timeLeftDraw going to timeLeft value */
	{		{	u32 s,cs,m,cm ;
				s	= timeLeft&0xffff;
				m	= timeLeft>>16;
				cs	= timeLeftDraw&0xffff;
				cm	= timeLeftDraw>>16;
				
				if(m < cm) // need remove time
					{	register u32 z = 1+4*(m!=cm)+4*(cm-m>4)+4*(cm-m>8); // yet not unrolled :)
						for(u32 n=0;n<z && cs;n++) cs-- ;
						if(cs<=0) { cm--; cs=59; }
					} else if(cm < m) // need adding time
								{	register u32 z = 1+4*(m!=cm)+4*(m-cm>4)+4*(m-cm>8);
									for(u32 n=0;n<z && cs<60;n++) cs++ ;
									if(cs>59) { cm++; cs=0; }
								} else // compute second
					{	if(s > cs) cs++; else if(cs > s) cs--;
					}

				timeLeftDraw = (cm<<16)|cs;
			}
	}

	static int oldMouse = 0 ;
	if(!mouseLeftState && oldMouse)
	{	if(mousex>10&&mousex<38)	/* does end click on flower ? */
			if(mousey>10&&mousey<38)
				setPausedState(1);
	}
	oldMouse = mouseLeftState;

//	playAnim(&banane,10,82);
}

	/*  picore save file format ..

  all not text value are unsigned ..

	0	4	"psve"		it's a picore save..
	4	2	playerNb	number of players
	6	2	pzNb		number of puzzles
	8

 offset : 8
	for(eatch players)

	0	2	"**"		0x2a2a    42 pawa :)
	2	2	lastLvl		curent level (first unfinished)
	4	8	options		player options..
	12	18	name		player name.. 17 char + 0 ;
	30

 offset : 8 + playerNb*30 ;

	for(eatch puzzles)

	0	2	"**"		file check
	
		for(eatch players)

		0	4	player stats for the level
				
				0	2	number of try
				2	2	best time left in second, 0 if never finished
				4

 offset : 8 + playerNb*30 + pzNb * (2+4*playerNb);

	"****"	end of file

  so if filesize != [8 + playerNb*30 + pzNb * (2+4*playerNb) + 4], file  is corupt !

*/

void updateSaveFile(void)
{	u16 * stat = retrevePlayerStat(curentPlayer,mapNb);
	if(stat)
		{	u32 curentTimeLeft = ((timeLeft>>16)*60)+(timeLeft&0xff);
			++stat; // set on bestTimeLeft
			if(!*stat || *stat < curentTimeLeft) *stat = curentTimeLeft ;
			writeFile(savePath,saveFile,saveFileSize);
		}
	
	char*ptr = saveFile + 8 + 30*curentPlayer + 2 ;
	if(mapNb>38){ drawMenu("congratulation!||you just finish|the|easy mode !||stay tuned|for next release!|||by ferof!         |    procvor.free.fr"); mapNb=-1 ; }
	*(u16*)ptr = mapNb + 1 ;

	#ifdef _debug
		printlr("save new map number of player %i, it's %i",curentPlayer,mapNb+1);
	#endif
}

void loadCurrentMapNb(void)
{	char*ptr = saveFile + 8 + 30*curentPlayer + 2 ;
	mapNb = *(u16*)ptr ;
	//if(mapNb>39) mapNb = -1 ;
	//mapNb=39;
	#ifdef _debug
		printlr("load map number of player %i, it's %i",curentPlayer,mapNb);
	#endif
}

void setOptions(void)
{	u32 leftMouse = mouseLeftState ;
	int exit=0, oldWidth=0;
	char * optString = (char*)malloc(2048);
	#define optNb 8
//	u32 opts[optNb] ;
	u32 optMax[optNb]		=	{ 2,2,0,3,0,0,0,2 };
	const char * opt[]		=	{ "info way","auto load last puzzle","difficult","need help ?" };
	const char * way[]		=	{ "classic","inverted" };
	const char * autoload[]	=	{ "yes","no" };
	const char * diff[]		=	{ "tapette","human","killer" };
	const char * hint[]		=	{ "no","yeah too lame" };

	while(!exit)
	{	if(WIDTH!=oldWidth) // does resolution just change ?
		{	//textPx = ((WIDTH - 13*8)>>1)   - 1 ;
			//textPy = ((HEIGHT - (3*8))>>1) + 8 ;
			oldWidth=WIDTH;
		}
		// show music playback
		setPausedState(1); drawGame();

		if(!leftMouse && mouseLeftState)

		leftMouse = mouseLeftState ;
	}

	free(optString);
}

void askforName(char*name,int maxSize)
{	if(!name) return;
	*name=0;
	setGdlfont(*mfont);
	int textPx,textPy;
	const char * charset = "abcdefghijklm\0nopqrstuvwxyz\0 -!0123456789";
	int leftMouse = mouseLeftState ;
	int exit=0;
	*name=0 ;
	int oldWidth=0;
	while(!exit)
	{	if(WIDTH!=oldWidth) // does resolution just change ?
		{	textPx = ((WIDTH - 13*8)>>1)   - 1 ;
			textPy = ((HEIGHT - (3*8))>>1) + 8 ;
			oldWidth=WIDTH;
		}
		// show music playback
		setPausedState(1); drawGame();
		// show box, charset and entered name for now..
		drawBox(-1,-1,8,6,16,tsetBox,0x2a2aff);
		prints(-1,textPy-34,"your name ?");
		prints(-1,textPy-16,".%s.",name);
		prints(textPx,textPy,charset); 
		prints(textPx,textPy+10,charset+14);
		prints(textPx,textPy+20,charset+28);
		prints(-1,textPy+32,"del        ok");
		// is mouse on box ?
		if(mousey>=textPy && mousey<=textPy+42)
			if(mousex>=textPx && mousex<textPx+8*13)
			{	// mouse is on box, draw selected char/command if there is one
				int y = (mousey-textPy)/10 ;
				int x = (mousex-textPx)>>3;
				if(y<3 && x<14)
					drawGfm((mfont[1])[charset[x+y*14]-' '],textPx+x*8,textPy+y*10);
				else	{	setGdlfont(mfont[1]);
							if(y>=3 && x<3)  prints(-1,textPy+32,"del          ");
								else
							if(y>=3 && x>10) prints(-1,textPy+32,"           ok");
							setGdlfont(mfont[0]);
						}
				// does left mouse button just clicked ?
				if(!leftMouse && mouseLeftState)
					{	char*p=name; while(*p)p++;
						if(y<3 && x<14) // we click on a char..
						{	if(p-name == maxSize) p-- ;
							*p++ = charset[x+y*14] ; *p=0;
							//if(p-name>=maxSize) exit++;
						} else {	// else maybe click on 'del' or 'ok'
							if(y>=3 && x<3) { if(p>name) *--p=0; playS3mSample(3,255,2,2,64); } // del
								else
									if(y>=3 && x>10) { exit++; playS3mSample(1,255,2,3,64); } // ok/cancel
						}
					}
			}
		leftMouse = mouseLeftState ;
			ptc_update(pixel);
	};  setPausedState(0);
}

char * createSaveFile(int pzNb, u32 * size=0)
{	int playerNb = 1 ;
	int fileSize = 8 + playerNb*30 + pzNb * (2+4*playerNb) + 4 ; // see a bit highter for that ..
	char * file = (char*)malloc(fileSize);
	char * ptr = file+4 ;
// header
	*(int*)file = *(int*)"psve" ;	// it's a picore save file
	*(u16*)ptr  = 1		; ptr+=2;	// only one player
	*(u16*)ptr  = pzNb	; ptr+=2;	// puzzle number
// players info
	*(u16*)ptr  = 0x2a2a ;	ptr+=2;	// check
	*(u16*)ptr  = 0 ;		ptr+=2;	// curent level, 0 for a new save
	memcpy(ptr,option,8);	ptr+=8;	// default options
	memset(ptr,0,18); // fill name char with 0
	/* here ask for player name ... but for now, it's only my name :D */
	while(!*ptr || *ptr == ' ') askforName(ptr,10);
	//*ptr='r'; ptr[1]='o'; ptr[2]='v';
	//memset(ptr+3,0,15); // fill other name char with 0

// stats for all players and levels
	ptr = file + 8 + 30 ; // point after header & users info
	// now write stats for all puzzle !
	for(int n=0;n<pzNb;n++)
		{	*ptr = ptr[1] = 42 ;	ptr+=2; // fill check with '*'
			*(int*)ptr = 0 ;		ptr+=4;	// fill level stat with 0
		};
// end of file
	*(u32*)ptr  = 0x2a2a2a2a ; ptr+=4 ;

	#ifdef _debug
	// make a final test to see if all is ok ..
		if(ptr-file != fileSize)
		 printlr("error on create save file !\nnormal size %i\ncurent size %i",fileSize,ptr-file);
		else printlr("\nsucces on create file save..");
	#endif

	if(size) *size = fileSize ; // save filesize, if a pointer was specified

	return file ;
}

char * getPlayerName(int player=0)
{	char * ptr = saveFile ;
	ptr += 20+(player*30);
	return ptr ;
}

	/*
	// ok, we have the list, now draw it and ask user to select his name
	int py,bx,by,bsy ;
	int selected=0xff;
	int oldHeight = 0 ;
	int oldMousey=0; // it's for prevent maximize command second click bug
	int exit = 0 ;
	setGdlfont(mfont[1]);
	askName:;				// yet i use goto :D
	while(!exit)
		{	if(oldHeight!=HEIGHT) // does resolution change ?
			{	py = (HEIGHT - ((playerNb+1)*10))>>1; // list draw will be centered
				bsy = ((playerNb+3)*10 + 6)>>4 ;	// box y size, in 16² tiles
				by  = ((HEIGHT-(bsy<<4))>>1)-1 ;	// box y screen pos
				bx	= (WIDTH-7*16)>>1;
				oldHeight = HEIGHT ;
			}
		
			// show title screen
				// drawGfm(title,...
			// and box with referenced names
			setPausedState(1); drawGame();
			drawBox(bx,by,7,bsy,16,tsetBox,0x2a2aff);
			setGdlfont(mfont[0]);
			if(selected!=0xff) // any name selected ?
			{	for(int n=0;n<selected;n++)
					prints(-1,py+n*10,"%s",players[n]);
				prints(-1,py+n*10,".%s.",players[n]);
				setGdlfont(mfont[1]);
				prints(-1,py+n*10,"%s",players[n]);
				setGdlfont(mfont[0]);
				for(n=selected+1;n<=playerNb;n++)
					prints(-1,py+n*10,"%s",players[n]);
			} else	{	for(int n=0;n<=playerNb;n++)
							prints(-1,py+n*10,"%s",players[n]);
					}
			
			//if(selected!=0xff) prints(-1,HEIGHT-30,"%s",players[selected]);
			
				selected = 0xff ;
			if(mousex>bx+16&&mousex<bx+16*6)
				if(mousey>py&&mousey<py+10+playerNb*10)
					selected = (mousey - py)/10 ;	

			// refresh windows
			ptc_update(pixel);
			
			// if left mouse click on a name, select it and return
			if(mouseLeftState && oldMousey-mousey < 20)
				if(selected!=0xff) exit++;
			else mouseLeftState=0;
			oldMousey = mousey ;
		};

	while(mouseLeftState) { setPausedState(1); drawGame(); ptc_update(pixel); };	setPausedState(0);
	*/
	

int choosePlayer(void)
{	// first, extract all the player referenced into the save file..
	char ** players = 0 ;
	extractAndAskName:; if(players) free(players);
	u16*ptr = (u16*)saveFile ; ptr+=2;
	int playerNb = *ptr++ ;	int pzNb = *ptr-- ;
	players = (char**)malloc(4*playerNb+4+1024);
	*players = getPlayerName(0);
	char *workBf = (char*)players; workBf += 256 ;
	sprintf(workBf,"%s",*players);
	for(int n=1;n<playerNb;n++)
		{	players[n] = getPlayerName(n);
			sprintf(workBf,"%s|%s",workBf,players[n]);
		};
	 // add a new player entry if there was less than 10 players who enter their name..
	if(playerNb<10) {	players[playerNb] = "new player" ;
						sprintf(workBf,"%s|%s",workBf,players[playerNb]);
					}
	else playerNb-- ;
	
	drawChooseNick = 1 ;
	int selected = drawMenu(workBf); // ask player name
	drawChooseNick = 0 ;
	//if(selected<0) goto extractAndAskName; // if invalid, rerequest (will never append)
	//printlr("selected name %i ->\'%s\'",selected,players[selected]);
	// if user selected 'new player', ask for his name !
	if(selected == playerNb && *(u16*)(players[playerNb]+2) == *(u16*)"w ")
		{	// ask for name
			char name[20];	askforName(name,10);
			// update save file with the new name if there is one
		  if(*name && *name != ' ') // if first char is a space or 0 assume it's not a valid name
		  {	int newPlayerNb = playerNb+1 ;
			int newfileSize = 8 + newPlayerNb*30 + pzNb * (2+4*newPlayerNb) + 4 ; // compute new file size
			char * newSaveFile = (char*)malloc(newfileSize);	// alloc ram
			memcpy(newSaveFile,saveFile,8+30*playerNb);		// copy header & old player info from the old file
			*(u16*)(newSaveFile+4) = newPlayerNb ; // fix player number in header
			u16*ptr = (u16*)(newSaveFile+8+30*playerNb) ;	// point onto newest player info
			*ptr++ = 10794 ;				// write check
			*ptr++ = 0 ;					// write last level
			memcpy(ptr,option,8) ;			// copy default option
			ptr+=4;	memset(ptr,0,18);		// fill name with 0
			int size = strlen(name) ; if(size>17) size=17;
			memcpy(ptr,name,size); // and copy new player name
			ptr+=9;
			// ok, header and player info are fixed
			char * offset = saveFile+8+playerNb*30;
			for(int n=0;n<pzNb;n++)
				{	memcpy(ptr,offset,2+4*playerNb); offset += 2+4*playerNb ; // copy old stat
					ptr += (2+4*playerNb)>>1;
					*ptr++=0; *ptr++=0;	// set new stats to 0
				};
			*ptr = ptr[1] = 10794 ;	// file end check
			// save the old file..
			//sprintf(name,"%s.old",savePath);
			remove(name);//rename(savePath,name);
			// and write the new
			writeFile(savePath,newSaveFile,newfileSize);
			free(saveFile);	saveFile = newSaveFile ;
			saveFileSize = newfileSize ;
			goto extractAndAskName ;
		  } else { goto extractAndAskName ; } // if new player name is invalid, rerequest it
		}

	// player is select, free list and return name choosed
	free(players);
	return selected ;
}

void loadSaveFile(void)
{	if(saveFile) free(saveFile);
	saveFile = (char*)loadFile(savePath,&saveFileSize);

	if(saveFile) // save file found, now test size to see if it's a valid file..
		{	u16*ptr = (u16*)saveFile; ptr += 2 ;
			int check = saveFileSize;
			int playerNb = *ptr++;
			int pzNb     = *ptr;
			check -= 8 + playerNb*30 + pzNb * (2+4*playerNb) + 4 ;
			if(check) // file is corupt !
			{	free(saveFile); saveFile=0;
				#ifdef _debug
					log("\nsave file is corupt ! sorry :|");
				#endif
			}
		}

	if(!saveFile)
		{	saveFile = createSaveFile(puzzle->numberOfPuzzle,&saveFileSize);
			writeFile(savePath,saveFile,saveFileSize);
		}
		#ifdef _debug
			else log("\nsucces on load save file..");
		#endif
	
	curentPlayerName = getPlayerName(0);
		#ifdef _debug
			printl("current player name : %s\n",curentPlayerName);
		#endif
}

///////////////////////////////////


/*void _getString(char*buffer)
{	char * s = buffer ;
	*s++ = ':' ; *s = 0 ;
	while(!keyUp(kenter))
	{	for(int n='A';n<='Z';n++) if(keyUp(n)) { *s++ = n-'A'+'a' ; *s=0; n=255; }
		if(keyUp(' ')) { *s++ = ' ' ; *s=0; }
		memset(pixel,0,WIDTH*HEIGHT*4);
		drawText(buffer,30,30,0,*mfont);
		ptc_update(pixel);
	};
}

int getString(char*buffer,int maxChar,int x,int y)
{	static int nb=0,n ;
	char *s = buffer ;
	if(!s) { nb=0; return 0;}
	
	if(!nb) *s = 0 ; else s += nb;
	
	for(n='A';n<='Z';n++)
		if(keyUp(n))
			{ *s++ = n-'A'+'a' ; *s=0; nb++; n=0xffff; }
	if(keyUp(' '))
		{ *s++ = ' ' ; *s=0; nb++; }
	
	if(x&&y)
		{	prints(x,y,".%s.",buffer);
		}

	if(nb<maxChar && !keyUp(kenter)) return 0 ;
	
	n = nb ; nb = 0 ; return n ;
}*/

void onFocusKill(void)
{	setPausedState(1);
}

int focusTime=0; // tick when focus was set

void onFocus(void)
{	if(mousex > (WIDTH-26) && mousex < WIDTH) // end click on song button
		if(mousey > (HEIGHT-10) && mousey < HEIGHT)
			return ;
	startClickX=startClickY=255 ;
	while(mouseLeftState) { setPausedState(1); drawGame(); ptc_update(pixel); };
	setPausedState(0);
	focusTime=tick;
}

void startClick(int button) // selection start point
{	doesShowCursor();

	//if(paused){ SetCursor(0); onFocus(); } // any click will exit pause
	if(tick - focusTime < 100 || paused || !canClick) return ; // to prevent false click	

	if(mouseIsInArray)
	{	int cursorx, cursory ;
		cursorx = mousex - xinfopos ;	cursorx /= xspace ;
		cursory = mousey - yinfopos ;	cursory /= yspace ;
		char * curentTile = &(maskedMap[cursory*currentMapSizeX+cursorx]);
		//if(*curentTile < 3)	// click on a masked tile, set it as start point
		{	startClickY = cursory ;	startClickX = cursorx ;
		}// else startClickX=startClickY=255 ;
	}
}

void endClick(int button) // selection end point, yet not add box boom
{	doesShowCursor();
	
	if(tick - focusTime < 100) return ;
	if(paused||!canClick) { startClickX=startClickY=255 ; return ; }

	int xsame, ysame ;
	if(mouseIsInArray && startClickX!=255)
	{	int cursorx, cursory ;
		cursorx = mousex - xinfopos ;	cursorx /= xspace ;
		cursory = mousey - yinfopos ;	cursory /= yspace ;
		char * curentTile = &(maskedMap[cursory*currentMapSizeX+cursorx]);
		//if(*curentTile < 3) // is end point valid ?
		{		ysame = (startClickY == cursory) ;
				xsame = (startClickX == cursorx) ;
				
				   switch(button)
					{	case -1 : // left	
							if(xsame && ysame) { leftclick(curentTile); goto end ; }
							if(ysame) // line boom
								{	int n=startClickX-cursorx ; if(n<0) n = -n ;
										if(startClickX > cursorx)
										{		curentTile+=n ;
												while(n--) if(!leftclick(curentTile--)) goto end;
										}	else	{	curentTile-=n ;
														while(n--) if(!leftclick(curentTile++)) goto end;
													}
									leftclick(curentTile);
								}
							else if(xsame) // colon boom
									{	int n=cursory-startClickY ; if(n<0) n = -n ;
										if(startClickY > cursory)
										{	curentTile += currentMapSizeX*n ; // to invert boom way ..
												while(n--)
													{	if(!leftclick(curentTile)) goto end;
														curentTile -= currentMapSizeX ;
													}
										}	else	{	curentTile -= currentMapSizeX*n ;
														while(n--)
														{	if(!leftclick(curentTile)) goto end;
															curentTile += currentMapSizeX ;
														}
													}
										leftclick(curentTile);
									}
							end: startClickX=startClickY=255 ; 
						break;

						case  0 : // middle
							doesShowCursor();
						break ;

						case  1 : // right
							doesShowCursor();
							int startTileValue = maskedMap[startClickY*currentMapSizeX+startClickX];
							if(xsame && ysame) { rightclick(curentTile); break ; }
							if(ysame) // line boom
								{	int n=startClickX-cursorx ; if(n<0) n = -n ;
										if(startClickX > cursorx)
										{		curentTile+=n ;
												while(n--) rightclick(curentTile--,startTileValue);
										}	else	{	curentTile-=n ;
														while(n--) rightclick(curentTile++,startTileValue);
													}
									rightclick(curentTile,startTileValue);
								}
							else if(xsame) // colon boom
									{	int n=cursory-startClickY ; if(n<0) n = -n ;
										if(startClickY > cursory)
										{	curentTile += currentMapSizeX*n ; // to invert boom way ..
												while(n--)
													{	rightclick(curentTile,startTileValue);
														curentTile -= currentMapSizeX ;
													}
										}	else	{	curentTile -= currentMapSizeX*n ;
														while(n--)
														{	rightclick(curentTile,startTileValue);
															curentTile += currentMapSizeX ;
														}
													}
										rightclick(curentTile,startTileValue);
									}
						break ;
				   };
				startClickX=startClickY=255 ; 
		}
	}
}

void unmaskAbit(void) // will help you a bit
{	register int finish=0, px=0, py=0, speedX=1, speedY=1, nb=0, nbb=0;
	int stopX = rand()%currentMapSizeX ; // 
	int stopY = rand()%currentMapSizeY ;
	while(!finish) // show moving line and colon marker
	{		drawGame();
			if(!paused)
			{
			if(--speedX==0)
				{	speedX=1+(rand()%4)+nbb;
					if(++px == currentMapSizeX) px=0;
					
					if(++nbb > 9 && px==stopX) speedX = -1;
				}

			if(--speedY==0)
				{	speedY=2+(rand()%4)+nb;
					if(++py == currentMapSizeY) py=0;
					if(++nb > 8 && py==stopY) speedY = -1;
				}

			if(speedX<0 && speedY<0) finish=1;

			for(int i=0;i<currentMapSizeY;i++)
				drawGfm((*mfont)['.'-' '],xinfopos + (px*xspace) - 1,yinfopos+yspace*i -1);

			for(i=0;i<currentMapSizeX;i++)
				drawGfm((*mfont)['.'-' '],xinfopos+xspace*i -1,yinfopos + (py*yspace) - 1);
			showCursor();
			}
			ptc_update(pixel);
	};
	// now that we have select a line and a colon, destruct them !
	char *colontile = &(maskedMap[stopX]);
	char * linetile = &(maskedMap[stopY*currentMapSizeX]); ;

	canLoseTime = 0 ;
	for(int i=0;i<currentMapSizeY;i++) // colon boom
		{	leftclick(colontile);
			colontile += currentMapSizeX ;
		}

	for(i=0;i<currentMapSizeX;i++) // and line boom
		{	leftclick(linetile++);
		}
	canLoseTime = 1 ;
	playS3mSample(1,255,9,4,64);
	playS3mSample(1,255,0,5,48,1);
	playS3mSample(1,255,0,6,32);
}

	unsigned char *zik=0 ;	// pointer to output song
	unsigned long dataSize ;	// size of song..
	int currentSong ;

void playSong(char*path)
{	u32 fsize=0; char *p = path ;
	/*u32 inrar=0;
	while(*p)
	{	if(*p == 'r' || *p == 'R')
			if(p[1] == 'a' || p[1] == 'A')
				if(p[2] == 'r' || p[2] == 'R')	
					inrar = 1 ;
		p++;
	};

	if(inrar)	return ; // playing song from an archive is not coded for now ..
*/
	// load and play current song..
	unsigned char * song = loadFile(path,&fsize);
	FMUSIC_StopSong(mod[stzIk]);
	FMUSIC_FreeSong(mod[stzIk]);
	if(zik)	free(zik);
	zik = song ;	*mzIk = zik; *mzIkSze = fsize;
	zk[stzIk].pos=0 ; zk[stzIk].length=fsize; zk[stzIk].data=zik;
	mod[stzIk] = FMUSIC_LoadSong((char*)&(zk[stzIk]), NULL);
	FMUSIC_PlaySong(mod[stzIk]);
	memcpy(sngName,&(zik[17]),20);
	sngName[20]=0;
	sprintf(sngString,"%s",sngName);
	resetSongTitlePos();
}

void onOpen(void) // on window open callback
{	resetScreenSize();
}

void onExitProcess(void)
{	if(!*mzIk) return ;
	volumeOfSpl=0;
	//Sleep(500);
	FMUSIC_fadeSound2Zero();
	Sleep(200);
	//freeAllEntry(); // can made a crash... i prefer let windows free all
}

struct s3m * curents3m = 0 ;

void onS3mDrop(const char*path)
{	clearMix();
	u32 fileSize ;	static u8*s3m = 0 ;
	if(s3m) free(s3m);	s3m = loadFile(path,&fileSize);
	struct s3m * mys3m = loadS3m(s3m,fileSize);
	if(curents3m) free(curents3m) ;	curents3m = mys3m ;
}

void onDrop(const char*path)
{ const char*p = path;
	while(*p++); p-=2;
	if(*p=='d') // level definition file, to retreve original bitmap from only info.
		{	u32 fileSize; u8*file = loadFile(path,&fileSize);
			u8*p=file;	int pzsize = *p++ ;


			free(file);
		}
}

void iniEffect(void)
{	clearMix();
	if(curents3m) free(curents3m) ;
	curents3m = loadS3m(effect,effectSize);
}

void playS3mSample(int splNb,int chn,int period,int octave,int volume,int differed,int maxTime)
{	if(!curents3m) return ;
	struct sample * spl = curents3m->sample[splNb] ;	if(!spl) return ;
	addSample(	spl->data,chn,
				period,octave,
				*spl->hrz16,
				*spl->size,0,volume,
				*spl->looped,
				*spl->loopStart,
				*spl->loopEnd,
				differed,maxTime);
}

MemoryFile		zikRAR, includeRAR, *currentRAR ;

int iniSomeAndSomeThing(void)
{
		#ifdef _debug
			log();	// clear log..
		#endif
	savePath = (char*)malloc(512);
	sprintf(savePath,"%s\\picore.sav",getenv("windir")) ;
	
	u8*isApuzzleFile = loadFile("picore.lvl");
	if(!isApuzzleFile) isApuzzleFile = easyp ;

	puzzle = loadPuzzleFile(isApuzzleFile); // load puzzle file
	
	// fill a memory file structure with included rar
	includeRAR.data		= zikmu ;
	includeRAR.offset	= 0 ;
	includeRAR.size		= zikmuSize ;

	const char * songRARPath = "./picore.rar" ;
	// search for an external song..
	if(fileExist(songRARPath)) currentRAR = (MemoryFile*)songRARPath;
		else currentRAR = &includeRAR ;	// or the included

		// uncompress some gfx..
	for(int n=0;n<91;n++)
		{	s0font[n]		=	data2Gfm(s1font[n]);
			greenfont[n]	=	data2Gfm((u8*)greenfont[n]);
			flashfont[n]	=	data2Gfm((u8*)flashfont[n]);
		};
	for(n=0;n<4;n++)	{	zcase[n]	=	data2Gfm((u8*)(zcase[n]));
							cursor[n]	=	data2Gfm((u8*)(cursor[n]));
							bombe[n]	=	data2Gfm((u8*)(bombe[n]));
							cursorSelected[n] =	data2Gfm((u8*)(cursorSelected[n]));
							if(n!=3)	{	marteau[n]	=	data2Gfm((u8*)(marteau[n]));	// hammer got only 3 step
											flower[n]	=	data2Gfm((u8*)(flower[n]));		// same as for flower anim
											gomme[n]	=	data2Gfm((u8*)(gomme[n]));		// and eraser..
											snow2[n] = data2Gfm((u8*)(snow2[n]));
											//icon[n]		=	data2Gfm((u8*)(icon[n]));		// avatar icons
										}
						}
	for(n=0;n<6;n++)songChoice[n] = data2Gfm((u8*)(songChoice[n]));
	for(n=0;n<11;n++)	chfont[n] = data2Gfm((u8*)chfont[n]);
//	for(n=0;n<26;n++)	ltfont[n] = data2Gfm((u8*)ltfont[n]);
	for(n=0;n<8;n++)	{	 caseboom[n]	=	data2Gfm((u8*)(caseboom[n]));
							_caseboom[n]    =	data2Gfm((u8*)(_caseboom[n]));
							//banana[n]	=	data2Gfm((u8*)(banana[n]));
							tsetBox[n]	=	data2Gfm((u8*)(tsetBox[n]));
							pzcursor[n] = data2Gfm((u8*)(pzcursor[n]));
						}
	*note = data2Gfm((u8*)*note);

	choosePz = data2Gfm(choosePz0);
	chooseNick = data2Gfm(chooseNick0);

		// duplicate somes pictures in anims ..
	marteau[4] = marteau[0] ;	marteau[3] = marteau[1] ;	// in hammer anim
	gomme[3] = gomme[1] ;		gomme[4] = gomme[2] ;	gomme[5] = gomme[1] ;	// and eraser

	// create some anim..
	gom = setAnim(gomme,6,100,1,&animSwitch);
	pen = setAnim(bombe,4,100,1,&animSwitch);
	cur = setAnim(cursor,4,110);
	fleur = setAnim(flower,3,75);
	scur = setAnim(cursorSelected,4,110);
	mapcur = setAnim(&pzcursor[4],4,110);
	hammer = setAnim(marteau,5,100,1);
	//banane = setAnim(banana,8,100);
	hammer -> curentFrm = 4 ; // for hammer anim don't play at start
	currentCursor = hammer ; // set current cursor as the hammer
	iniCaseBoom();
	sngString = (char*)malloc(2048);
	sngName   = sngString + 1024 ;

	setGdlfont(*mfont);

	srand(GetTickCount() + rand()%rand()%GetTickCount()); // randomize..
	if(currentRAR->data && currentRAR->size)
	{	int songNb = urarlib_list(currentRAR,(ArchiveList_struct*)&songList);
		
		currentSong = rand()%songNb ;
		lst = songList ;
		for(int n=0;n<currentSong;n++)
				lst = lst->next ;	
		if(!urarlib_get(&zik,&dataSize,(lst->item).Name,currentRAR,"r043v")) zik=0;
		 else strcpy(sngName,(lst->item).Name) ;
		urarlib_freelist(songList);
	}

	if(zik)	{ *mzIk = zik; *mzIkSze = dataSize;	}

	maskedMap = (char*)allocAndAddFree(1024);
	loadMap(mapNb,0);					// load the curent map
	
	resizeWindows();	// resize curent windows to array size..

	pixel = (int*)malloc(640*480*4);// declare a 32bits 640*480 buffer

	if(!ptc_open("[subuto]",&WIDTH,&HEIGHT)) return 0; // create framebuffer

	ptc_iniCallBack(&startClick,&endClick,&onFocus,&onFocusKill,&onmove,&onDrop,&onExitProcess);

	iniEffect();
	iniZik();	// start musik playback..

	mousex=mousey=-30 ; onmove();
	
	srand(GetTickCount() + rand()%rand()%255);
	
	return 1 ;
}
u8 showSnow=1;
void showSomeSnow(void)
{	if(paused && !showSnow) return ;
	static u8 * snow = 0 ;
	static u32 snowNb=0;
	if(!snow) { snow = (u8*)allocAndAddFree(2048); memset(snow,0,2048); }
	u8 * enable = snow ;
	u8 * gfx = snow + 128 ;
	char * way = (char*)(gfx + 128);
	u8 * wchange = gfx + 256 ;
	int * lastTime = (int*)(wchange+128);
	int * py  = lastTime ; py += 128 ;
	int * px  = py ; px += 128 ;

	tick = GetTickCount();

	static int flip=0;
	if(snowNb < 42 && ++flip > 42+(rand()%64)) // 42 snow draw will be max (real max is 128)
		{	u8*p=enable;	while(*p)p++ ;
			u32 z = p-snow ;
			*p=1;
			px[z] = rand()%WIDTH-32;
			py[z] = -32 ;
			gfx[z] = rand()%3 ;
			for(int zz=0;zz<3;zz++) if(gfx[z] == 2) if(gfx[z] == 2) gfx[z] = rand()%3 ;

			way[z] = -1 + 2*(rand()%2) ;
			wchange[z] = rand()%8 ;
			flip=0;
			snowNb++ ;
		}

	u32 snowDrawed = 0 ;
	u32 nb = snowNb ;
	while(snowDrawed < nb) // draw snow
	{	while(!*enable) enable ++ ;
		u32 z = enable-snow ;
		if(z<127)
			{	if(tick - lastTime[z] > (16+rand()%64))
				{	lastTime[z]=tick;
					/*if(z)*/ px[z] += way[z];
					//else px[z] = COS(py[z]) ;
					py[z] += 1 + (rand()%2);

					if(py[z]>HEIGHT) { snowNb--; *enable=0 ; }
					if(wchange[z]) wchange[z]--;
					else	{	way[z] = -1 + 2*(rand()%2) ;
								wchange[z] = rand()%8 ;
							}
				}
				
					drawGfm(snow2[gfx[z]],px[z],py[z]);
				//if(!paused) else drawGfmOnBlackOnly(snow2[gfx[z]],px[z],py[z]);
			}//	else	return ;
		enable ++ ;	snowDrawed++;
	};
}

	/*  picore save file format ..

  all not text value are unsigned ..

	0	4	"psve"		it's a picore save..
	4	2	playerNb	number of players
	6	2	pzNb		number of puzzles
	8

 offset : 8
	for(eatch players)

	0	2	"**"		0x2a2a    42 pawa :)
	2	2	lastLvl		curent level (first unfinished)
	4	8	options		player options..
	12	18	name		player name.. 17 char + 0 ;
	30

 offset : 8 + playerNb*30 ;

	for(eatch puzzles)

	0	2	"**"		file check
	
		for(eatch players)

		0	4	player stats for the level
				
				0	2	number of try
				2	2	best time left in second, 0 if never finished
				4

 offset : 8 + playerNb*30 + pzNb * (2+4*playerNb);

	"****"	end of file

  so if filesize != [8 + playerNb*30 + pzNb * (2+4*playerNb) + 4], file  is corupt !
*/

u32 selectMap(void)
{	int bx,by,bsx,bsy; // box pos (in screen&pixels) & size (in tiles)
	u32 leftMouse = mouseLeftState ; // for retreve up front left click
	u32 exit=0, selected; // user finally select & valid an entry ?
	int oldHeight=0; // to intercept buffer resolution change & recompute box pos..

	#define mapByLine 10
	u32 playerNb = *(u16*)(saveFile+4);
	u32 mapNb = *(u16*)(saveFile+6);
	u32 curMap = *(u16*)(saveFile+8+curentPlayer*30+2);
	u32 szx=mapByLine , szy ;
	szy = mapNb / mapByLine ;
	if(mapNb != szy*mapByLine) szy++;
	char * offsetToFirstLevel = saveFile + 8 + playerNb*30 + 2 + 4*curentPlayer ;
	u16 * lvlOffset ;// = (u16*)offsetToFirstLevel ;

	char * infoString = (char*)malloc(1024);
	u32 old = 0xff ;
	
	*infoString = 0 ;

	while(!exit)
	{	if(oldHeight!=HEIGHT) // does resolution change ?
			{	bsx = ((mapByLine+2)<<3)>>4;
				bsy = ((szy+2)<<3)>>4;	//  designed to work
				bx	= (WIDTH -(bsx<<4))>>1;		//     only with
				by	= (HEIGHT-(bsy<<4))>>1;		// 8²fonts & 16²tiles
				oldHeight = HEIGHT ;
				//printlr("set size.. bx %i by %i bsx %i bsy %i",bx,by,bsx,bsy);
			}

			// show music playback
		setPausedState(1); drawGame();
		
		// show box
		drawBox(bx,by,bsx,bsy,16,tsetBox,0x2a2aff);
		
		drawGfm(choosePz,(WIDTH-64)>>1,by-8);

		// reset pointer on save file to the first map
		lvlOffset = (u16*)offsetToFirstLevel ;

		// declare some vars..
		int x=1,y=1 ; u32 n,z=0, tileToDraw;
		
		// show all referenced levels
		for(n=0;n<mapNb;n++)
			{ // lvlOffset = retrevePlayerStat(curentPlayer,n);
			  switch(*lvlOffset) // *lvlOffset point to map try nb
				{	case 0 : if(n) tileToDraw = 0 ; else tileToDraw = 2 ; break; // never played, in purple
					default : // was played a minimum of one time
						if(lvlOffset[1] < 1740) // if time left < 29 minuts, draw it as played
							{	if(lvlOffset[1]) tileToDraw = 1 ; // set it as played, in blue
								 else tileToDraw = 2 ; // never finished ? set it in green
							}
						else tileToDraw = 3 ; // set it as greatly finished if less than a minut, in yellow
					break;
				};
				
				//if(z == curMap) // if map is the next to play, show it in green
				//	tileToDraw = 2 ;

					// finally draw the tile who represent map
				drawGfm(pzcursor[tileToDraw],bx+(x<<3),by+(y<<3));

				if(old != selected)
					{	u16 * stat = retrevePlayerStat(curentPlayer,selected);
						if(stat)
							{	if(!*stat) sprintf(infoString,"never played");
								 else
								if(stat[1]) 
								 {	u32 m = 29-(stat[1]/60) ;
									u32 s = 59-(stat[1]%60) ;
									if(m) sprintf(infoString,"%i try bestTime %i:%i",*stat,m,s);
									else  sprintf(infoString,"%i try bestTime %i\"",*stat,s);
								 } else sprintf(infoString,"never finished !");
							} else sprintf(infoString,"internal error");
						old = selected ;
					}

				if(z == selected) // current map is selected by mouse ?
				{	// show selected map anim (rotating red)
						playAnim(&mapcur,bx+(x<<3),by+(y<<3));
					// also draw your stat for this level...
						if(*infoString)
							prints(-1,HEIGHT-32,infoString);			
				}

				// update pointer in save file & coordonate in screen for the next map..
				lvlOffset += 1+playerNb*2 ; z++ ;
				if(++x == 11) { x=1; y++; }
			};

		// is mouse on box ?
		selected = 0xff ;
		if(mousex>=bx+8 && mousex<= bx+6+((mapByLine)<<3) )
		 if(mousey>=by+8 && mousey<=by+8+((y-1)<<3) )
			{	// mouse is on box, draw selected char/command if there is one
				selected=0xff;
				y = (mousey-(by+8))>>3 ;
				x = (mousex-(bx+8))>>3 ;
				if(y*mapByLine+x < (int)mapNb) selected = y*mapByLine+x ;
			}

		if(!leftMouse && mouseLeftState && selected != 0xff)
		{	 // can only choose an tryed level (or level 0)
			if(!selected || *retrevePlayerStat(curentPlayer,selected)) exit=1;
		}

		leftMouse = mouseLeftState ;
		ptc_update(pixel);
	};	setPausedState(0);

	free(infoString);
	return selected;
}

int drawMenu(char*entry,const char * entryStr,const char * separator,int**font0,int**font1,void(*customBlit)(void))
{	if(!entry || !*entry) return -1;
	int entryNb=1 ;
	u32 entryMaxSize=0 ;
	char *offsetEntry[32] ; *offsetEntry = entry ;
	int**systemfnt = getGdlfont();
	u32 selected = 0xff ; // 0xff, nothing for now
	//log(); log(entry);
	// count number of menu entry && entry max size
	{	char *e = entry, *start=e ;
		while(*e)
			{	if(*e == '|') // new entry flag
						{	register u32 n = e-start ;
							if(n > entryMaxSize) entryMaxSize = n ;
							*(u32*)&offsetEntry[entryNb+15] = n; // save entry size
							*e++ = 0;	// yes, write onto the input string	!
							//printlr("entry %i size .. %i.%i .. %s",entryNb,n,*(u32*)&offsetEntry[entryNb+15],start);
							offsetEntry[entryNb++] = e; // save entry offset
							if(entryNb == 16) *e=0; // 16 entry max !
							start=e ;	//log(e);
						} else e++;
			};	//printlr("%i entry .. max size %i",entryNb,entryMaxSize);
		{	register u32 n = e-start ;
			if(n > entryMaxSize) entryMaxSize = n ;
			*(u32*)&offsetEntry[entryNb+15] = n;
		}
		
	// maybe we have to return now ?
	if(!entryMaxSize) return -1;	if(entryNb==1) return 0 ;

	e = entry ;	setGdlfont(font0); // first array font for unselected option, use the second for the selected, if there is one
	
	int bx,by,bsx,bsy; // box pos (in screen&pixels) & size (in tiles)
	u32 leftMouse = mouseLeftState ; // for retreve up front left click
	u32 exit=0; // user finally select & valid an entry ?
	int oldHeight=0; // to intercept buffer resolution change & recompute box pos..
	int tx,ty ; // text pos x & y

	while(!exit || leftMouse)
	{	if(oldHeight!=HEIGHT) // does resolution change ?
			{	bsx = ((entryMaxSize+5)<<3)>>4;
				bsy = ((entryNb+3)*10 + 6)>>4;	//  designed to work
				bx	= (WIDTH -(bsx<<4))>>1;		//     only with
				by	= (HEIGHT-(bsy<<4))>>1;		// 8²fonts & 16²tiles
				tx	= (WIDTH - (entryMaxSize<<3))>>1;
				ty	= (HEIGHT - (entryNb*10))>>1;

				oldHeight = HEIGHT ;
				//printlr("set size.. bx %i by %i bsx %i bsy %i",bx,by,bsx,bsy);
			}
		// show music playback
		
		if(!customBlit) { setPausedState(1); drawGame(); }
		 else customBlit();
		
		if(drawChooseNick)
			drawGfm(chooseNick,(WIDTH-64)>>1,by-10);

		// show box & entry
		drawBox(bx,by,bsx,bsy,16,tsetBox,0x2a2aff);
		u32* sizeptr = (u32*)&offsetEntry[16];
		for(int n=0;n<entryNb;n++)
		{	if(*sizeptr++)
				prints(-1,ty+10*n,offsetEntry[n]);
			else if(separator && *separator) prints(-1,ty+10*n,separator);
		};

		// is mouse on box ?
		if(mousey>=ty && mousey<=ty+10*entryNb)
			//if(mousex>=bx && mousex<bx+(bsx<<4))
			{	// mouse is on box, draw selected char/command if there is one
				selected=0xff;
				int y = (mousey-ty)/10 ;
				register int size = *(u32*)&offsetEntry[y+16];
				if(size)
					{	register int w = (WIDTH - (size<<3))>>1;
						//int x = (mousex-bx)>>3;
						if(mousex>=w && mousex<=w+(size<<3))
						if(y<entryNb)
							{	static int oldMousey=0; // it's for prevent maximize command second click bug
								selected=y;
								prints(-1,ty+10*y,entryStr,offsetEntry[selected]);
								setGdlfont(font1);
								prints(-1,ty+10*y,offsetEntry[selected]);
								setGdlfont(font0);
								// does left mouse button just clicked ?
								if(!leftMouse && mouseLeftState && oldMousey-mousey < 8 && oldMousey-mousey > -8)
								{	if(selected != 0xff) exit++;
									//log("click");
								}	oldMousey = mousey ;
							}
					}
			}		

		leftMouse = mouseLeftState ;
		ptc_update(pixel);
	};	setPausedState(0);
}
	setGdlfont(systemfnt);
	return selected;
}

	char* startString[] =	{	"picore says you hello !",
								".. welcome into picore ..",
								"       .picore  ..  this is the emy release.14.2.2k6",
								"ready ?",
								"............................. dphaz.",
								".. ferof says you hello !      also hello to * bruno & s.Maudit * ramucho & viviane * jean & zeus the hair cousins * chaaa...aaaaaang killer & yasmine * stakhanov * sir lion king & lucie * wesh & olivia * don justin ridou * salomon & anne-laure * lunatic * tavu! * elodie * jah * vV.. viper & rabten brothers * aurore * joeFait * fabien * kyzer & anais * lordViolence * freeb * footx * marina * denoil * dendoo * carine * alexia * simon * el toukal * all perchmen (and perchwomen:) * julie * gros * nastar & family * denis * manue * laurianne & family * jackDa * marie/anne lucie & all vandange makr! * magalie * nourmounir * technic * loveMakerz * kial * marc * thunderZ * anata * melgo * meldo (still play goblin ?) * atrkid * jp * Ze * phil * mathieu * mariane & family * makt1, doh & all chip writr * leffe * all gp32 scene! * off course my emy, hope i see you today! . and others i forgot here ..                         have a happy new year!   -rov                                                         rov you suxx it is too old school ...       yes .. but the old is so cool :D"
							};

void playARandomSong(void)
{	if(currentRAR->data && currentRAR->size)
	{	static int lastPlayed = 0xffff ;
		curentMixBf=0;
		ArchiveList_struct *list,*plop;
		u32 songNb = urarlib_list(currentRAR,(ArchiveList_struct*)&plop);
		tick = GetTickCount();
		srand(tick);
		if(!readLastSong || lastPlayed == 0xffff)
		{	int tmp = lastPlayed ;
			lastPlayed = currentSong ;
			do	{ currentSong = rand()%songNb ;
				} while(lastPlayed == currentSong || tmp == currentSong);
		} else	{	int tmp = currentSong ;
					currentSong = lastPlayed ;
					lastPlayed = tmp ;	
				}

		list = plop ;
		for(int n=0;n<currentSong;n++)
				list = list->next ;
		u8*newZik ;
		if(deadSong) { newZik = empty ; dataSize = emptySize; }
		else	{	if(!urarlib_get(&newZik,&dataSize,(list->item).Name,currentRAR,"r043v"))
						newZik=0;
					else strcpy(sngName,(list->item).Name) ;
				}
		
		if(newZik)	{	
			zk[0].pos=0 ; zk[0].length=dataSize; zk[0].data=newZik;
			mod[1] = mod[0];
			mod[0] = FMUSIC_LoadSong((char*)&(zk[0]), NULL);			
			FMUSIC_changeSong(mod[0]);
			if(mod[1]) FMUSIC_FreeSongWithoutStop(mod[1]);
			mod[1] = 0 ;
			if(*mzIk) free(*mzIk);
			*mzIk = newZik; *mzIkSze = dataSize;
			char * name = (char*)*mzIk ;
			name += 16 ;
			memcpy(sngName,name,21);
			sngName[21]=0;
			sprintf(sngString,"%s",sngName);
			resetSongTitlePos();
			FMUSIC_setVolumeUpdateSpeed(10);
			readLastSong=0;
		} else	{	if(readLastSong) return ; // to prevent infinite loop if last song is bad..
					readLastSong = 1 ;
					playARandomSong();
					return ;
				}
		urarlib_freelist(plop);
	}
}

