/*

MD Dumper CLI Version
X-death - 07/2024

MD Dumper SDL GUI Version
Jackobo Le Chocobo (Akina Usagi) - 31/08/2024

- Read mode automatic / manual
- Read mode bankswitch

*/

//Operating System Libraries and variables declarations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// SDL Libraries for GUI
#include <SDL.h>				//Main Library file
#include <SDL_image.h>			//For PNG loading files

#include <libusb.h>				//Library for detecting the MD Dumper device
#include "csv.h"
#include "csv.c"


// USB Special Command
#define WAKEUP          0x10	// WakeUP for first STM32 Communication
#define READ_MD         0x11
#define MAPPER_SSF2     0x20


// Version
#define MAX_VERSION 	2
#define MIN_VERSION 	1

char * game_rom = NULL;
char * game_name = NULL;

// CSV Gamelist specific Variable
#define BUFFER_SIZE 1024
#define MAX_NON_EMPTY_CELLS 50 // Ajustez cette valeur selon vos besoins
#define CHKSM_TEXT_SIZE 16
#define ROM_TEXT_SIZE 3
#define COL3_TEXT_SIZE 4 // Taille maximale pour les valeurs texte de la colonne C
#define TEXT_SIZE 16

FILE *fp;
struct csv_parser p;
size_t bytes_read;
unsigned char options = 0;

char buffer[BUFFER_SIZE];
size_t bytes_read;
int current_row = 0;
int current_col = 0;
char cell_A2[BUFFER_SIZE] = ""; // Variable pour stocker la donnée de la cellule A2
int non_empty_cells_in_col_A = 0; // Compteur de cellules non vides en colonne A
int Index_chksm=0;

// Tableau pour stocker les valeurs non vides en colonne A CHKSM sous forme de texte
char chksm_text_values[MAX_NON_EMPTY_CELLS][CHKSM_TEXT_SIZE + 1];
int chksm_text_values_count = 0;
// Tableau pour stocker les valeurs de la troisième colonne rom_size
char rom_size_text_values[MAX_NON_EMPTY_CELLS][CHKSM_TEXT_SIZE + 1];
int rom_size_text_values_count = 0;
// Tableau pour stocker les différentes valeurs du fichier csv
char txt_csv_chksm[4];
unsigned short csv_chksm=0;
unsigned char txt_csv_game_size[4+1];
unsigned char csv_game_size=0;
char txt_mapper_number[1];
unsigned char csv_mapper_number=0;
char txt_save_type[1];
unsigned char csv_save_type=0;
char txt_save_size[2];
unsigned char csv_save_size=0;

void cb1(void *s, size_t len, void *data)
	{
    int target_row = 0; // Index basé sur 0 (deuxième ligne a l'index 1)
    int col_A = 0;     // Index basé sur 0 (première colonne a l'index 0)
    int col_C = 1;     // Index basé sur 0 (troisième colonne a l'index 2)

    /* if (current_row == target_row && current_col == col_A) {
         // Copier la donnée dans la variable cell_A2
         strncpy(cell_A2, (char *)s, len);
         cell_A2[len] = '\0'; // Ajouter le caractère de fin de chaîne
     }*/

    // Compter les cellules non vides en colonne A et stocker les valeurs sous forme de texte
    if (current_row > 0 && current_col == col_A && len > 0)   // Ignorer la première ligne
		{
        non_empty_cells_in_col_A++;
        if (chksm_text_values_count < MAX_NON_EMPTY_CELLS)
			{
            // Assurer que la chaîne est de taille TEXT_SIZE
            strncpy(chksm_text_values[chksm_text_values_count], (char *)s, TEXT_SIZE);
            chksm_text_values[chksm_text_values_count][TEXT_SIZE] = '\0'; // Ajouter le caractère de fin de chaîne
            chksm_text_values_count++;
			}
		}
	}

// Fonction de rappel pour traiter la fin de chaque ligne
void cb2(int c, void *data)
	{
    current_row++;
    current_col = 0; // Réinitialiser le compteur de colonnes à la fin de chaque ligne
	}


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
	unsigned long save_size = 0;
	unsigned char *BufferROM;
	FILE *myfile;
	unsigned char NumberOfBank=0;
	unsigned char ActualBank=0;
	unsigned long offset=0;

	if (strcmp(argv[1], "-help") == 0) {
		printf("\nHow to use the program:\n\n");
		printf("GUI Mode:\n");
		printf("  %s -gui\n\n", argv[0]);
		printf("CLI Mode:\n");
		printf("  %s -read a  -  Auto Mode\n", argv[0]);
		printf("  %s -read b  -  Bankswitch Mode\n", argv[0]);
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
				case 2:						//Bankswitch Mode
					opts_mode = IMG_Load("./images/opts_dump_bankswitch.png");
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
						else if(mouse_y>=86 && mouse_y<=91)		{ dump_mode = 2; }	//Bankswitch Mode Selected
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

	//Init Lib CSV

    if (csv_init(&p, options) != 0)
		{
        printf("\n\n ERROR Failed to init CSV Parser for Gamelist ...\n");
        exit(EXIT_FAILURE);
		}
    csv_set_quote(&p,';');

    FILE *fp = fopen("gameslist.csv", "r");
    if (!fp)
		{
        printf("\n\n ERROR Can't find gamelist.csv ...\n");
        return EXIT_FAILURE;
		}

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
		{
        if (csv_parse(&p, buffer, bytes_read, cb1, cb2, NULL) != bytes_read)
			{
            buffer_header[i]=0x00;
            printf("\n\n ERROR while parsing file ...\n");
            return EXIT_FAILURE;
			}
		}
        i = 0;

    csv_fini(&p, cb1, cb2, NULL);
    csv_free(&p);
    fclose(fp);

    printf("\nCSV Gamelist file opened sucessfully\n");
	//Afficher le nombre de cellules non vides en colonne A
    printf("Add : %d Special Games into MD Dumper Database \n", non_empty_cells_in_col_A);

	//First try to read ROM MD Header

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
		if (strcmp(argv[2], "a") != 0 && strcmp(argv[2], "b") != 0 && strcmp(argv[2], "m") != 0) {
			printf("Le mode doit être 'a' (Automatique), 'b' (Bankswitch) ou 'm' (Manuel).\n");
			return 1;
			}
		
		if (strcmp(argv[2], "a") == 0) { dump_mode=0; }		//Mode Auto
		else if (strcmp(argv[2], "m") == 0)					//Mode Manuel
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
		else { dump_mode=2; }								//Mode Bankswitch
		}
	
    if ( dump_mode==0 )										//Mode Automatique
		{
		printf("\nRead ROM in automatic mode\n");

		printf("Sending command Dump ROM \n");
		printf("Dumping please wait ...\n");
		timer_start();
		address=0;
		game_size *= 1024;		//game_size=4096*1024;
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
    else if ( dump_mode==1 )								//Mode Manuel
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
    else if ( dump_mode==2 )								//Mode Bankswitch
		{
        printf("Read ROM in mode : Bankswitch SSF2 \n");
		i=0;
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

		// Search checksum cartridge in Custom Hardware games csv table
		i=0;
		for (i = 0; i < chksm_text_values_count ; i++)
			{
			strncpy(txt_csv_chksm,chksm_text_values[i],4);
			//printf(" \n txt chksm value : %s \n",txt_csv_chksm);
			csv_chksm = (unsigned short)strtol(txt_csv_chksm, NULL, 16);

            if ( checksum_header == csv_chksm  )
				{
				Index_chksm = i;
				printf("\nFound game in extra CSV Gamelist  \n");
				printf("Position in csv table %d \n",i);
				strncpy(txt_csv_game_size,chksm_text_values[i]+5,4);
				txt_csv_game_size[4] = '\0'; // Null-terminate the output string
				//printf(" txt game size : %s \n",txt_csv_game_size);
			    csv_game_size = (unsigned char)strtol(txt_csv_game_size, NULL, 10);
				//printf(" CSV Game Size  %d \n",csv_game_size);
				game_size=1024*csv_game_size;
				printf("ROM Size from CSV is %ld Ko \n",game_size);		
				}
			}
		NumberOfBank = game_size/512;
		//printf("Game Size is %ld Ko \n",game_size);
		printf("Number of Banks is %d \n",NumberOfBank);
		printf("Bank Size is 512 Ko  \n");

		game_size = game_size * 1024;
		BufferROM = (unsigned char*)malloc(game_size);
        // Cleaning ROM Buffer
        for (i=0; i<game_size; i++)
			{
            BufferROM[i]=0x00;
            }

		// Dump the first 4MB of the ROM as fast as possible
        printf("Bankswith bank O-7 to $080000 - $3FFFFF \n");

        printf("Dumping please wait ...\n");
        timer_start();
        address = 0;
        i=0;

        usb_buffer_out[0] = READ_MD;
        usb_buffer_out[1]=address & 0xFF;
        usb_buffer_out[2]=(address & 0xFF00)>>8;
        usb_buffer_out[3]=(address & 0xFF0000)>>16;
        usb_buffer_out[4]=1;

        libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
        printf("ROM dump in progress...\n");
        res = libusb_bulk_transfer(handle, 0x82,BufferROM,4096*1024, &numBytes,0);
        if (res != 0)
			{
            printf("Error \n");
            return 1;
            }

		ActualBank = 8;
		offset = 4096 - 1024;

		while ( offset != (game_size/1024)-1024)
			{
			printf("Bankswith bank %d - %d to $200000 - $2FFFFF \n",ActualBank,ActualBank+1);
            printf("Dumping please wait ...\n");
			
			address = 0xA130F9/2; // bank 4
            usb_buffer_out[0] = MAPPER_SSF2;
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=0;
            usb_buffer_out[5]=ActualBank;

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);

            // Send 0x09 to the bank 7

            address = 0xA130FB/2; // bank 5
            usb_buffer_out[0] = MAPPER_SSF2;
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=0;
            usb_buffer_out[5]=ActualBank+1;

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);

            address = (2048*1024)/2;

            // Dump lastMB

            // Do a simple read maybe needed for init bank or slow down the dumper :D
            usb_buffer_out[0] = READ_MD;
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=0;

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 0);
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 0);


            // Re-start the dump

            address = (2048*1024)/2;
            usb_buffer_out[0] = READ_MD;
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=1;

			offset = offset + 1024;
			ActualBank = ActualBank +2;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            res = libusb_bulk_transfer(handle, 0x82,BufferROM+offset*1024,1024*1024, &numBytes, 60000);
			}

		printf("\nDump ROM completed !\n");
		timer_end();
        timer_show();
        myfile = fopen("dump_smd.bin","wb");
        fwrite(BufferROM,1,game_size, myfile);
        fclose(myfile);
		}
    return 0;
	}


