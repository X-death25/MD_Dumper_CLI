// SDL Libraries for GUI
#include <SDL.h>				//Main Library file
#include <SDL_image.h>			//For PNG loading files

#include <libusb.h>				//Library for detecting the MD Dumper device

//Read CSV Files
#include "csv.h"
#include "csv.c"

//File Manager Library
#include "sfd.h"
#include "sfd.c"

// USB Special Command
#define WAKEUP             0x10	 // WakeUP for first STM32 Communication
#define READ_MD            0x11
#define READ_MD_SAVE       0x12
#define WRITE_MD_SAVE      0x13
#define WRITE_MD_FLASH 	   0x14
#define ERASE_MD_FLASH     0x15
#define READ_SMS           0x16
#define INFOS_ID           0x18
#define MAPPER_SSF2        0x20
#define CREATE_MX_BUFFER   0x60
#define WRITE_MX_BUFFER    0x61
#define FLASH_MX_BUFFER    0x62
#define SECTOR_ERASE       0x64
#define SEND_FLASH_ALGO    0x65


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

// csv SMS-GG CRC specific Variable

#define MAX_NON_EMPTY_CELLS2 2500 // Ajustez cette valeur selon vos besoins
#define TEXT_SIZE3 97 // taille de toute la chaine
#define BUFFER_SIZE3 4096

FILE *fp3;
struct csv_parser p3;
char buffer3[BUFFER_SIZE3];

int current_row3 = 0;
int current_col3 = 0;
int non_empty_cells_in_col_A3 = 0; // Compteur de cellules non vides en colonne A
char smsgg_text_values[1500][TEXT_SIZE3+ 1];
int smsgg_text_values_count = 0;
unsigned int HeaderCRC=0;
unsigned char *SMS_Header = NULL;
unsigned char gg_mode=1; // 1 for GG 0 for SMS

// Tableau pour stocker les différentes valeurs du fichier csv sms-gg-crc

char txt_csv_chksm1[8];
unsigned long csv_chksm1=0;
unsigned char txt_csv_chksm2[8];
unsigned long csv_chksm2=0;
unsigned char txt_csv_game_size2[4+1];
unsigned char csv_game_size2=0;
unsigned char txt_csv_game_type2[3+1];
unsigned char csv_game_type2=0;
unsigned char txt_csv_game_name2[48+1];
unsigned char txt_csv_region2[20+1];

//CSV Flashlist specific Variables

#define CHIPID_TEXT_SIZE 48 // taille de toute la chaine
#define TEXT_SIZE2 48

int current_row2 = 0;
int current_col2 = 0;
int non_empty_cells_in_col_A2 = 0; // Compteur de cellules non vides en colonne A
char chipid_text_values[MAX_NON_EMPTY_CELLS][CHIPID_TEXT_SIZE + 1];
int chipid_text_values_count = 0;

char txt_csv_deviceID[4];
unsigned short csv_deviceID=0;
unsigned char txt_csv_flash_size[3+1];
unsigned char csv_flash_size=0;
unsigned long flash_size=0;
unsigned char txt_csv_erase_algo[2+1];
unsigned char csv_erase_algo=0;
unsigned char txt_csv_write_algo[2+1];
unsigned char csv_write_algo=0;
unsigned char txt_csv_flash_name[11+1];
unsigned char txt_csv_man_name[18+1];
unsigned char txt_csv_voltage[2+1];
unsigned char csv_voltage=0;
unsigned char flash_algo=0;

FILE *fp2;
struct csv_parser p2;

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
unsigned long j=0;
unsigned long k=0;
unsigned char *buffer_header = NULL;
unsigned char *buffer_rom = NULL;
unsigned char md_dumper_type=0;
unsigned char sms_mode=0;
unsigned char dump_name[32];
unsigned char region[5];
char *game_region = NULL;
const char unk[] = {"unknown"};
int checksum_header = 0;
int use_gui=0;							/* 0=CLI Mode, 1=GUI Mode */

int gui_tab_mode=0;						/* 0=Read Mode, 1=Write Mode */

int data_type=0;						/* 0=ROM, 1=RAM */
int dump_mode=0; 						/* 0=Auto, 1=Manual, 2=Bankswitch */
int dump_rom_size_opts=0; 				/* 0=32KB, 1=64KB, 2=128KB, 3=256KB, 4=512KB, 5=1024KB, 6=2048KB, 7=4096KB, 8=8192KB */
int dump_cart_mode_opts=0; 				/* 0=GG MODE, 1=MD MODE, 2=SMS MODE */
int dump_sram_size_opts=0; 				/* 0=8192, 1=32768 */
int dump_sram_type_opts=0; 				/* 0=parallel_sram, 1=serial_i2c, 2=serial_spi */
int erase_or_write=0;					/* 0=erase, 1=write */

int game_size=0;
unsigned long save_size1 = 0;
unsigned long save_size2 = 0;
unsigned long save_size = 0;
unsigned long save_address = 0;
unsigned char *BufferROM;
unsigned char *BufferSAVE;
char empty_flash[512];
char dump_flash[512];
FILE *myfile;
unsigned char NumberOfBank=0;
unsigned char ActualBank=0;
unsigned long offset=0;
unsigned short rom_id=0;
unsigned short flash_id=0;
unsigned char chip_id=0;
unsigned char manufacturer_id=0;
const char * wheel[] = { "-","\\","|","/"}; //erase wheel
int lockon_mode=0;

//Others Functions
void Display_Help(char *prog_name)
{
    SDL_Log("\n");
    SDL_Log("How to use the program:\n");
    SDL_Log("\n");
    SDL_Log("GUI Mode:\n");
    SDL_Log("  %s -gui|-gui_fr \n", prog_name);
    SDL_Log("\n");
    SDL_Log("CLI Mode:\n");
    SDL_Log("\n");
    SDL_Log("  %s -read auto  -  Auto Mode\n", prog_name);
    SDL_Log("  %s -read bankswitch  -  Bankswitch Mode\n", prog_name);
    SDL_Log("  %s -read manual (32|64|128|256|512|1024|2048|4096|8192) (gg|md|sms) -  Manual Mode\n", prog_name);
    SDL_Log("  %s -backup auto  -  Auto Mode\n", prog_name);
    SDL_Log("  %s -backup bankswitch  -  Bankswitch Mode\n", prog_name);
    SDL_Log("  %s -backup manual (8192|32768) (serial_spi|serial_i2c|parallel_sram) -  Manual Mode\n", prog_name);
    SDL_Log("\n");
    SDL_Log("  %s -erase_flash -  Erase Flash Data\n", prog_name);
    SDL_Log("  %s -write_flash -  Write Flash Data\n", prog_name);
    SDL_Log("  %s -erase_memory (serial_spi|serial_i2c|parallel_sram) -  Erase Save/Memory Data\n", prog_name);
    SDL_Log("  %s -restore_memory (serial_spi|serial_i2c|parallel_sram) - Write Save/Memory Data\n", prog_name);
    SDL_Log("\n");
}

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

void cb3(void *s, size_t len, void *data)
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
    if (current_row2 > 0 && current_col2 == col_A && len > 0)   // Ignorer la première ligne
    {
        non_empty_cells_in_col_A2++;
        if (chipid_text_values_count < MAX_NON_EMPTY_CELLS)
        {
            // Assurer que la chaîne est de taille TEXT_SIZE
            strncpy(chipid_text_values[chipid_text_values_count], (char *)s, TEXT_SIZE2);
            chipid_text_values[chipid_text_values_count][TEXT_SIZE2] = '\0'; // Ajouter le caractère de fin de chaîne
            chipid_text_values_count++;
        }
    }
}


// Fonction de rappel pour traiter la fin de chaque ligne
void cb4(int c, void *data)
{
    current_row2++;
    current_col2 = 0; // Réinitialiser le compteur de colonnes à la fin de chaque ligne
}

// SMS-GG

void cb5(void *s, size_t len, void *data)
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
    if (current_row3 > 0 && current_col3 == col_A && len > 0)   // Ignorer la première ligne
    {
        non_empty_cells_in_col_A3++;
        if (smsgg_text_values_count < MAX_NON_EMPTY_CELLS2)
        {
            // Assurer que la chaîne est de taille TEXT_SIZE
            strncpy(smsgg_text_values[smsgg_text_values_count], (char *)s, TEXT_SIZE3);
            smsgg_text_values[smsgg_text_values_count][TEXT_SIZE3] = '\0'; // Ajouter le caractère de fin de chaîne
            smsgg_text_values_count++;
        }
    }
}


// Fonction de rappel pour traiter la fin de chaque ligne
void cb6(int c, void *data)
{
    current_row3++;
    current_col3 = 0; // Réinitialiser le compteur de colonnes à la fin de chaque ligne
}


unsigned int crc32(unsigned int seed, const void* data, int data_size)
{
    unsigned int crc = ~seed;
    static unsigned int crc32_lut[256] = { 0 };
    if(!crc32_lut[1])
    {
        const unsigned int polynomial = 0xEDB88320;
        unsigned int i, j;
        for (i = 0; i < 256; i++)
        {
            unsigned int crc = i;
            for (j = 0; j < 8; j++)
                crc = (crc >> 1) ^ ((unsigned int)(-(int)(crc & 1)) & polynomial);
            crc32_lut[i] = crc;
        }
    }

    {
        const unsigned char* data8 = (const unsigned char*)data;
        int n;
        for (n = 0; n < data_size; n++)
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ data8[n]];
    }
    return ~crc;
}


//Timer functions according to Operating Systems
#if defined(_WIN32)		//Windows
clock_t microsec_start;
clock_t microsec_end;

void timer_start()
{
    microsec_start = clock();
}
void timer_end()
{
    microsec_end = clock();
}
void timer_show()
{
    SDL_Log("~ Elapsed time: %lds", (microsec_end - microsec_start)/1000);
    SDL_Log(" (%ldms)\n", (microsec_end - microsec_start));
}
#else 				//Others
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
    SDL_Log("~ Elapsed time: %lds", (microsec_end - microsec_start)/1000000);
    SDL_Log(" (%ldms)\n", (microsec_end - microsec_start)/1000);
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


int Detect_Device(void)
{
    /* Initialise libusb. */
    SDL_Log("Init LibUSB... \n");
    res = libusb_init(0);
    if (res != 0)
    {
        SDL_Log("Error initialising libusb.\n");
        return 1;
    }

    SDL_Log("LibUSB Init Sucessfully ! \n");

    SDL_Log("Detecting MD Dumper... \n");

    /* Get the first device with the matching Vendor ID and Product ID. If
     * intending to allow multiple demo boards to be connected at once, you
     * will need to use libusb_get_device_list() instead. Refer to the libusb
     * documentation for details. */

    handle = libusb_open_device_with_vid_pid(0, 0x0483, 0x5740);

    if (!handle)
    {
        SDL_Log("Unable to open device.\n");
        return 1;
    }

    /* Claim interface #0. */

    if(libusb_kernel_driver_active(handle, 0) == 1)
    {
        SDL_Log("Kernel Driver Active");
        if(libusb_detach_kernel_driver(handle, 0) == 0)
            SDL_Log("Kernel Driver Detached!");
        else
        {
            SDL_Log("Couldn't detach kernel driver!\n");
            libusb_close(handle);
        }
    }
    res = libusb_claim_interface(handle, 0);
    //SDL_Log("Test 1 : %d\n",res);
    if (res != 0)
    {
        if(libusb_kernel_driver_active(handle, 1) == 1)
        {
            SDL_Log("Kernel Driver Active");
            if(libusb_detach_kernel_driver(handle, 1) == 0)
                SDL_Log("Kernel Driver Detached!");
            else
            {
                SDL_Log("Couldn't detach kernel driver!\n");
                libusb_close(handle);
            }
        }
        res = libusb_claim_interface(handle, 1);
        //SDL_Log("Test 2 : %d\n",res);
        if (res != 0)
        {
            SDL_Log("Error claiming interface.\n");
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
    for (i = 0; i < 512; i++)
    {
        empty_flash[i]=0xFF;
        dump_flash[i]=0xFF;
    }

    usb_buffer_out[0] = WAKEUP;// Affect request to  WakeUP Command
    libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 0); // Send Packets to Sega Dumper
    libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 0);
    SDL_Log("\n");
    SDL_Log("MD Dumper %.*s",6, (char *)usb_buffer_in);
    SDL_Log("\n");
    md_dumper_type = usb_buffer_in[24];
    if ( md_dumper_type == 0 )
    {
        SDL_Log("MD Dumper type : Old Version \n");
    }
    if ( md_dumper_type == 1 )
    {
        SDL_Log("MD Dumper type : BluePill non aligned \n");
    }
    if ( md_dumper_type == 2 )
    {
        SDL_Log("MD Dumper type : BluePill aligned \n");
    }
    if ( md_dumper_type == 3 )
    {
        SDL_Log("MD Dumper type : SMD ARM TQFP100 Aligned  \n");
    }
    if ( md_dumper_type == 4 )
    {
        SDL_Log("MD Dumper type : Marv17 aligned tqfp48  \n");
    }
    SDL_Log("Hardware Firmware version : %d.%d\n", usb_buffer_in[20],usb_buffer_in[21]);

    sms_mode = usb_buffer_in[25];
    if ( sms_mode == 0 )
    {
        SDL_Log("Dumper started in 16 bit mode \n");
        //Hardwaretype = 0 ;
    }
    if ( sms_mode == 1 )
    {
        SDL_Log("Dumper started in 8 bit mode \n");
    }
    SDL_Log("\n");


    return 0;
}

int Open_CSV_Files(void)
{
    if (csv_init(&p, options) != 0)
    {
        SDL_Log("\n");
        SDL_Log("\n");
        SDL_Log("ERROR Failed to init CSV Parser for Gamelist ...\n");
        exit(EXIT_FAILURE);
    }
    csv_set_quote(&p,';');

    FILE *fp = fopen("gameslist.csv", "r");
    if (!fp)
    {
        SDL_Log("\n");
        SDL_Log("\n");
        SDL_Log("ERROR Can't find gamelist.csv ...\n");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
        if (csv_parse(&p, buffer, bytes_read, cb1, cb2, NULL) != bytes_read)
        {
            buffer_header[i]=0x00;
            SDL_Log("\n");
            SDL_Log("\n");
            SDL_Log("ERROR while parsing file ...\n");
            return EXIT_FAILURE;
        }
    }
    i = 0;

    csv_fini(&p, cb1, cb2, NULL);
    csv_free(&p);
    fclose(fp);

    SDL_Log("\n");
    SDL_Log("CSV Gamelist file opened sucessfully\n");
    //Afficher le nombre de cellules non vides en colonne A
    SDL_Log("Add : %d Special Games into MD Dumper Database \n", non_empty_cells_in_col_A);

    // open csv flash list File

    if (csv_init(&p2, options) != 0)
    {
        SDL_Log("\n");
        SDL_Log("\n");
        SDL_Log(" ERROR Failed to init CSV Parser for Flashlist ...\n");
        exit(EXIT_FAILURE);
    }
    csv_set_quote(&p2,';');


    FILE *fp2 = fopen("flashlist.csv", "r");
    if (!fp)
    {
        SDL_Log("\n");
        SDL_Log("\n");
        SDL_Log(" ERROR Can't find flashlist.csv ...\n");
        return EXIT_FAILURE;
    }

    char buffer2[BUFFER_SIZE];
    size_t bytes_read2;
    while ((bytes_read2 = fread(buffer2, 1, BUFFER_SIZE, fp)) > 0)
    {
        if (csv_parse(&p2, buffer2, bytes_read2, cb3, cb4, NULL) != bytes_read2)
        {
            SDL_Log("\n");
            SDL_Log("\n");
            SDL_Log(" ERROR while parsing file ...\n");
            return EXIT_FAILURE;
        }
    }

    csv_fini(&p, cb1, cb2, NULL);
    csv_free(&p);
    fclose(fp);

    SDL_Log("CSV Flashlist file opened sucessfully\n");
    // Afficher le nombre de cellules non vides en colonne A
    SDL_Log("Add : %d Flash ID into MD Dumper Database \n", non_empty_cells_in_col_A2);

    // open csv sms-gg crc

    if (csv_init(&p3, options) != 0)
    {
        SDL_Log("\n\n ERROR Failed to init CSV Parser for SMS-GG crc ...\n");
        exit(EXIT_FAILURE);
    }
    csv_set_quote(&p3,';');


    FILE *fp3 = fopen("sms-gg_crc.csv", "r");
    if (!fp)
    {
        SDL_Log("\n\n ERROR Can't find flashlist.csv ...\n");
        return EXIT_FAILURE;
    }

    char buffer3[BUFFER_SIZE3];
    size_t bytes_read3;
    while ((bytes_read3 = fread(buffer3, 1, BUFFER_SIZE3, fp)) > 0)
    {
        if (csv_parse(&p3, buffer3, bytes_read3, cb5, cb6, NULL) != bytes_read3)
        {
            SDL_Log("\n\n ERROR while parsing file ...\n");
            return EXIT_FAILURE;
        }
    }

    csv_fini(&p, cb5, cb6, NULL);
    csv_free(&p);
    fclose(fp);

    SDL_Log("CSV SMS-GG CRC file opened sucessfully\n");
// Afficher le nombre de cellules non vides en colonne A
    SDL_Log("Add : %ld SMS/GG Games into MD Dumper Database \n", non_empty_cells_in_col_A3);

    return 0;
}

void Game_Header_Infos(void)
{

    if ( sms_mode == 0 ) //Read in 16 bits mode
    {


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
            SDL_Log("\n");
            SDL_Log("Megadrive/Genesis/32X cartridge detected!\n");
            SDL_Log("\n");
            SDL_Log(" --- HEADER ---\n");
            memcpy((unsigned char *)dump_name, (unsigned char *)buffer_header+32, 48);
            trim((unsigned char *)dump_name, 0);
            SDL_Log(" Domestic: %.*s\n", 48, (char *)game_name);
            memcpy((unsigned char *)dump_name, (unsigned char *)buffer_header+80, 48);
            trim((unsigned char *)dump_name, 0);

            if(memcmp((unsigned char *)game_name,"SONIC & KNUCKLES",16) == 0)
                lockon_mode=1;

            SDL_Log(" International: %.*s\n", 48, game_name);
            SDL_Log(" Release date: %.*s\n", 16, buffer_header+0x10);
            SDL_Log(" Version: %.*s\n", 14, buffer_header+0x80);
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

            SDL_Log(" Region: %s\n", game_region);

            checksum_header = (buffer_header[0x8E]<<8) | buffer_header[0x8F];
            SDL_Log(" Checksum: %X\n", checksum_header);

            game_size = 1 + ((buffer_header[0xA4]<<24) | (buffer_header[0xA5]<<16) | (buffer_header[0xA6]<<8) | buffer_header[0xA7])/1024;
            SDL_Log(" Game size: %dKB\n", game_size);

            if((buffer_header[0xB0] + buffer_header[0xB1])!=0x93)
            {
                SDL_Log(" Extra Memory : No\n");
            }
            else
            {
                SDL_Log(" Extra Memory : Yes ");
                switch(buffer_header[0xB2])
                {
                case 0xF0:
                    SDL_Log(" 8bit backup SRAM (even addressing)\n");
                    break;
                case 0xF8:
                    SDL_Log(" 8bit backup SRAM (odd addressing)\n");
                    break;
                case 0xB8:
                    SDL_Log(" 8bit volatile SRAM (odd addressing)\n");
                    break;
                case 0xB0:
                    SDL_Log(" 8bit volatile SRAM (even addressing)\n");
                    break;
                case 0xE0:
                    SDL_Log(" 16bit backup SRAM\n");
                    break;
                case 0xA0:
                    SDL_Log(" 16bit volatile SRAM\n");
                    break;
                case 0xE8:
                    SDL_Log(" Serial EEPROM\n");
                    break;
                }
                if ( buffer_header[0xB2] != 0xE0 | buffer_header[0xB2] != 0xA0 ) // 8 bit SRAM
                {
                    save_size2 = (buffer_header[0xB8]<<24) | (buffer_header[0xB9]<<16) | (buffer_header[0xBA] << 8) | buffer_header[0xBB];
                    save_size1 = (buffer_header[0xB4]<<24) | (buffer_header[0xB5]<<16) | (buffer_header[0xB6] << 8) | buffer_header[0xB7];

                    save_size = save_size2 - save_size1;
                    save_size = (save_size/1024); // Kb format
                    save_size=(save_size/2) + 1; // 8bit size
                }
                save_address = (buffer_header[0xB4]<<24) | (buffer_header[0xB5]<<16) | (buffer_header[0xB6] << 8) | buffer_header[0xB7];
                SDL_Log(" Save size: %dKb\n", save_size);
                SDL_Log(" Save address: %lX\n", save_address);

                if(usb_buffer_in[0xB2]==0xE8) // EEPROM Game
                {
                    SDL_Log(" No information on this game!\n");
                }
            }
        }
    }
    if ( sms_mode == 1 ) //Read in 8 bits mode
    {

        buffer_header = (unsigned char *)malloc(0x200);
        i = 0;
        // Cleaning header Buffer
        for (i=0; i<512; i++)
        {
            buffer_header[i]=0x00;
        }
        i = 0;
		SDL_Log("\nTry to read SMS - GG cartridge...\n");
        address = 0x7FF0;

        i=0;
        while (i<8)
        {

            usb_buffer_out[0] = READ_SMS;
            usb_buffer_out[1] = address&0xFF ;
            usb_buffer_out[2] = (address&0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4] = 0; // Slow Mode

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82,buffer_header+(64*i),64, &numBytes, 60000);
            address+=64;
            i++;
        }

        if(memcmp((unsigned char *)buffer_header,"TMR SEGA",8) == 0)
        {
			SDL_Log("Valid cartridge detected !\n\n");

            for(i=0; i<(256/16); i++)
            {
				SDL_Log("\n");
				SDL_Log("%03lX", 0x100+(i*16));
                for(j=0; j<16; j++)
                {
				   SDL_Log(" %02X", buffer_header[j+(i*16)]);
                }
				SDL_Log(" %.*s", 16, buffer_header +(i*16));
            }
            i=0;

            // Calculate Checksum of first bank

            //printf("\nRead First BANK...\n");
            // Read first 32 Ko of the ROM

            i=0;
            SMS_Header = (unsigned char *)malloc(32*1024);

            // Cleaning Bank Buffer
            for (i=0; i<32*1024; i++)
            {
                SMS_Header[i]=0x00;
            }

            i=0;
            address=0;
            while (i<32*1024)
            {

                usb_buffer_out[0] = READ_SMS;
                usb_buffer_out[1] = address&0xFF ;
                usb_buffer_out[2] = (address&0xFF00)>>8;
                usb_buffer_out[3]=(address & 0xFF0000)>>16;
                usb_buffer_out[4] = 0; // Slow Mode

                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                libusb_bulk_transfer(handle, 0x82,SMS_Header+i,64, &numBytes, 60000);
                address+=64;
                i=i+64;
            }
            //printf("Calculate CRC..\n");
            HeaderCRC = crc32(0,SMS_Header,32*1024);
            //  printf("BANK0 CRC value is : 0x%08X\n",HeaderCRC);

            // Search valid CRC in SMS-GG CRC csv table

            // Search checksum cartridge in Custom Hardware games csv table
            i=0;
            for (i = 0; i < non_empty_cells_in_col_A3 ; i++)
            {
                strncpy(txt_csv_chksm1,smsgg_text_values[i],8);
                //printf(" \n txt chksm value : %s \n",txt_csv_chksm1);
                csv_chksm1 = (unsigned long)strtoul(txt_csv_chksm1, NULL, 16);
                //printf("CRC value is : 0x%08X\n",csv_chksm1 );

                if ( csv_chksm1 == HeaderCRC )
                {

                    strncpy(txt_csv_game_name2,smsgg_text_values[i]+27,48);
                    txt_csv_game_name2[48] = '\0'; // Null-terminate the output string

                    // Copy Cartridge Type :

                    strncpy(txt_csv_game_type2,smsgg_text_values[i]+23,3);
                    txt_csv_game_type2[3] = '\0'; // Null-terminate the output string

                    // Copy Cartridge Size :

                    strncpy(txt_csv_game_size2,smsgg_text_values[i]+18,4);

                    // Copy Cartridge Region :

                    strncpy(txt_csv_region2,smsgg_text_values[i]+76,20);
                    txt_csv_region2[20] = '\0'; // Null-terminate the output string
                }
            }


            SDL_Log("\n\n --- HEADER --- \n");

            SDL_Log("Game Name: %.*s\n",48, (char *)txt_csv_game_name2);
            if(memcmp((unsigned char *)txt_csv_game_type2,"GG ",3) == 0)
            {
                SDL_Log("Game Type : GAME GEAR \n");
            }
            if(memcmp((unsigned char *)txt_csv_game_type2,"SMS",3) == 0)
            {
                SDL_Log("Game Type : MASTER SYSTEM / MARK3\n");
                gg_mode=0;
            }

            game_size = buffer_header[15] & 0xF;
            if (game_size == 0x00)
            {
                SDL_Log("Game Size (Header) : 256 Ko");
                game_size = 256*1024;
            }
            if (game_size == 0x01)
            {
                SDL_Log("Game Size (Header) : 512 Ko");
                game_size = 512*1024;
            }
            if (game_size == 0x0c)
            {
                SDL_Log("Game Size (Header) : 32 Ko");
                game_size = 32*1024;
            }
            if (game_size == 0x0e)
            {
                SDL_Log("Game Size (Header) : 64 Ko");
                game_size = 64*1024;
            }
            if (game_size == 0x0f)
            {
                SDL_Log("Game Size (Header) : 128 Ko");
                game_size = 128*1024;
            }

            // Real Cartridge Size

            SDL_Log("\nGame Size (Real): %.*s Ko\n",4, (char *)txt_csv_game_size2);

            // Region Header

            if ( buffer_header[15] >> 6 == 0x01 )
            {
                SDL_Log("Game Region (Header) : Export");
            }
            if ( buffer_header[15] >> 4 == 0x03 )
            {
                SDL_Log("Game Region (Header) : Japan");
            }

            // Region REAL

            SDL_Log("\nGame Region (Real) : %.*s\n",40, (char *)txt_csv_region2);


        }

    }
}
