static int startGame=0; // game at top of list as it is displayed on the menu
static int gamesperscreen=12;
static int gametoplay=0;

void process_menu()
{
	for (unsigned int i=startGame,game_counter=0; game_counter<gamesperscreen; i++,game_counter++)
	{
		if (i>0 && i<nBurnDrvCount)
		{
			nBurnDrvSelect=i;
			if (game_counter==6)
			{
				pgPrint(0,game_counter,RGB(255,255,255),BurnDrvGetTextA(DRV_FULLNAME)))/2,0);
				gametoplay=i;
			}
			else
			{
				pgPrint(0,game_counter,RGB(255,255,0),BurnDrvGetTextA(DRV_FULLNAME)))/2,0);
			}
		}
	}
	nBurnDrvSelect=gametoplay;
 /*   
    char players[8];
    sprintf(players, "%d", BurnDrvGetMaxPlayers());
    SFont_Write(screen, greenfont,30 ,370 ,"Rom Name:");
    SFont_Write(screen, bluefont,150 ,370 ,BurnDrvGetTextA(DRV_NAME));
    SFont_Write(screen, greenfont,270 ,370 ,"Players:");
    SFont_Write(screen, bluefont,354 ,370 ,players);
    SFont_Write(screen, greenfont,404 ,370 ,"Year:");
    SFont_Write(screen, bluefont,464 ,370 ,BurnDrvGetTextA(DRV_DATE));

    SFont_Write(screen, greenfont,30 ,400 ,"Company:");
    SFont_Write(screen, bluefont,138 ,400 ,BurnDrvGetTextA(DRV_MANUFACTURER));

    SFont_Write(screen, greenfont,30 ,430 ,"Hardware:");
    SFont_Write(screen, bluefont,138 ,430 ,BurnDrvGetTextA(DRV_SYSTEM));

    SFont_Write(screen, bluefont,30,5, "FB Alpha/SDL V");
    static char version[8];
    sprintf(version,"%06x",nBurnVer);

    SFont_Write(screen, bluefont,176,5, version);
    SDL_UpdateRect(screen, 0, 0,0,0);*/
}