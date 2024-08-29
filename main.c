/*
MD Dumper CLI Version
X-death 07/2024

-TODO-
-Read mode automatic / manual


*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb.h>


// USB Special Command

#define WAKEUP          0x10  // WakeUP for first STM32 Communication
#define READ_MD         0x11

// Version

#define MAX_VERSION 	2
#define MIN_VERSION 	1


char * game_rom = NULL;
char * game_name = NULL;


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


#if defined(_WIN32)
clock_t microsec_start;
clock_t microsec_end;
#else
struct timeval ostime;
long microsec_start = 0;
long microsec_end = 0;
#endif

void timer_start()
{
#if defined(_WIN32)
    microsec_start = clock();
#else
    gettimeofday(&ostime, NULL);
    microsec_start = ((unsigned long long)ostime.tv_sec * 1000000) + ostime.tv_usec;
#endif
}

void timer_end()
{
#if defined(_WIN32)
    microsec_end = clock();
#else
    gettimeofday(&ostime, NULL);
    microsec_end = ((unsigned long long)ostime.tv_sec * 1000000) + ostime.tv_usec;
#endif
}

void timer_show()
{
#if defined(_WIN32)
    printf("~ Elapsed time: %lds", (microsec_end - microsec_start)/1000);
    printf(" (%ldms)\n", (microsec_end - microsec_start));
#else
    printf("~ Elapsed time: %lds", (microsec_end - microsec_start)/1000000);
    printf(" (%ldms)\n", (microsec_end - microsec_start)/1000);
#endif
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
	int game_size=0;
	unsigned char *BufferROM;
	FILE *myfile;


	// Affiche le titre du programme
printf("----------------------------------------------------------------------------\n");
printf("8b   d8 888b.      888b. 8    8 8b   d8 888b. 8888 888b.      .d88b 8    888\n"); 
printf("8YbmdP8 8   8      8   8 8    8 8YbmdP8 8  .8 8www 8  .8      8P    8     8\n");  
printf("8     8 8   8 wwww 8   8 8b..d8 8     8 8wwP' 8    8wwK'      8b    8     8\n");  
printf("8     8 888P'      888P' `Y88P' 8     8 8     8888 8  Yb wwww `Y88P 8888 888\n");
printf("----------------------------------------------------------------------------\n");
printf("\nRelease : 1.0 alpha \n\n");


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
    if (argc != 3) {
        printf("Usage: %s -read <mode>\n", argv[0]);
        printf("Mode:\n");
        printf("  a - Auto\n");
        printf("  m - Manual\n");
        return 1;
    }

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

    // Afficher le mode de lecture
    if (strcmp(argv[2], "a") == 0) 
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
		


    } else {
        printf("Mode de lecture: Manuel\n");
    }

    return 0;
}


