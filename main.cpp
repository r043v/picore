
	/*          ___
		picore ]|[ yet another picross versus the world ..
				   .(c) 2k5/2k6 by ferof/dph
															* /
   ./ scene addiction ./
/.//////////////////./////////////////////////////////////////////////-

	hello world !

  ok, this game is made to test some change i made into tinyPtc
  this will not the best picross on world, but i'll try to made it under 64ko

		.. an dephased work ..

/*///////////////////////////////////-

  //	-  d p h a z

#include "./picore.h"

// callback who'll be launched at xml file parsing, when any argument is find
void onArg(char *arg,char*value)
{	static u32 goodPlayer=0;
	
	if(!strcmp("name",arg))
		goodPlayer = !strcmp(curentPlayerName,value) ;

	if(!goodPlayer) return ;

	if(!strcmp("skin",arg))
	{	arrayTheme = atoi(value);
		if(arrayTheme<0 || arrayTheme>1) arrayTheme=0 ;
	}

	if(!strcmp("isALamer",arg))
	{	playerIsALamer = atoi(value);
		if(playerIsALamer<0 || playerIsALamer>1) playerIsALamer=0 ;
	}

	if(!strcmp("infoWay",arg))
	{	infoWay = atoi(value);
		if(infoWay<0 || infoWay>1) infoWay=0 ;
	}
}

int main(void)
{	if(!iniSomeAndSomeThing())	// ini game,gfx,song,... blahblablb
		{ showMsg("     cannot render the framebuffer\n\nyou need directx 5 or more,\n\ta minimum of 16b color deep","that's too bad :|"); return 1 ; }

	loadSaveFile();	// load save file
	//log();
	curentLvlString = startString[rand()%6] ;
	lvlStringPos = WIDTH+10; lvlStringDec = 0 ;

	curentPlayer = choosePlayer(); // ask user to select his fucking name
	curentPlayerName = getPlayerName(curentPlayer);

	xml myxml("option.xml");
		myxml.scanAllFile(0,&onArg);	
	
	mapNb = selectMap();
	//loadCurrentMapNb();
	loadMap(mapNb); // load the curent map
	switch(arrayTheme)
	{	case 1 : zcase[0] = zsmall[0]; zcase[1] = zsmall[1]; zcase[2] = zsmall[2]; zcase[3] = zsmall[6]; break ;
		case 2 : zcase[0] = zsmall[3]; zcase[1] = zsmall[4]; zcase[2] = zsmall[5]; zcase[3] = zsmall[6]; break ;
		default : break ;
	};
	if(!inFullScreen) resizeWindows();
	canClick=1;
	if(lvlStringPos == 0xffff)
	{	sprintf(lvlString+lvlStringOffset,decimal,mapNb+1);
		curentLvlString = lvlString ;
		lvlStringPos = WIDTH+10; lvlStringDec = 0 ;
	}

	showSnow=0; // to not show snow while game is paused

	if(playerIsALamer) unmaskAbit(); // unmask a bit the array to help this lame player

	while(!key[kenter])
	{	drawGame();	if(!paused) showCursor();

	//zdrawGfm(*snow2,zz);
	
	if(keyUp(kspace)) paused=1;

	if(goodBlocInCurrentMap<=0 || youLose) // we unmask all good block !
	{	static int timeOnFinish=0, blockLeft, nbblock, blockLeft2 ;
		if(!timeOnFinish) { timeOnFinish = tick ; nbblock = currentMapSizeX*currentMapSizeY ; blockLeft = blockLeft2 = 0 ; }
		 else if(tick - timeOnFinish >= 100) // wait a demi second after the win
				{		if(!youLose && blockLeft < nbblock>>1) // fast clean array, let only good and masked block
								{	canLoseTime=0;
									register int tilePos   = blockLeft ;
									int tileValue = maskedMap[tilePos];
									//while(tileValue && tileValue!=2 && tilePos<nbblock) tileValue = maskedMap[++tilePos];

									switch(arrayTheme)
									{	case 0 :
										case 1 :
										case 2 :
											if(tileValue<2) // if block isn't good or bad
											{	addBoom(tilePos);
												if(currentMap[tilePos] != '0')	maskedMap[tilePos] = 2 ;
												 else	maskedMap[tilePos] = 3 ;
											};
										break ;
										default :
											if(tileValue && tileValue!=2) // if block isn't good or masked
											{	addBoom(tilePos);
												if(currentMap[tilePos] != '0')	maskedMap[tilePos] = 2 ;
												 else	maskedMap[tilePos] = 0 ;
											};
										break ;
									};

									tilePos = nbblock-1-tilePos ;
									tileValue = maskedMap[tilePos];
/*
									if(tileValue && tileValue!=2) // if block isn't good or masked
									{	addBoom(tilePos);
										if(currentMap[tilePos] != '0')	maskedMap[tilePos] = 2 ;
										 else	maskedMap[tilePos] = 0 ;
									};
*/
									switch(arrayTheme)
									{	case 0 :
										case 1 :
										case 2 :
											if(tileValue<2) // if block isn't good or bad
											{	addBoom(tilePos);
												if(currentMap[tilePos] != '0')	maskedMap[tilePos] = 2 ;
												 else	maskedMap[tilePos] = 3 ;
											};
										break ;
										default :
											if(tileValue && tileValue!=2) // if block isn't good or masked
											{	addBoom(tilePos);
												if(currentMap[tilePos] != '0')	maskedMap[tilePos] = 2 ;
												 else	maskedMap[tilePos] = 0 ;
											};
										break ;
									};

									blockLeft++;
									canLoseTime=1;
								}
						   else
						{
							if(blockLeft2 < nbblock) // fully clean array
							{	if(tick - timeOnFinish > 3333 || (youLose && tick - timeOnFinish > 1000))
								{	if(blockLeft2 < nbblock)
										{	canLoseTime=0;
											register int tilePos = blockLeft2 ;
											if(maskedMap[tilePos])
												{	addBoom(tilePos);
													maskedMap[tilePos]=0;
												};
											blockLeft2++;
											canLoseTime=1;
										}
								}
							} else { // array is cleaned, load next puzzle (if win :)
								while(caseBoomNumber) {	drawGame();	showCursor(); ptc_update(pixel); }
								timeOnFinish = 0 ;
								if(!youLose)
								{	if(mapNb == (puzzle->numberOfPuzzle)-1) mapNb=0;
									loadMap(++mapNb);
									if(!inFullScreen)	resizeWindowsToGame();
									else				resizeWindowedWindows();
									if(lvlStringPos==0xffff)
									{	sprintf(lvlString+lvlStringOffset,decimal,mapNb+1);
										curentLvlString = lvlString ;
										lvlStringPos = WIDTH+10; lvlStringDec = 0 ;
									}
								} else loadMap(mapNb);
								canClick=1;
								if(playerIsALamer) unmaskAbit();
							}
						}
					}
		}
/*
		if(keyUp(kright))
		{	if(mapNb == (puzzle->numberOfPuzzle)-1) mapNb=7;
				loadMap(++mapNb);
				if(!inFullScreen) resizeWindowsToGame();
				 else resizeWindowedWindows();
			unmaskAbit();
			//sprintf(string,"%2i/%2i",mapNb+1,puzzle->numberOfPuzzle);
		}
*/
		if(keyUp(kctrl)) {  // tum.tum..tchack !
							playS3mSample(2,255,0,4,64);
							playS3mSample(2,255,0,4,64,8);
							playS3mSample(3,255,0,4,64,25);
							//playS3mSample(1,255,0,4,64);
							//playS3mSample(2,255,0,4,64);
							unmaskAbit();
							//playS3mSample(1,255,0,3,64);
						}

		ptc_update(pixel);
	};
	
	// close windows, process and exit
		ptc_close(1);	return 1 ;
}

	/*if(argv[1]) // damn, argument doesn't work with tinyPtc :|
    {	int argCnt = 1 ;
		const char *opt ;
		do {	opt = argv[argCnt++];
				if( *opt == '-' )
				{ switch(opt[1])
				   { case 'f':
					 case 'F':
						inFullScreen = 1 ;
					 break;
				   };
				} else	{	opt = (const char *)argv[argCnt] ;
							while(*opt && *opt != '.') opt++ ;
							if(*opt++)
								{	if(*opt == 'x' || *opt == 'X')
									{	opt = (const char *)argv[argCnt] ;
										*mzIk = loadFile(opt,mzIkSze);
									}
								} else { *mzIk = 0 ; *mzIkSze = 0 ; }
							argCnt++;
						}
		} while(argCnt<argc && argv[argCnt]);
	}*/

