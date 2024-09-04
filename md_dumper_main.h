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
        SDL_Log("MD Dumper type : Marv17 aligned tqfp44  \n");
    }
    SDL_Log("Hardware Firmware version : %d.%d\n", usb_buffer_in[20],usb_buffer_in[21]);
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
        SDL_Log("\n\n ERROR Failed to init CSV Parser for Flashlist ...\n");
        exit(EXIT_FAILURE);
    }
    csv_set_quote(&p2,';');


    FILE *fp2 = fopen("flashlist.csv", "r");
    if (!fp)
    {
        SDL_Log("\n\n ERROR Can't find flashlist.csv ...\n");
        return EXIT_FAILURE;
    }

    char buffer2[BUFFER_SIZE];
    size_t bytes_read2;
    while ((bytes_read2 = fread(buffer2, 1, BUFFER_SIZE, fp)) > 0)
    {
        if (csv_parse(&p2, buffer2, bytes_read2, cb3, cb4, NULL) != bytes_read2)
        {
            SDL_Log("\n\n ERROR while parsing file ...\n");
            return EXIT_FAILURE;
        }
    }

    csv_fini(&p, cb1, cb2, NULL);
    csv_free(&p);
    fclose(fp);

    SDL_Log("CSV Flashlist file opened sucessfully\n");
    // Afficher le nombre de cellules non vides en colonne A
    SDL_Log("Add : %d Flash ID into MD Dumper Database \n", non_empty_cells_in_col_A2);
    return 0;
}

void Game_Header_Infos(void)
{
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
        SDL_Log("\n");
        SDL_Log("Megadrive/Genesis/32X cartridge detected!\n");
        SDL_Log("\n");
        SDL_Log(" --- HEADER ---\n");
        memcpy((unsigned char *)dump_name, (unsigned char *)buffer_header+32, 48);
        trim((unsigned char *)dump_name, 0);
        SDL_Log(" Domestic: %.*s\n", 48, (char *)game_name);
        memcpy((unsigned char *)dump_name, (unsigned char *)buffer_header+80, 48);
        trim((unsigned char *)dump_name, 0);

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
