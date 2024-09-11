/*

MD Dumper CLI Version
X-death - 07/2024

MD Dumper SDL GUI Version
Jackobo Le Chocobo (Akina Usagi) - 31/08/2024

*/

//Operating System Libraries and variables declarations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//MD Dumper Functions
#include "md_dumper_main.h"
#include "md_dumper_read.h"
#include "md_dumper_write.h"

int main(int argc, char *argv[])
{
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO); //Display informations on console

	if(argc == 1)
	{
		Display_Help(argv[0]);
		return 1;
	}
    else if (strcmp(argv[1], "-help") == 0)
    {
        Display_Help(argv[0]);
        return 1;
    }

    //Using GUI Mode ?
    if (strcmp(argv[1], "-gui") == 0)
    {
        use_gui=1;
        int quit = 0;
        SDL_Event event;
        int mouse_x, mouse_y;

        //Init Window
        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window * window = SDL_CreateWindow("MD Dumper version 1.0 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 424, 240, 0);
        SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

        //Create Background Texture
#if defined(_WIN32)
        SDL_Surface * image1 = IMG_Load(".\\images\\opts_background_read_default.png");
        SDL_Surface * image2 = IMG_Load(".\\images\\opts_background_read_ram_manual.png");
		SDL_Surface * image3 = IMG_Load(".\\images\\opts_background_read_rom_manual.png");
		SDL_Surface * image4 = IMG_Load(".\\images\\opts_background_write_default.png");
		SDL_Surface * image5 = IMG_Load(".\\images\\opts_background_write_ram.png");
#else
        SDL_Surface * image1 = IMG_Load("./images/opts_background_read_default.png");
        SDL_Surface * image2 = IMG_Load("./images/opts_background_read_ram_manual.png");
		SDL_Surface * image3 = IMG_Load("./images/opts_background_read_rom_manual.png");
		SDL_Surface * image4 = IMG_Load("./images/opts_background_write_default.png");
		SDL_Surface * image5 = IMG_Load("./images/opts_background_write_ram.png");
#endif

        while (quit==0)
        {
			SDL_Texture * texture;
            if(gui_tab_mode==0 && (dump_mode==0 || dump_mode==2))
				{
				texture = SDL_CreateTextureFromSurface(renderer, image1);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				}
			else if(gui_tab_mode==0 && data_type==1 && dump_mode==1)
				{
				texture = SDL_CreateTextureFromSurface(renderer, image2);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				}
			else if(gui_tab_mode==0 && data_type==0 && dump_mode==1)
				{
				texture = SDL_CreateTextureFromSurface(renderer, image3);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				}
			else if(gui_tab_mode==1 && data_type==0)
				{
				texture = SDL_CreateTextureFromSurface(renderer, image4);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				}
			else if(gui_tab_mode==1 && data_type==1)
				{
				texture = SDL_CreateTextureFromSurface(renderer, image5);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				}
				
            SDL_SetRenderDrawColor(renderer, 250, 173, 5, 255);

			if(gui_tab_mode==0)
			{
				switch(data_type)
				{
					case 0: //ROM								
						for (int x = 224; x <=230; x++)
							for (int y = 67; y <=73; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
					case 1: //RAM
						for (int x = 270; x <=276; x++)
							for (int y = 67; y <=73; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
				}
				
				switch(dump_mode)
				{
					case 0: //Auto
						for (int x = 224; x <=230; x++)
							for (int y = 84; y <=90; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
					case 1: //Manual
						for (int x = 270; x <=276; x++)
							for (int y = 84; y <=90; y++)
								SDL_RenderDrawPoint(renderer, x, y);
								
						switch(data_type)
						{
							case 0: //ROM								
								switch(dump_rom_size_opts)
								{
									case 0: //32
										for (int x = 20; x <=26; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 1: //64
										for (int x = 66; x <=72; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 2: //128
										for (int x = 112; x <=118; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 3: //256
										for (int x = 158; x <=164; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 4: //512
										for (int x = 204; x <=210; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 5: //1024
										for (int x = 20; x <=26; x++)
											for (int y = 157; y <=163; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 6: //2048
										for (int x = 66; x <=72; x++)
											for (int y = 157; y <=163; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 7: //4096
										for (int x = 112; x <=118; x++)
											for (int y = 157; y <=163; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 8: //8192
										for (int x = 158; x <=164; x++)
											for (int y = 157; y <=163; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
								}
								
								switch(dump_cart_mode_opts)
								{
									case 0: //GG
										for (int x = 250; x <=256; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 1: //MD
										for (int x = 296; x <=302; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 2: //SMS
										for (int x = 342; x <=348; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
								}
								break;
							case 1: //RAM
								for (int x = 270; x <=276; x++)
									for (int y = 67; y <=73; y++)
										SDL_RenderDrawPoint(renderer, x, y);
								
								switch(dump_sram_size_opts)
								{
									case 0: //8192
										for (int x = 20; x <=26; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 1: //32768
										for (int x = 66; x <=72; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
								}
								
								switch(dump_sram_type_opts)
								{
									case 0: //Parallel SRAM
										for (int x = 250; x <=256; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 1: //Serial I2C
										for (int x = 342; x <=348; x++)
											for (int y = 140; y <=146; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
									case 2: //Serial SPI
										for (int x = 250; x <=256; x++)
											for (int y = 157; y <=163; y++)
												SDL_RenderDrawPoint(renderer, x, y);
										break;
								}
								
								break;
						}		
						break;		
					case 2: //Bankswitch
						for (int x = 329; x <=335; x++)
							for (int y = 84; y <=90; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
				}		
			}
			else if(gui_tab_mode==1)
			{
				if(data_type==1)
				{
					switch(dump_sram_type_opts)
					{
						case 0: //Parallel SRAM
							for (int x = 250; x <=256; x++)
								for (int y = 140; y <=146; y++)
									SDL_RenderDrawPoint(renderer, x, y);
							break;
						case 1: //Serial I2C
							for (int x = 342; x <=348; x++)
								for (int y = 140; y <=146; y++)
									SDL_RenderDrawPoint(renderer, x, y);
							break;
						case 2: //Serial SPI
							for (int x = 250; x <=256; x++)
								for (int y = 157; y <=163; y++)
									SDL_RenderDrawPoint(renderer, x, y);
							break;
					}
				}
				
				switch(erase_or_write)
				{
					case 0: //Erase								
						for (int x = 224; x <=230; x++)
							for (int y = 67; y <=73; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
					case 1: //Write
						for (int x = 282; x <=288; x++)
							for (int y = 67; y <=73; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
				}	
				
				switch(data_type)
				{
					case 0: //ROM								
						for (int x = 224; x <=230; x++)
							for (int y = 84; y <=90; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
					case 1: //RAM
						for (int x = 282; x <=288; x++)
							for (int y = 84; y <=90; y++)
								SDL_RenderDrawPoint(renderer, x, y);
						break;
				}	
			}
					
			//Display Texture	
			SDL_RenderPresent(renderer);
		
			SDL_GetMouseState(&mouse_x, &mouse_y);
			SDL_WaitEvent(&event);
				 
			switch (event.type)		//Window Events according to mouse positions and left click on this Window
			{
				case SDL_QUIT:
					quit = 1;
					SDL_DestroyTexture(texture);
					SDL_FreeSurface(image1);
					SDL_FreeSurface(image2);
					SDL_FreeSurface(image3);
					SDL_FreeSurface(image4);
					SDL_FreeSurface(image5);
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
					SDL_Quit();
					return 1;
				case SDL_MOUSEBUTTONDOWN:
					
					if(mouse_x>=18  && mouse_x<=108) 
						{
						if (mouse_y>=46  && mouse_y<=58)
							gui_tab_mode = 0;
						}
					else if(mouse_x>=117  && mouse_x<=207) 
						{
						if (mouse_y>=46  && mouse_y<=58)
							gui_tab_mode = 1;
						}
					else if(mouse_x>=337 && mouse_x<=419)
						{
						if(mouse_y>=214 && mouse_y<=235)	//Launch
							{
							quit = 1;
							SDL_DestroyTexture(texture);
							SDL_FreeSurface(image1);
							SDL_FreeSurface(image2);
							SDL_FreeSurface(image3);
							SDL_FreeSurface(image4);
							SDL_FreeSurface(image5);
							SDL_DestroyRenderer(renderer);
							SDL_DestroyWindow(window);
							SDL_Quit();
							break;
							}
						}
					
					if(gui_tab_mode==0 && (dump_mode==0 || dump_mode==2))
						{
						if(mouse_x>=222  && mouse_x<=232) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								data_type = 0;
							else if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 0;
							}
						else if(mouse_x>=268  && mouse_x<=278) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								data_type = 1;
							else if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 1;
							}
						else if(mouse_x>=327  && mouse_x<=337) 
							{
							if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 2;
							}
						}
					else if(gui_tab_mode==0 && data_type==1 && dump_mode==1)
						{
						if(mouse_x>=222  && mouse_x<=232) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								data_type = 0;
							else if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 0;
							}
						else if(mouse_x>=268  && mouse_x<=278) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								data_type = 1;
							else if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 1;
							}
						else if(mouse_x>=327  && mouse_x<=337) 
							{
							if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 2;
							}
						else if(mouse_x>=18  && mouse_x<=28) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_sram_size_opts = 0;
							}
						else if(mouse_x>=64  && mouse_x<=74) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_sram_size_opts = 1;
							}
						
						else if(mouse_x>=248  && mouse_x<=258) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_sram_type_opts = 0;
							else if (mouse_y>=155  && mouse_y<=165)
								dump_sram_type_opts = 2;
							}
						else if(mouse_x>=340  && mouse_x<=350) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_sram_type_opts = 1;
							}						
						}
					else if(gui_tab_mode==0 && data_type==0 && dump_mode==1)
						{
						if(mouse_x>=222  && mouse_x<=232) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								data_type = 0;
							else if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 0;
							}
						else if(mouse_x>=268  && mouse_x<=278) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								data_type = 1;
							else if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 1;
							}
						else if(mouse_x>=327  && mouse_x<=337) 
							{
							if (mouse_y>=82  && mouse_y<=92)
								dump_mode = 2;
							}
						else if(mouse_x>=18  && mouse_x<=28) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_rom_size_opts = 0;
							else if (mouse_y>=155  && mouse_y<=165)
								dump_rom_size_opts = 5;
							}
						else if(mouse_x>=64  && mouse_x<=74) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_rom_size_opts = 1;
							else if (mouse_y>=155  && mouse_y<=165)
								dump_rom_size_opts = 6;
							}
						else if(mouse_x>=110  && mouse_x<=120) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_rom_size_opts = 2;
							else if (mouse_y>=155  && mouse_y<=165)
								dump_rom_size_opts = 7;
							}
						else if(mouse_x>=156  && mouse_x<=166) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_rom_size_opts = 3;
							else if (mouse_y>=155  && mouse_y<=165)
								dump_rom_size_opts = 8;
							}
						else if(mouse_x>=202  && mouse_x<=212) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_rom_size_opts = 4;
							}
						else if(mouse_x>=248  && mouse_x<=258) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_cart_mode_opts = 0;
							}
						else if(mouse_x>=294  && mouse_x<=304) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_cart_mode_opts = 1;
							}
						else if(mouse_x>=340  && mouse_x<=350) 
							{
							if (mouse_y>=138  && mouse_y<=148)
								dump_cart_mode_opts = 2;
							}
						}
					else if(gui_tab_mode==1)
						{
						if(data_type==1)
							{
							if(mouse_x>=248  && mouse_x<=258) 
								{
								if (mouse_y>=138  && mouse_y<=148)
									dump_sram_type_opts = 0;
								else if (mouse_y>=155  && mouse_y<=165)
									dump_sram_type_opts = 2;
								}
							else if(mouse_x>=340  && mouse_x<=350) 
								{
								if (mouse_y>=138  && mouse_y<=148)
									dump_sram_type_opts = 1;
								}		
							}
						if(mouse_x>=222  && mouse_x<=232) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								erase_or_write = 0;
							else if (mouse_y>=82  && mouse_y<=92)
								data_type = 0;
							}
						else if(mouse_x>=280  && mouse_x<=290) 
							{
							if (mouse_y>=65  && mouse_y<=75)
								erase_or_write = 1;
							else if (mouse_y>=82  && mouse_y<=92)
								data_type = 1;
							}
						}
			}
        }
    }
    else
    {
        SDL_Log("\n");
        SDL_Log("----------------------------------------------------------------------------\n");
        SDL_Log("8b   d8 888b.      888b. 8    8 8b   d8 888b. 8888 888b.      .d88b 8    888\n");
        SDL_Log("8YbmdP8 8   8      8   8 8    8 8YbmdP8 8  .8 8www 8  .8      8P    8     8\n");
        SDL_Log("8     8 8   8 wwww 8   8 8b..d8 8     8 8wwP' 8    8wwK'      8b    8     8\n");
        SDL_Log("8     8 888P'      888P' `Y88P' 8     8 8     8888 8  Yb wwww `Y88P 8888 888\n");
        SDL_Log("----------------------------------------------------------------------------\n");
        SDL_Log("\n");
        SDL_Log("Release : 1.0 alpha \n");
        SDL_Log("\n");
    }

    //LibUsb : Init & Detect
    if(Detect_Device()!=0)
        return 1;
    
    //LibCsv : Init & Open files
    if(Open_CSV_Files()!=0)
        return 1;

    //Read Game Header/Infos
    Game_Header_Infos();

    // Vérifier le nombre d'arguments
    if(use_gui==0)						//Vérifier que nous utilisons le mode CLI
    {
        //Lecture de la ROM
        if (strcmp(argv[1], "-read") == 0)
		{
            if (strcmp(argv[2], "auto")==0)
            {
				gui_tab_mode=0;
				data_type=0;
                dump_mode=0;
			}
			else  if (strcmp(argv[2], "manual")==0)
            {
				gui_tab_mode=0;
				data_type=0;
                dump_mode=1;
                
                if (strcmp(argv[3], "32") == 0)
                {
                    dump_rom_size_opts = 0;
                }
                else if (strcmp(argv[3], "64") == 0)
                {
                    dump_rom_size_opts = 1;
                }
                else if (strcmp(argv[3], "128") == 0)
                {
                    dump_rom_size_opts = 2;
                }
                else if (strcmp(argv[3], "256") == 0)
                {
                    dump_rom_size_opts = 3;
                }
                else if (strcmp(argv[3], "512") == 0)
                {
                    dump_rom_size_opts = 4;
                }
                else if (strcmp(argv[3], "1024") == 0)
                {
                    dump_rom_size_opts = 5;
                }
                else if (strcmp(argv[3], "2048") == 0)
                {
                    dump_rom_size_opts = 6;
                }
                else if (strcmp(argv[3], "4096") == 0)
                {
                    dump_rom_size_opts = 7;
                }
				else if (strcmp(argv[3], "8192") == 0)
                {
                    dump_rom_size_opts = 8;
                }
                else
                {
                    SDL_Log("You must write one of the following values to set the game size : 32, 64, 128, 256, 512, 1024, 2048, 4096.\n");
                    return 1;
                }
                
                if (strcmp(argv[4], "gg") == 0)
                {
                    dump_cart_mode_opts = 0;
                }
                else if (strcmp(argv[4], "md") == 0)
                {
                    dump_cart_mode_opts = 1;
                }
                else if (strcmp(argv[4], "sms") == 0)
                {
                    dump_cart_mode_opts = 2;
                }
                else
                {
                    SDL_Log("You must write one of the following values to set the cartridge type : gg, md or sms.\n");
                    return 1;
                }
			}
            else if (strcmp(argv[2], "bankswitch") == 0)
            {
				gui_tab_mode=0;
				data_type=0;
                dump_mode=2;
            }
            else
			{
                SDL_Log("You must select 'a' (Auto), 'b' (Bankswitch) or 'm' (Manual).\n");
                return 1;
			}
        }
        else if (strcmp(argv[1], "-backup") == 0)
		{
            if (strcmp(argv[2], "auto")==0)
            {
				gui_tab_mode=0;
				data_type=1;
                dump_mode=0;
			}
			else  if (strcmp(argv[2], "manual")==0)
            {
				gui_tab_mode=0;
				data_type=1;
                dump_mode=1;
                
                if (strcmp(argv[3], "8192") == 0)
                {
                    dump_sram_size_opts = 0;
                }
                else if (strcmp(argv[3], "32768") == 0)
                {
                    dump_sram_size_opts = 1;
                }
                else
                {
                    SDL_Log("You must write one of the following values to set the game size : 8192, 32768.\n");
                    return 1;
                }
                
                if (strcmp(argv[4], "parallel_sram") == 0)
                {
                    dump_sram_type_opts = 0;
                }
                else if (strcmp(argv[4], "serial_i2c") == 0)
                {
                    dump_sram_type_opts = 1;
                }
                else if (strcmp(argv[4], "serial_spi") == 0)
                {
                    dump_sram_type_opts = 2;
                }
                else
                {
                    SDL_Log("You must write one of the following values to set the cartridge type : parallel_sram, serial_i2c or serial_spi.\n");
                    return 1;
                }
			}
            else if (strcmp(argv[2], "bankswitch") == 0)
            {
				gui_tab_mode=0;
				data_type=1;
                dump_mode=2;
            }
            else
			{
                SDL_Log("You must select 'a' (Auto), 'b' (Bankswitch) or 'm' (Manual).\n");
                return 1;
			}
        }
        else if (strcmp(argv[1], "-erase_flash") == 0)
        {
			gui_tab_mode=1;
			data_type=0;
            erase_or_write=0;
		}
		else if (strcmp(argv[1], "-write_flash") == 0)
        {
			gui_tab_mode=1;
			data_type=0;
            erase_or_write=1;
		}
		else if (strcmp(argv[1], "-erase_memory") == 0)
        {
			gui_tab_mode=1;
			data_type=1;
            erase_or_write=0;
            
            if (strcmp(argv[2], "serial_spi") == 0)
				dump_sram_type_opts = 2;
            else if (strcmp(argv[2], "serial_i2c") == 0)
                dump_sram_type_opts = 1;
            else if (strcmp(argv[2], "parallel_sram") == 0)
                dump_sram_type_opts = 0;
            else
            {
                SDL_Log("You must write one of the following values to select the save type : serial_spi, serial_i2c, parallel_sram.\n");
                return 1;
            }
		}
		else if (strcmp(argv[1], "-write_memory") == 0)
        {
			gui_tab_mode=1;
			data_type=1;
            erase_or_write=1;
            
            if (strcmp(argv[2], "serial_spi") == 0)
				dump_sram_type_opts = 2;
            else if (strcmp(argv[2], "serial_i2c") == 0)
                dump_sram_type_opts = 1;
            else if (strcmp(argv[2], "parallel_sram") == 0)
                dump_sram_type_opts = 0;
            else
            {
                SDL_Log("You must write one of the following values to select the save type : serial_spi, serial_i2c, parallel_sram.\n");
                return 1;
            }
		}
        //Erreur
        else
        {
            SDL_Log("You must write '-read', '-backup', '-erase_flash', '-write_flash', '-erase_memory' or '-write_memory' .\n");
            return 1;
        }
    }

    if ( gui_tab_mode==0 && data_type==0 && dump_mode==0 )
    {
        return Read_ROM_Auto();
    }
    else if ( gui_tab_mode==0 && data_type==0 && dump_mode==1 )
    {
        return Read_ROM_Manual();
    }
    else if ( gui_tab_mode==0 && data_type==0 && dump_mode==2 )
    {
        return Read_ROM_Bankswitch();
    }
    else if ( gui_tab_mode==0 && data_type==1 && dump_mode==0 )
    {
        return Read_RAM_Auto();
    }
    else if ( gui_tab_mode==0 && data_type==1 && dump_mode==1 )
    {
        return Read_RAM_Manual();
    }
    else if ( gui_tab_mode==0 && data_type==1 && dump_mode==2 )
    {
        return Read_RAM_Bankswitch();
    }
    else if ( gui_tab_mode==1 && data_type==0 && erase_or_write==0 )
    {
        return Erase_Flash();
    }
    else if ( gui_tab_mode==1 && data_type==0 && erase_or_write==1 )
    {
        return Write_Flash();
    }
    else if ( gui_tab_mode==1 && data_type==1 && erase_or_write==0 )
    {
        return Erase_RAM();
    }
    else if ( gui_tab_mode==1 && data_type==1 && erase_or_write==1 )
    {
        return Write_RAM();
    }
    return 0;
}


