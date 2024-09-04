int Erase_Flash()
{
        if ( md_dumper_type == 0 ) // Keep the old detection code for oldest MD Dumper exemple STM32F4 / Blue Pill
        {
            SDL_Log("Write Mode : Erase Flash Data\n");
            SDL_Log("Launch Flash Erase command ... \n");
            SDL_Log("Detecting Flash Memory... \n");
            usb_buffer_out[0] = INFOS_ID;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
            manufacturer_id = usb_buffer_in[1];
            chip_id = usb_buffer_in[3];
            flash_id = (manufacturer_id<<8) | chip_id;
            SDL_Log("Flash ID : %04X \n",flash_id);

            switch(flash_id)
            {
            case 0xBFB6 :
                SDL_Log("SST Flash use algo number 1 \n");
                usb_buffer_out[1] = 1;
                break;
            case 0xBFB7 :
                SDL_Log("SST Flash use algo number 1 \n");
                usb_buffer_out[1] = 1;
                break;
            case 0xC2CB :
                SDL_Log("Macronix Flash use algo number 2 \n");
                usb_buffer_out[1] = 2;
                break;
            case 0x20ED :
                SDL_Log("STMicroelectronics Flash use algo number 2 \n");
                usb_buffer_out[1] = 2;
                break;
            default :
                SDL_Log("Flash use algo number 1 \n");
                usb_buffer_out[1] = 1;
                break;
            }

            usb_buffer_out[0] = ERASE_MD_FLASH;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            i=0;
            while(usb_buffer_in[0]!=0xFF)
            {
                SDL_Log("ERASE SMD flash in progress: %s ", wheel[i]);
                libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                fflush(stdout);
                i++;
                if(i==4)
                {
                    i=0;
                }
            }
            SDL_Log("\n");
            SDL_Log("\nFlash Erased sucessfully !");
            fflush(stdout);
        }

        else  // Erase Flash code for new STM32 board
        {
            SDL_Log("Execute Erase code V2\n");
            SDL_Log("Write Mode : Erase Flash Data\n");
            SDL_Log("Launch Flash Erase command ... \n");
            SDL_Log("Detecting Flash Memory... \n");
            address=0;
            usb_buffer_out[0] = READ_MD;
            usb_buffer_out[1] = address&0xFF ;
            usb_buffer_out[2] = (address&0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4] = 0; // Slow Mode

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82,usb_buffer_in,64, &numBytes, 60000);

            //printf("Flash data at address 0 : 0x%02X \n",usb_buffer_in[0]);
            //printf("Flash data at address 1 : 0x%02X \n",usb_buffer_in[1]);
            rom_id = (usb_buffer_in[1]<<8) | usb_buffer_in[0];

            usb_buffer_out[0] = INFOS_ID;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);

            manufacturer_id = usb_buffer_in[1];
            chip_id = usb_buffer_in[3];
            flash_id = (manufacturer_id<<8) | chip_id;

            SDL_Log("Flash ID : %04X \n",flash_id);

            for (i = 0; i < chipid_text_values_count; i++)
            {
                strncpy(txt_csv_deviceID,chipid_text_values[i],4);
                //SDL_Log(" \n txt chipid value : %s \n",txt_csv_deviceID);
                csv_deviceID = (unsigned short)strtol(txt_csv_deviceID, NULL, 16);
                //SDL_Log(" \n DEC Device ID value : %ld \n",csv_deviceID);

                // If found we need to copy all usefull info from CSV to MD dumper Var
                if ( flash_id == csv_deviceID  )
                {
                    Index_chksm = i;
                    SDL_Log("Found chip in CSV Flashlist ! \n");
                    SDL_Log("Position in csv table %d \n",i);

                    // Flash Size
                    strncpy(txt_csv_flash_size,chipid_text_values[i]+5,3);
                    txt_csv_flash_size[4] = '\0'; // Null-terminate the output string
                    //printf("Txt flash size : %s \n",txt_csv_flash_size);
                    csv_flash_size = (unsigned char)strtol(txt_csv_flash_size, NULL, 10);
                    //printf("CSV Flash Size  %d \n",csv_flash_size);
                    flash_size=1024*csv_flash_size;
                    //printf("La valeur de FlashSize est %ld Ko \n",flash_size);

                    // Algo Erase
                    strncpy(txt_csv_erase_algo,chipid_text_values[i]+9,2);
                    txt_csv_erase_algo[2] = '\0'; // Null-terminate the output string
                    //printf("Txt Erase Algo : %s \n",txt_csv_erase_algo);
                    csv_erase_algo = (unsigned char)strtol(txt_csv_erase_algo, NULL, 8);
                    //printf("CSV Erase Algo  %d \n",csv_erase_algo);

                    // Write Algo
                    strncpy(txt_csv_write_algo,chipid_text_values[i]+12,2);
                    txt_csv_write_algo[2] = '\0'; // Null-terminate the output string
                    //printf("Txt Write Algo  : %s \n",txt_csv_write_algo);
                    csv_write_algo = (unsigned char)strtol(txt_csv_write_algo, NULL, 8);
                    //printf("CSV Write Algo %d \n",csv_write_algo);

                    // Chip Name
                    strncpy(txt_csv_flash_name,chipid_text_values[i]+15,11);
                    txt_csv_flash_name[12] = '\0'; // Null-terminate the output string
                    //printf("Flash Device Reference : %s \n",txt_csv_flash_name);

                    // Voltage

                    strncpy(txt_csv_voltage,chipid_text_values[i]+27,2);
                    txt_csv_voltage[2] = '\0'; // Null-terminate the output string
                    //printf("Txt Chip Voltage : %s \n",txt_csv_voltage);
                    csv_voltage = (unsigned char)strtol(txt_csv_voltage, NULL, 8);
                    //printf("CSV Chip Voltage  %d \n",csv_voltage);

                    // Manufacturer

                    strncpy(txt_csv_man_name,chipid_text_values[i]+30,18);
                    txt_csv_man_name[19] = '\0'; // Null-terminate the output string
                    //printf("Chip Manufacturer : %s \n",txt_csv_man_name);

                    SDL_Log("Memory : %s \n",txt_csv_flash_name);
                    SDL_Log("Capacity %ld Ko \n",flash_size);
                    SDL_Log("Chip Manufacturer : %s \n",txt_csv_man_name);
                    SDL_Log("Chip Voltage %ld V \n",csv_voltage);
                    SDL_Log("CSV Erase Algo  %d \n",csv_erase_algo);
                    SDL_Log("CSV Write Algo %d \n",csv_write_algo);

                }
            }
            if  ( flash_id == 0x9090 )
            {
                SDL_Log("No compatible Flash detected ! \n");
            }

            if  ( flash_id == 0x0000 )
            {
                SDL_Log("No compatible Flash detected ! \n");
            }

            usb_buffer_out[0] = ERASE_MD_FLASH;
            usb_buffer_out[1] = csv_erase_algo;
            SDL_Log("Memory : %s \n",txt_csv_flash_name);
            SDL_Log("Erase flash with algo %d \n ",csv_erase_algo);
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            i=0;
            while(usb_buffer_in[0]!=0xFF)
            {
                libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                SDL_Log("\rERASE SMD flash in progress: %s ", wheel[i]);
                fflush(stdout);
                i++;
                if(i==4)
                {
                    i=0;
                }
            }
        }
        return 0;
}

int Write_Flash(void)
{
SDL_Log("Write Mode : Write Flash Data\n");
        sfd_Options opt =
        {
            .title        = "Select your Flash Data File",
            .filter_name  = "Flash Data",
            .filter       = "*.*",
        };
        const char *filename = sfd_open_dialog(&opt);
        if (filename)
        {
            SDL_Log("You selected the file: '%s'\n", filename);
        }
        else
        {
            SDL_Log("Operation canceled\n");
            return 0;
        }

        address=0;
        i=0;

        if ( md_dumper_type == 0 ) // Keep the old detection code for oldest MD Dumper exemple STM32F4 / Blue Pill
        {

            while (i<8)
            {
                usb_buffer_out[0] = READ_MD;
                usb_buffer_out[1] = address&0xFF ;
                usb_buffer_out[2] = (address&0xFF00)>>8;
                usb_buffer_out[3]=(address & 0xFF0000)>>16;
                usb_buffer_out[4] = 0; // Slow Mode

                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                libusb_bulk_transfer(handle, 0x82,dump_flash+(64*i),64, &numBytes, 60000);
                address+=32;
                i++;
            }

            SDL_Log("Detecting Flash Memory ID... \n");
            usb_buffer_out[0] = INFOS_ID;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
            manufacturer_id = usb_buffer_in[1];
            chip_id = usb_buffer_in[3];
            flash_id = (manufacturer_id<<8) | chip_id;
            SDL_Log("Flash ID : %04X \n",flash_id);

            switch(flash_id)
            {
            case 0xBFB6 :
                SDL_Log(" SST Flash use algo number 1 \n");
                usb_buffer_out[1] = 1;
                flash_algo = 1;
                break;
            case 0xBFB7 :
                SDL_Log("SST Flash use algo number 2 \n");
                usb_buffer_out[1] = 2;
                flash_algo = 1;
                break;
            case 0x20ED :
                SDL_Log("STMicroelectronics Flash use algo number 2 \n");
                usb_buffer_out[1] = 2;
                flash_algo = 2;
                break;
            case 0xC2CB :
                SDL_Log("Macronix Flash use algo number 2 \n");
                usb_buffer_out[1] = 2;
                flash_algo = 2;
                break;
            default :
                SDL_Log("Flash use algo number 2 \n");
                usb_buffer_out[1] = 2;
                flash_algo = 2;
                break;

                SDL_Log("Detect if Flash is empty... \n");
                if(memcmp((char *)dump_flash,(char *)empty_flash,512) == 0)
                {
                    SDL_Log("Flash is empty !\n");
                }
                else
                {
                    SDL_Log("Flash Memory is not empty \n");

                    SDL_Log("Erasing flash with algo %d \n ",flash_algo);
                    usb_buffer_out[0] = ERASE_MD_FLASH;
                    libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                    i=0;
                    SDL_Log("ERASE SMD flash in progress...");
                    while(usb_buffer_in[0]!=0xFF)
                    {
                        libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                        SDL_Log("ERASE SMD flash in progress: %s ", wheel[i]);
                        fflush(stdout);
                        i++;
                        if(i==4)
                        {
                            i=0;
                        }
                    }
                    SDL_Log("Flash Erased sucessfully !\n");
                    fflush(stdout);
                }
            }
            myfile = fopen(filename,"rb");
            fseek(myfile,0,SEEK_END);
            game_size = ftell(myfile);
            BufferROM = (unsigned char*)malloc(game_size);
            rewind(myfile);
            fread(BufferROM, 1, game_size, myfile);
            fclose(myfile);
            i=0;
            int new =0;
            int old =0;
            address = 0;
            SDL_Log("Writing flash with algo %d . In progress...\n ",flash_algo);
            while(i<game_size)
            {
                usb_buffer_out[0] = WRITE_MD_FLASH; // Select write in 16bit Mode
                usb_buffer_out[1] = address & 0xFF;
                usb_buffer_out[2] = (address & 0xFF00)>>8;
                usb_buffer_out[3] = (address & 0xFF0000)>>16;

                if((game_size - i)<54)
                {
                    usb_buffer_out[4] = (game_size - i); //adjust last packet
                }
                else
                {
                    usb_buffer_out[4] = 54; //max 58 bytes - must by pair (word)
                }

                memcpy((unsigned char *)usb_buffer_out +5, (unsigned char *)BufferROM +i, usb_buffer_out[4]);

                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                i += usb_buffer_out[4];
                address += (usb_buffer_out[4]>>1);
                new=(100 * i)/game_size;
                if(new!=old)
                {
                    old=new;
                    SDL_Log("WRITE SMD flash in progress: %ld%%", new);
                    fflush(stdout);
                }
            }
            printf("SMD flash completed !\n");
        }
        else  // MD Dumper New Flash write code with CSV
        {
            //SDL_Log("Execute Flash Write code CSV\n");
            SDL_Log("Write Mode : Erase Flash Data\n");
            SDL_Log("Launch Flash Erase command ... \n");
            SDL_Log("Detecting Flash Memory... \n");
            address=0;
            i=0;
            while (i<8)
            {

                usb_buffer_out[0] = READ_MD;
                usb_buffer_out[1] = address&0xFF ;
                usb_buffer_out[2] = (address&0xFF00)>>8;
                usb_buffer_out[3]=(address & 0xFF0000)>>16;
                usb_buffer_out[4] = 0; // Slow Mode

                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                libusb_bulk_transfer(handle, 0x82,dump_flash+(64*i),64, &numBytes, 60000);
                address+=32;
                i++;
            }
            SDL_Log("Detecting Flash Memory... \n");
            address=0;
            usb_buffer_out[0] = READ_MD;
            usb_buffer_out[1] = address&0xFF ;
            usb_buffer_out[2] = (address&0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4] = 0; // Slow Mode

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82,usb_buffer_in,64, &numBytes, 60000);

            //printf("Flash data at address 0 : 0x%02X \n",usb_buffer_in[0]);
            //printf("Flash data at address 1 : 0x%02X \n",usb_buffer_in[1]);
            rom_id = (usb_buffer_in[1]<<8) | usb_buffer_in[0];

            usb_buffer_out[0] = INFOS_ID;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);

            manufacturer_id = usb_buffer_in[1];
            chip_id = usb_buffer_in[3];
            flash_id = (manufacturer_id<<8) | chip_id;

            SDL_Log("Flash ID : %04X \n",flash_id);

            for (i = 0; i < chipid_text_values_count; i++)
            {
                strncpy(txt_csv_deviceID,chipid_text_values[i],4);
                //SDL_Log(" \n txt chipid value : %s \n",txt_csv_deviceID);
                csv_deviceID = (unsigned short)strtol(txt_csv_deviceID, NULL, 16);
                //SDL_Log(" \n DEC Device ID value : %ld \n",csv_deviceID);

                // If found we need to copy all usefull info from CSV to MD dumper Var
                if ( flash_id == csv_deviceID  )
                {
                    Index_chksm = i;
                    SDL_Log("Found chip in CSV Flashlist ! \n");
                    SDL_Log("Position in csv table %d \n",i);

                    // Flash Size
                    strncpy(txt_csv_flash_size,chipid_text_values[i]+5,3);
                    txt_csv_flash_size[4] = '\0'; // Null-terminate the output string
                    //printf("Txt flash size : %s \n",txt_csv_flash_size);
                    csv_flash_size = (unsigned char)strtol(txt_csv_flash_size, NULL, 10);
                    //printf("CSV Flash Size  %d \n",csv_flash_size);
                    flash_size=1024*csv_flash_size;
                    //printf("La valeur de FlashSize est %ld Ko \n",flash_size);

                    // Algo Erase
                    strncpy(txt_csv_erase_algo,chipid_text_values[i]+9,2);
                    txt_csv_erase_algo[2] = '\0'; // Null-terminate the output string
                    //printf("Txt Erase Algo : %s \n",txt_csv_erase_algo);
                    csv_erase_algo = (unsigned char)strtol(txt_csv_erase_algo, NULL, 8);
                    //printf("CSV Erase Algo  %d \n",csv_erase_algo);

                    // Write Algo
                    strncpy(txt_csv_write_algo,chipid_text_values[i]+12,2);
                    txt_csv_write_algo[2] = '\0'; // Null-terminate the output string
                    //printf("Txt Write Algo  : %s \n",txt_csv_write_algo);
                    csv_write_algo = (unsigned char)strtol(txt_csv_write_algo, NULL, 8);
                    //printf("CSV Write Algo %d \n",csv_write_algo);

                    // Chip Name
                    strncpy(txt_csv_flash_name,chipid_text_values[i]+15,11);
                    txt_csv_flash_name[12] = '\0'; // Null-terminate the output string
                    //printf("Flash Device Reference : %s \n",txt_csv_flash_name);

                    // Voltage

                    strncpy(txt_csv_voltage,chipid_text_values[i]+27,2);
                    txt_csv_voltage[2] = '\0'; // Null-terminate the output string
                    //printf("Txt Chip Voltage : %s \n",txt_csv_voltage);
                    csv_voltage = (unsigned char)strtol(txt_csv_voltage, NULL, 8);
                    //printf("CSV Chip Voltage  %d \n",csv_voltage);

                    // Manufacturer

                    strncpy(txt_csv_man_name,chipid_text_values[i]+30,18);
                    txt_csv_man_name[19] = '\0'; // Null-terminate the output string
                    //printf("Chip Manufacturer : %s \n",txt_csv_man_name);

                    SDL_Log("Memory : %s \n",txt_csv_flash_name);
                    SDL_Log("Capacity %ld Ko \n",flash_size);
                    SDL_Log("Chip Manufacturer : %s \n",txt_csv_man_name);
                    SDL_Log("Chip Voltage %ld V \n",csv_voltage);
                    SDL_Log("CSV Erase Algo  %d \n",csv_erase_algo);
                    SDL_Log("CSV Write Algo %d \n",csv_write_algo);

                }
            }

            // At this point flash ID is known so we can start Write flash with csv algo

            SDL_Log("Memory : %s \n",txt_csv_flash_name);
            SDL_Log("Detect if Flash is empty... \n");
            if(memcmp((char *)dump_flash,(char *)empty_flash,512) == 0)
            {
                SDL_Log("Flash is empty !\n");
            }
            else
            {
                SDL_Log("Flash Memory is not empty \n");

                SDL_Log("Erasing flash with algo %d \n ",csv_erase_algo);
                usb_buffer_out[0] = ERASE_MD_FLASH;
                usb_buffer_out[1] = csv_erase_algo;
                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                i=0;
                while(usb_buffer_in[0]!=0xFF)
                {
                    libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                    SDL_Log("\rERASE SMD flash in progress: %s ", wheel[i]);
                    fflush(stdout);
                    i++;
                    if(i==4)
                    {
                        i=0;
                    }
                }

                SDL_Log("\rERASE SMD flash in progress: 100%%");
                SDL_Log("\nFlash Erased sucessfully \n");
                fflush(stdout);
            }
            if (csv_write_algo != 5 )
            {

                SDL_Log(" Please enter rom file name \n ");
                SDL_Log(" ROM file: ");
                scanf("%60s", dump_name);
                myfile = fopen(dump_name,"rb");
                fseek(myfile,0,SEEK_END);
                game_size = ftell(myfile);
                buffer_rom = (unsigned char*)malloc(game_size);
                rewind(myfile);
                fread(buffer_rom, 1, game_size, myfile);
                fclose(myfile);
                i=0;
                j=0;
                k=0;
                address = 0;


                SDL_Log("Writing flash with algo %d \n ",csv_write_algo);

                // Send correct flah Algo to MD dumper

                usb_buffer_out[0] = SEND_FLASH_ALGO;
                usb_buffer_out[4] = csv_write_algo;
                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                while ( usb_buffer_in[6] != 0xDD)
                {
                    libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
                }

                timer_start();
                while(i<game_size)
                {

                    usb_buffer_out[0] = WRITE_MD_FLASH; // Select write in 16bit Mode
                    usb_buffer_out[1] = address & 0xFF;
                    usb_buffer_out[2] = (address & 0xFF00)>>8;
                    usb_buffer_out[3] = (address & 0xFF0000)>>16;

                    if((game_size - i)<54)
                    {
                        usb_buffer_out[4] = (game_size - i); //adjust last packet
                    }
                    else
                    {
                        usb_buffer_out[4] = 54; //max 58 bytes - must by pair (word)
                    }

                    memcpy((unsigned char *)usb_buffer_out +5, (unsigned char *)buffer_rom +i, usb_buffer_out[4]);

                    libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                    i += usb_buffer_out[4];
                    address += (usb_buffer_out[4]>>1);
                    SDL_Log("\r WRITE SMD flash in progress: %ld%%", ((100 * i)/game_size));
                    fflush(stdout);
                }
                SDL_Log("\r SMD flash completed\n");
                timer_end();
                timer_show();
                free(buffer_rom);
            }
             if (csv_write_algo == 5 ) // Special Write mode for flash type MX29L3211

             {
                 SDL_Log(" Please enter rom file name \n ");
                 SDL_Log(" ROM file: ");
                 scanf("%60s", dump_name);
                 myfile = fopen(dump_name,"rb");
                 fseek(myfile,0,SEEK_END);
                 game_size = ftell(myfile);
                 buffer_rom = (unsigned char*)malloc(game_size);
                 rewind(myfile);
                 fread(buffer_rom, 1, game_size, myfile);
                 fclose(myfile);
                 i=0;
                 j=0;
                 k=0;
                 address = 0;

                 SDL_Log("Writing flash with algo %d \n ",csv_write_algo);

                 // Send correct flah Algo to MD dumper

                 usb_buffer_out[0] = SEND_FLASH_ALGO;
                 usb_buffer_out[4] = csv_write_algo;
                 libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                 while ( usb_buffer_in[6] != 0xDD)
                 {
                     libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
                 }

                 timer_start();

                 // printf("Create MX Buffer...\n");
                 usb_buffer_out[0] = CREATE_MX_BUFFER;
                 libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                 i=0;
                 while(usb_buffer_in[6]!=0xAA)
                 {
                     libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                 }
                 //  printf("Buffer MX Created sucessfully ! \n");

                 while (k < game_size)
                 {

                     // Send ROM To MX Buffer
                     while (j!=8)
                     {


                         // Fill usb out buffer with save data in 8bit

                         for (i=0; i<32; i++)
                         {
                             usb_buffer_out[32+i] = buffer_rom [i+k];
                         }
                         k=k+32;
                         i=0;

                         //  printf("Send ROM to MX Buffer...\n");
                         usb_buffer_out[0] = WRITE_MX_BUFFER; // Select write in 16bit Mode
                         usb_buffer_out[1] = address & 0xFF;
                         usb_buffer_out[2] = (address & 0xFF00)>>8;
                         usb_buffer_out[3] = (address & 0xFF0000)>>16;
                         usb_buffer_out[6] = j;
                         j=j+1;

                         libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 6000);
                         while(usb_buffer_in[6]!=0xBB)
                         {
                             libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                         }


                     }
                     j=0;


                     // printf("Flash Buffer in memory...\n");
                     usb_buffer_out[0] = FLASH_MX_BUFFER; // Select write in 16bit Mode
                     usb_buffer_out[1] = address & 0xFF;
                     usb_buffer_out[2] = (address & 0xFF00)>>8;
                     usb_buffer_out[3] = (address & 0xFF0000)>>16;

                     libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 6000);
                     while(usb_buffer_in[6]!=0xCC)
                     {
                         libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);   //wait status
                     }

                     // printf("Buffer Flashed ! \n");

                     address=address+128;
                     SDL_Log("\r Flash ROM in progress: %ld%%", ((200 * (address))/		 (game_size)));
                     fflush(stdout);
                 }

                 SDL_Log("MX Flashed sucessfully ! \n");
                 timer_end();
                 timer_show();
             }
        }
        return 0;
}

int Erase_RAM(void)
{
        SDL_Log("Write Mode : Erase Save Data\n");
        SDL_Log("ALL SRAM DATAS WILL BE ERASED ...\n");
        address=(save_address/2);
        usb_buffer_out[0] = WRITE_MD_SAVE; // Select write in 8bit Mode
        usb_buffer_out[1]=address & 0xFF;
        usb_buffer_out[2]=(address & 0xFF00)>>8;
        usb_buffer_out[3]=(address & 0xFF0000)>>16;
        usb_buffer_out[7] = 0xBB;  // SRAM Clean Flag
        libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
        while ( usb_buffer_in[6] != 0xAA)
        {
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
        }

        SDL_Log("SRAM Sucessfully Erased !\n");
        return 0;
}

int Write_SRAM(void)
{
SDL_Log("Write Mode : Write Save Data\n");
        sfd_Options opt =
        {
            .title        = "Select your Save Data File",
            .filter_name  = "Save Data",
            .filter       = "*.*",
        };
        const char *filename = sfd_open_dialog(&opt);
        if (filename)
        {
            SDL_Log("You selected the file: '%s'\n", filename);
        }
        else
        {
            SDL_Log("Operation canceled\n");
            return 0;
        }
        SDL_Log(" ALL DATAS WILL BE ERASED BEFORE ANY WRITE!\n");
        myfile = fopen(filename,"rb");
        fseek(myfile,0,SEEK_END);
        save_size = ftell(myfile);
        BufferSAVE = (unsigned char*)malloc(save_size);
        rewind(myfile);
        fread(BufferSAVE, 1, save_size, myfile);
        fclose(myfile);

        // Erase SRAM

        address=(save_address/2);
        usb_buffer_out[0] = WRITE_MD_SAVE; // Select write in 8bit Mode
        usb_buffer_out[1]=address & 0xFF;
        usb_buffer_out[2]=(address & 0xFF00)>>8;
        usb_buffer_out[3]=(address & 0xFF0000)>>16;
        usb_buffer_out[7] = 0xBB;  // SRAM Clean Flag
        libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
        while ( usb_buffer_in[6] != 0xAA)
        {
            libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
        }

        SDL_Log("SRAM Sucessfully Erased\n");

        // Write SRAM

        i=0;
        j=0;
        unsigned long k=1;
        // save_size=save_size/2; // 16 bit input to 8 bit out
        address=(save_address/2);
        while ( j < save_size)
        {

            // Fill buffer 8bit with save_data

            // Fill usb out buffer with save data in 8bit
            for (i=32; i<64; i++)
            {
                usb_buffer_out[i] = BufferSAVE[k];
                k=k+2;

            }
            i=0;
            j+=64;

            usb_buffer_out[0] = WRITE_MD_SAVE; // Select write in 8bit Mode
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[7] = 0xCC;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            while ( usb_buffer_in[6] != 0xAA)
            {
                libusb_bulk_transfer(handle, 0x82, usb_buffer_in, sizeof(usb_buffer_in), &numBytes, 6000);
            }
            address+=32;
        }

        SDL_Log("SRAM Sucessfully Writted !\n");
        return 0;
}
