/*

MD Dumper CLI Version
X-death - 07/2024

MD Dumper SDL GUI Version
Jackobo Le Chocobo (Akina Usagi) - 31/08/2024

-TODO-
-Read mode automatic / manual

*/

//Operating System Libraries and variables declarations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SDL Libraries for GUI
#include <SDL.h>				//Main Library file
#include <SDL_image.h>			//For PNG loading files

#include <libusb.h>				//Library for detecting the MD Dumper device


// USB Special Command
#define WAKEUP          0x10	// WakeUP for first STM32 Communication
#define READ_MD         0x11

// Version
#define MAX_VERSION 	2
#define MIN_VERSION 	1

char * game_rom = NULL;
char * game_name = NULL;


//Timer functions according to Operating Systems
#if defined(_WIN32)		//Windows
	clock_t microsec_start;
	clock_t microsec_end;
	
	void timer_start() 	{ microsec_start = clock();		}
	void timer_end()	{ microsec_end = clock();		}
	void timer_show()
		{
		printf("~ Elapsed time: %lds", (microsec_end - microsec_start)/1000);
		printf(" (%ldms)\n", (microsec_end - microsec_start));
		}
#else 					//Others
	struct timeval ostime;
	long microsec_start = 0;
	long microsec_end = 0;
	
	void timer_start()
		{     
		gettimeofday(&ostime, NULL);
		microsec_start = ((unsigned long long)ostime.tv_sec * 1000000) + ostime.tv_usec;
		}

	void timer_end()
		{
		gettimeofday(&ostime, NULL);
		microsec_end = ((unsigned long long)ostime.tv_sec * 1000000) + ostime.tv_usec;
		}

	void timer_show()
		{
		printf("~ Elapsed time: %lds", (microsec_end - microsec_start)/1000000);
		printf(" (%ldms)\n", (microsec_end - microsec_start)/1000);
		}
#endif


unsigned int trim(unsigned char * buf, unsigned char is_out)
	{
    unsigned char i=0, j=0;
    unsigned char tmp[49] = {0}; //max
    unsigned char tmp2[49] = {0}; //max
    unsigned char next = 1;

    /*check ascii remove unwanted ones and transform upper to lowercase*/
    if(is_out)
		{
		while(i<48)
			{
            if(buf[i]<0x30 || buf[i]>0x7A || (buf[i]>0x29 && buf[i]<0x41) || (buf[i]>0x5A && buf[i]<0x61))
                buf[i] = 0x20; //remove shiiit
            if(buf[i]>0x40 && buf[i]<0x5B)
                buf[i] += 0x20; //to lower case A => a
            i++;
			}
			i=0;
		}

    while(i<48)
		{
        if(buf[i]!=0x20)
			{
            if(buf[i]==0x2F)
                buf[i] = '-';
            tmp[j]=buf[i];
            tmp2[j]=buf[i];
            next = 1;
            j++;
			}
        else
			{
            if(next)
				{
                tmp[j]=0x20;
                tmp2[j]='_';
                next = 0;
                j++;
				}
			}
			i++;
		}

    next=0;
    if(tmp2[0]=='_')
		{
        next=1;    //offset
		}
    if(tmp[(j-1)]==0x20)
		{
        tmp[(j-1)] = tmp2[(j-1)]='\0';
		}
    else
		{
        tmp[j] = tmp2[j]='\0';
		}

    if(is_out)  //+4 for extension
		{
        game_rom = (char *)malloc(j-next +4);
        memcpy((unsigned char *)game_rom, (unsigned char *)tmp2 +next, j-next); //stringed file
		}

    game_name = (char *)malloc(j-next);
    memcpy((unsigned char *)game_name, (unsigned char *)tmp +next, j-next); //trimmed
    return 0;
	}

int main(int argc, char *argv[]) 
    {
	// LibUSB Specific Var
    int res                      = 0;        /* return codes from libusb functions */
    int kernelDriverDetached     = 0;        /* Set to 1 if kernel driver detached */
    unsigned long len            = 0;        /* Number of bytes transferred. */
    unsigned char usb_buffer_out[64] = {0};  /* 64 byte transfer buffer OUT */
    libusb_device_handle* handle = 0;        /* handle for USB device */
    int numBytes                 = 0;        /* Actual bytes transferred. */
    unsigned char usb_buffer_in[64] = {0};   /* 64 byte transfer buffer IN */

	// MD Dumper Var
	unsigned long address=0;
	unsigned long i=0;
	unsigned char md_dumper_type=0;
	unsigned long j=0;
	unsigned char *buffer_header = NULL;
	char dump_name[64];
	unsigned char region[5];
	char *game_region = NULL;
	const char unk[] = {"unknown"};
	int checksum_header = 0;
	int use_gui=0;							/* 0=CLI Mode, 1=GUI Mode */
	int dump_mode=0; 						/* 0=Auto, 1=Manual */
	int dump_manual_size_opts=0; 			/* 0=32KB, 1=64KB, 2=128KB, 3=256KB, 4=512KB, 5=1024KB, 6=2048KB, 7=4096KB */
	int game_size=0;
	int manual_game_size=0;
	unsigned char *BufferROM;
	FILE *myfile;

	if (argc < 3 && strcmp(argv[1], "-gui") != 0) {
		printf("\nHow to use the program:\n\n");
		printf("GUI Mode:\n");
		printf("  %s -gui\n\n", argv[0]);
		printf("CLI Mode:\n");
		printf("  %s -read a  -  Auto Mode\n", argv[0]);
		printf("  %s -read m (32|64|128|256|512|1024|2048|4096) -  Manual Mode\n", argv[0]);
		printf("\n");
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
		SDL_Window * window = SDL_CreateWindow("MD Dumper version 1.0 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 263, 0);
		SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

		//Create Background Texture
		SDL_Surface * image = IMG_Load("./images/opts_background.png");
		SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, image);

		while (quit==0)
			{    		
			SDL_GetMouseState(&mouse_x, &mouse_y);
			SDL_Surface * opts_mode;			SDL_Texture * texture2;
			SDL_Surface * opts_manual_size;		SDL_Texture * texture3;
			
			switch(dump_mode)				//Create Mode Texture
				{
				case 0:						//Auto Mode
					opts_mode = IMG_Load("./images/opts_dump_auto.png");
					texture2 = SDL_CreateTextureFromSurface(renderer, opts_mode);
					break;
				case 1:						//Manual Mode
					opts_mode = IMG_Load("./images/opts_dump_manual.png");
					texture2 = SDL_CreateTextureFromSurface(renderer, opts_mode);
					break;
				}

			switch(dump_manual_size_opts)	//Create Manual Size Texture
				{
				case 0:						//32KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_32.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 32;
					break;
				case 1:						//64KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_64.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 64;
					break;
				case 2:						//128KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_128.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 128;
					break;
				case 3:						//256KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_256.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 256;
					break;
				case 4:						//512KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_512.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 512;
					break;
				case 5:						//1024KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_1024.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 1024;
					break;
				case 6:						//2048KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_2048.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 2048;
					break;
				case 7:						//4096KB
					opts_manual_size = IMG_Load("./images/opts_dump_manual_size_4096.png");
					texture3 = SDL_CreateTextureFromSurface(renderer, opts_manual_size);
					manual_game_size = 4096;
					break;
				}
			
			//Display Texture	
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderCopy(renderer, texture2, NULL, NULL);
			SDL_RenderCopy(renderer, texture3, NULL, NULL);
			SDL_RenderPresent(renderer);
		
			SDL_WaitEvent(&event);
	 
			switch (event.type)		//Window Events according to mouse positions and left click on this Window
				{
				case SDL_QUIT:
					quit = 1;
					SDL_DestroyTexture(texture);
					SDL_FreeSurface(image);
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
					SDL_Quit();
					return 1;
				case SDL_MOUSEBUTTONDOWN:
					if(mouse_x>=17 && mouse_x<=24)
						{
						if(mouse_y>=49 && mouse_y<=56)			{ dump_mode = 0; }	//Auto Mode Selected
						else if(mouse_y>=67 && mouse_y<=74)		{ dump_mode = 1; }	//Manual Mode Selected
						}
					else if(mouse_x>=215 && mouse_x<=221)
						{
						if(mouse_y>=101 && mouse_y<=106)		{ dump_manual_size_opts = 0; }	//Manual Size : 32KB
						else if(mouse_y>=117 && mouse_y<=122)	{ dump_manual_size_opts = 4; }	//Manual Size : 512KB
						}
					else if(mouse_x>=335 && mouse_x<=340)
						{
						if(mouse_y>=101 && mouse_y<=106)		{ dump_manual_size_opts = 1; }	//Manual Size : 64KB
						else if(mouse_y>=117 && mouse_y<=122)	{ dump_manual_size_opts = 5; }	//Manual Size : 1024KB
						}
					else if(mouse_x>=455 && mouse_x<=460)
						{
						if(mouse_y>=101 && mouse_y<=106)		{ dump_manual_size_opts = 2; }	//Manual Size : 128KB
						else if(mouse_y>=117 && mouse_y<=122)	{ dump_manual_size_opts = 6; }	//Manual Size : 2048KB
						}
					else if(mouse_x>=575 && mouse_x<=580)
						{
						if(mouse_y>=101 && mouse_y<=106)		{ dump_manual_size_opts = 3; }	//Manual Size : 286KB
						else if(mouse_y>=117 && mouse_y<=122)	{ dump_manual_size_opts = 7; }	//Manual Size : 4096KB
						}
					if(mouse_x>=16 && mouse_x<=199)
						{
						if(mouse_y>=183 && mouse_y<=214)			//Exit
							{
							quit = 1;
							SDL_DestroyTexture(texture);
							SDL_FreeSurface(image);
							SDL_DestroyRenderer(renderer);
							SDL_DestroyWindow(window);
							SDL_Quit();
							return 1;
							}
						}
					if(mouse_x>=228 && mouse_x<=411)
						{
						if(mouse_y>=183 && mouse_y<=214)			//Launch the dump
							{
							quit = 1;
							break;
							}
						}
					if(mouse_x>=440 && mouse_x<=623)
						{
						if(mouse_y>=183 && mouse_y<=214)			//Manual / PDF
							{
							printf("Open Manual : TODO\n");
							}
						}
					break;
				}
			}
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(image);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		}
	else
		{
		printf("----------------------------------------------------------------------------\n");
		printf("8b   d8 888b.      888b. 8    8 8b   d8 888b. 8888 888b.      .d88b 8    888\n"); 
		printf("8YbmdP8 8   8      8   8 8    8 8YbmdP8 8  .8 8www 8  .8      8P    8     8\n");  
		printf("8     8 8   8 wwww 8   8 8b..d8 8     8 8wwP' 8    8wwK'      8b    8     8\n");  
		printf("8     8 888P'      888P' `Y88P' 8     8 8     8888 8  Yb wwww `Y88P 8888 888\n");
		printf("----------------------------------------------------------------------------\n");
		printf("\nRelease : 1.0 alpha \n\n");
		}
	
    /* Initialise libusb. */
	printf("Init LibUSB... \n");
    res = libusb_init(0);
    if (res != 0)
    {
        fprintf(stderr, "Error initialising libusb.\n");
        return 1;
    }

    printf("LibUSB Init Sucessfully ! \n");


    printf("Detecting MD Dumper... \n");

    /* Get the first device with the matching Vendor ID and Product ID. If
     * intending to allow multiple demo boards to be connected at once, you
     * will need to use libusb_get_device_list() instead. Refer to the libusb
     * documentation for details. */

    handle = libusb_open_device_with_vid_pid(0, 0x0483, 0x5740);

    if (!handle)
    {
        fprintf(stderr, "Unable to open device.\n");
        return 1;
    }

    /* Claim interface #0. */

    res = libusb_claim_interface(handle, 0);
    if (res != 0)
    {
        res = libusb_claim_interface(handle, 1);
        if (res != 0)
        {
            printf("Error claiming interface.\n");
            return 1;
        }
    }

	// Clean Buffer
    for (i = 0; i < 64; i++)
    {
        usb_buffer_in[i]=0x00;
        usb_buffer_out[i]=0x00;
    }
    i=0;

	usb_buffer_out[0] = WAKEUP;// Affect request to  WakeUP Command
    libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 0); // Send Packets to Sega Dumper
    libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 0);
	printf("\nMD Dumper %.*s",6, (char *)usb_buffer_in);
    printf("\n");
	md_dumper_type = usb_buffer_in[24];
	if ( md_dumper_type == 0 )
		{
        printf("MD Dumper type : Old Version \n");
		}
	if ( md_dumper_type == 1 )
		{
        printf("MD Dumper type : BluePill non aligned \n");
		}
	if ( md_dumper_type == 2 )
		{
        printf("MD Dumper type : BluePill aligned \n");
		}
	if ( md_dumper_type == 3 )
		{
        printf("MD Dumper type : SMD ARM TQFP100 Aligned  \n");
		}
	printf("Hardware Firmware version : %d", usb_buffer_in[20]);
    printf(".%d\n", usb_buffer_in[21]);

	// Read ROM header

	// First try to read ROM MD Header

        buffer_header = (unsigned char *)malloc(0x200);
        i = 0;
        address = 0x80;

        // Cleaning header Buffer
        for (i=0; i<512; i++)
        {
            buffer_header[i]=0x00;
        }

        i = 0;


        while (i<8)
        {

            usb_buffer_out[0] = READ_MD;
            usb_buffer_out[1] = address&0xFF ;
            usb_buffer_out[2] = (address&0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4] = 0; // Slow Mode

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82,buffer_header+(64*i),64, &numBytes, 60000);
            address+=32;
            i++;
        }

		if(memcmp((unsigned char *)buffer_header,"SEGA",4) == 0)
        {
            printf("\nMegadrive/Genesis/32X cartridge detected!\n");
            printf("\n --- HEADER ---\n");
            memcpy((unsigned char *)dump_name, (unsigned char *)buffer_header+32, 48);
            trim((unsigned char *)dump_name, 0);
            printf(" Domestic: %.*s\n", 48, (char *)game_name);
            memcpy((unsigned char *)dump_name, (unsigned char *)buffer_header+80, 48);
            trim((unsigned char *)dump_name, 0);

            printf(" International: %.*s\n", 48, game_name);
            printf(" Release date: %.*s\n", 16, buffer_header+0x10);
            printf(" Version: %.*s\n", 14, buffer_header+0x80);
            memcpy((unsigned char *)region, (unsigned char *)buffer_header +0xF0, 4);
            for(i=0; i<4; i++)
            {
                if(region[i]==0x20)
                {
                    game_region = (char *)malloc(i);
                    memcpy((unsigned char *)game_region, (unsigned char *)buffer_header +0xF0, i);
                    game_region[i] = '\0';
                    break;
                }
            }

            if(game_region[0]=='0')
            {
                game_region = (char *)malloc(4);
                memcpy((char *)game_region, (char *)unk, 3);
                game_region[3] = '\0';
            }

            printf(" Region: %s\n", game_region);

            checksum_header = (buffer_header[0x8E]<<8) | buffer_header[0x8F];
            printf(" Checksum: %X\n", checksum_header);

            game_size = 1 + ((buffer_header[0xA4]<<24) | (buffer_header[0xA5]<<16) | (buffer_header[0xA6]<<8) | buffer_header[0xA7])/1024;
            printf(" Game size: %dKB\n", game_size);
		}

	// Vérifier le nombre d'arguments
	if(use_gui==0)						//Vérifier que nous utilisons le mode CLI
		{
		// Vérifier le premier argument
		if (strcmp(argv[1], "-read") != 0) {
			printf("Premier argument doit être '-read'.\n");
			return 1;
			}

		// Vérifier le deuxième argument
		if (strcmp(argv[2], "a") != 0 && strcmp(argv[2], "m") != 0) {
			printf("Le mode doit être 'a' (Automatique) ou 'm' (Manuel).\n");
			return 1;
			}
		
		if (strcmp(argv[2], "a") == 0) { dump_mode=0; }		//Mode Auto
		else 												//Mode Manuel
			{
			dump_mode=1; 
			// Vérifier le 3ème argument
			if (strcmp(argv[3], "32") == 0) 		{ manual_game_size = 32; 	}
			else if (strcmp(argv[3], "64") == 0) 	{ manual_game_size = 64; 	}
			else if (strcmp(argv[3], "128") == 0)	{ manual_game_size = 128; 	}
			else if (strcmp(argv[3], "256") == 0) 	{ manual_game_size = 256; 	}
			else if (strcmp(argv[3], "512") == 0) 	{ manual_game_size = 512; 	}
			else if (strcmp(argv[3], "1024") == 0) 	{ manual_game_size = 1024; 	}
			else if (strcmp(argv[3], "2048") == 0) 	{ manual_game_size = 2048; 	}
			else if (strcmp(argv[3], "4096") == 0) 	{ manual_game_size = 4096; 	}
			else
				{
				printf("Vous devez écrire une des valeurs suivantes : 32, 64, 128, 256, 512, 1024, 2048, 4096.\n");
				return 1;
				}	
			}								
		}
	

    // Afficher le mode de lecture
    if ( dump_mode==0 )
		{
		printf("\nRead ROM in automatic mode\n");

		printf("Sending command Dump ROM \n");
		printf("Dumping please wait ...\n");
		timer_start();
		address=0;
		game_size *= 1024;
		//	game_size=4096*1024;
		printf("\nRom Size : %ld Ko \n",game_size/1024);
		BufferROM = (unsigned char*)malloc(game_size);
		// Cleaning ROM Buffer
		for (i=0; i<game_size; i++)
			{
			BufferROM[i]=0x00;
			}

		usb_buffer_out[0] = READ_MD;
		usb_buffer_out[1]=address & 0xFF;
		usb_buffer_out[2]=(address & 0xFF00)>>8;
		usb_buffer_out[3]=(address & 0xFF0000)>>16;
		usb_buffer_out[4]=1;

		libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 0);
		printf("ROM dump in progress...\n");
		res = libusb_bulk_transfer(handle, 0x82,BufferROM,game_size, &numBytes, 0);
		if (res != 0)
			{
			printf("Error \n");
			return 1;
			}
		printf("\nDump ROM completed !\n");
		timer_end();
		timer_show();
		myfile = fopen("dump_smd.bin","wb");
		fwrite(BufferROM, 1,game_size, myfile);
		fclose(myfile);
		}
    else
		{
        printf("Read ROM in manual mode\n");
        
		printf("Sending command Dump ROM \n");
		printf("Dumping please wait ...\n");
		timer_start();
		address=0;
		game_size = manual_game_size * 1024;
		printf("\nRom Size (Manual Mode) : %ld Ko \n",game_size/1024);
		BufferROM = (unsigned char*)malloc(game_size);
		// Cleaning ROM Buffer
		for (i=0; i<game_size; i++)
			{
			BufferROM[i]=0x00;
			}

		usb_buffer_out[0] = READ_MD;
		usb_buffer_out[1]=address & 0xFF;
		usb_buffer_out[2]=(address & 0xFF00)>>8;
		usb_buffer_out[3]=(address & 0xFF0000)>>16;
		usb_buffer_out[4]=1;

		libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 0);
		printf("ROM dump in progress...\n");
		res = libusb_bulk_transfer(handle, 0x82,BufferROM,game_size, &numBytes, 0);
		if (res != 0)
			{
			printf("Error \n");
			return 1;
			}
		printf("\nDump ROM completed !\n");
		timer_end();
		timer_show();
		myfile = fopen("dump_smd.bin","wb");
		fwrite(BufferROM, 1,game_size, myfile);
		fclose(myfile);
		}
    return 0;
}


