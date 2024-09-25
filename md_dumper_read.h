int Read_ROM_Auto(void)
{
        SDL_Log("\n");
        SDL_Log("Read Mode : Read ROM in automatic mode\n");

        SDL_Log("Sending command Dump ROM \n");
        SDL_Log("Dumping please wait ...\n");
        timer_start();
        address=0;
        game_size *= 1024;		//game_size=4096*1024;
        SDL_Log("\n");
        SDL_Log("Rom Size : %ld Ko \n",game_size/1024);
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
        SDL_Log("ROM dump in progress...\n");
        res = libusb_bulk_transfer(handle, 0x82,BufferROM,game_size, &numBytes, 0);
        if (res != 0)
        {
            SDL_Log("Error \n");
            return 1;
        }
        SDL_Log("\n");
        SDL_Log("Dump ROM completed !\n");
        timer_end();
        timer_show();
        myfile = fopen("dump_smd.bin","wb");
        fwrite(BufferROM, 1,game_size, myfile);
        fclose(myfile);
        return 0;
}

int Read_ROM_Manual(void)
{
        SDL_Log("\n");
        SDL_Log("Read Mode : Read ROM in manual mode\n");

        SDL_Log("Sending command Dump ROM \n");
        SDL_Log("Dumping please wait ...\n");
        timer_start();
        switch(dump_rom_size_opts)
			{
			case 0:
				game_size = 32 * 1024;
				break;
			case 1:
				game_size = 64 * 1024;
				break;
			case 2:
				game_size = 128 * 1024;
				break;
			case 3:
				game_size = 256 * 1024;
				break;
			case 4:
				game_size = 512 * 1024;
				break;
			case 5:
				game_size = 1024 * 1024;
				break;
			case 6:
				game_size = 2048 * 1024;
				break;
			case 7:
				game_size = 4096 * 1024;
				break;
			case 8:
				game_size = 8192 * 1024;
				break;
			}
        
        SDL_Log("\n");
        SDL_Log("Rom Size (Manual Mode) : %ld Ko \n",game_size/1024);
        BufferROM = (unsigned char*)malloc(game_size);
        // Cleaning ROM Buffer
        for (i=0; i<game_size; i++)
        {
            BufferROM[i]=0x00;
        }

		if(dump_cart_mode_opts==0)						//Game Gear Mode
        {
			SDL_Log("Game Gear Mode : ROM dump in progress...\n");
		}
        if(dump_cart_mode_opts==1)						//Mega Drive Mode
        {
            address = 0;
            usb_buffer_out[0] = READ_MD;
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=1;

            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 0);
            SDL_Log("Mega Drive Mode : ROM dump in progress...\n");
            res = libusb_bulk_transfer(handle, 0x82,BufferROM,game_size, &numBytes, 0);
            if (res != 0)
            {
                SDL_Log("Error \n");
                return 1;
            }
            SDL_Log("\n");
            SDL_Log("Dump ROM completed !\n");
            timer_end();
            timer_show();
            myfile = fopen("dump_smd.bin","wb");
            fwrite(BufferROM, 1,game_size, myfile);
            fclose(myfile);
        }
        else if(dump_cart_mode_opts==2)					//Master System Mode
        {
            address = 0;
            int i=0;
            SDL_Log("Master System Mode : ROM dump in progress...\n");
            while (i<game_size)
            {
                usb_buffer_out[0] = READ_SMS;
                usb_buffer_out[1] = address&0xFF ;
                usb_buffer_out[2] = (address&0xFF00)>>8;
                usb_buffer_out[3] = (address & 0xFF0000)>>16;
                usb_buffer_out[4] = 0; // Slow Mode
                usb_buffer_out[5] = 0;
                libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
                libusb_bulk_transfer(handle, 0x82,(BufferROM+i),64, &numBytes, 60000);
                address +=64;
                i+=64;
            }
            SDL_Log("\n");
            SDL_Log("Dump ROM completed !\n");
            timer_end();
            timer_show();
            myfile = fopen("dump_sms.sms","wb");
            fwrite(BufferROM, 1,game_size, myfile);
            fclose(myfile);
        }
        return 0;
}

int Read_ROM_Bankswitch(void)
{
        SDL_Log("\n");
        SDL_Log("Read Mode : Read ROM in mode : Bankswitch SSF2 \n");
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
        }

        // Search checksum cartridge in Custom Hardware games csv table
        i=0;
        for (i = 0; i < chksm_text_values_count ; i++)
        {
            strncpy(txt_csv_chksm,chksm_text_values[i],4);
            //SDL_Log(" txt chksm value : %s \n",txt_csv_chksm);
            csv_chksm = (unsigned short)strtol(txt_csv_chksm, NULL, 16);

            if ( checksum_header == csv_chksm  )
            {
                Index_chksm = i;
                SDL_Log("\n");
                SDL_Log("Found game in extra CSV Gamelist  \n");
                SDL_Log("Position in csv table %d \n",i);
                strncpy(txt_csv_game_size,chksm_text_values[i]+5,4);
                txt_csv_game_size[4] = '\0'; // Null-terminate the output string
                //SDL_Log(" txt game size : %s \n",txt_csv_game_size);
                csv_game_size = (unsigned char)strtol(txt_csv_game_size, NULL, 10);
                //SDL_Log(" CSV Game Size  %d \n",csv_game_size);
                game_size=1024*csv_game_size;
                SDL_Log("ROM Size from CSV is %ld Ko \n",game_size);
            }
        }
        NumberOfBank = game_size/512;
        //SDL_Log("Game Size is %ld Ko \n",game_size);
        SDL_Log("Number of Banks is %d \n",NumberOfBank);
        SDL_Log("Bank Size is 512 Ko  \n");

        game_size = game_size * 1024;
        BufferROM = (unsigned char*)malloc(game_size);
        // Cleaning ROM Buffer
        for (i=0; i<game_size; i++)
        {
            BufferROM[i]=0x00;
        }

        // Dump the first 4MB of the ROM as fast as possible
        SDL_Log("Bankswith bank O-7 to $080000 - $3FFFFF \n");

        SDL_Log("Dumping please wait ...\n");
        timer_start();
        address = 0;
        i=0;

        usb_buffer_out[0] = READ_MD;
        usb_buffer_out[1]=address & 0xFF;
        usb_buffer_out[2]=(address & 0xFF00)>>8;
        usb_buffer_out[3]=(address & 0xFF0000)>>16;
        usb_buffer_out[4]=1;

        libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
        SDL_Log("ROM dump in progress...\n");
        res = libusb_bulk_transfer(handle, 0x82,BufferROM,4096*1024, &numBytes,0);
        if (res != 0)
        {
            SDL_Log("Error \n");
            return 1;
        }

        ActualBank = 8;
        offset = 4096 - 1024;

        while ( offset != (game_size/1024)-1024)
        {
            SDL_Log("Bankswith bank %d - %d to $200000 - $2FFFFF \n",ActualBank,ActualBank+1);
            SDL_Log("Dumping please wait ...\n");

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
        SDL_Log("\n");
        SDL_Log("Dump ROM completed !\n");
        timer_end();
        timer_show();
        myfile = fopen("dump_smd.bin","wb");
        fwrite(BufferROM,1,game_size, myfile);
        fclose(myfile);
        return 0;
}

int Read_RAM_Auto(void)
{
        SDL_Log("Read Mode Auto: Read Save Data\n");
        save_size *= 1024;

        BufferROM = (unsigned char*)malloc(save_size); // raw buffer
        BufferSAVE = (unsigned char*)malloc((save_size*2)); // raw in 16bit format

        for (i=0; i<(save_size*2); i++)
        {
            BufferSAVE[i]=0x00;
        }

        usb_buffer_out[0] = READ_MD_SAVE;
        address=(save_address/2);
        i=0;
        while ( i< save_size)
        {
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=0;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82,(BufferROM+i),64, &numBytes, 60000);
            address +=64; //next adr
            i+=64;
            fflush(stdout);
        }
        i=0;
        j=0;
        myfile = fopen("raw.srm","wb");
        fwrite(BufferROM,1,save_size, myfile);

        for (i=0; i<save_size; i++)
        {
            j=j+1;
            BufferSAVE[i+j]=BufferROM[i];
        }

        myfile = fopen("dump_smd.srm","wb");
        fwrite(BufferSAVE,1,save_size*2, myfile);
        fclose(myfile);
        SDL_Log("\n");
        SDL_Log("Save Data completed !\n");
        timer_end();
        timer_show();
        return 0;
}

int Read_RAM_Bankswitch(void)
{
        SDL_Log("Read Mode Bankswitch: Read Save Data\n");
        SDL_Log("TODO !...\n");
}

int Read_RAM_Manual(void)
{
        SDL_Log("Read Mode Manual : Read Save Data\n");
        SDL_Log("Reading in progress...\n");
        SDL_Log(%ld,dump_sram_size_opts);
        timer_start();
        if(dump_sram_size_opts==1)
            save_size = 8192;
        else if(dump_sram_size_opts==2)
            save_size = 32768;
        else
            save_size *= 1024;

        BufferROM = (unsigned char*)malloc(save_size); // raw buffer
        BufferSAVE = (unsigned char*)malloc((save_size*2)); // raw in 16bit format

        for (i=0; i<(save_size*2); i++)
        {
            BufferSAVE[i]=0x00;
        }

        usb_buffer_out[0] = READ_MD_SAVE;
        address=(save_address/2);
        i=0;
        while ( i< save_size)
        {
            usb_buffer_out[1]=address & 0xFF;
            usb_buffer_out[2]=(address & 0xFF00)>>8;
            usb_buffer_out[3]=(address & 0xFF0000)>>16;
            usb_buffer_out[4]=0;
            libusb_bulk_transfer(handle, 0x01,usb_buffer_out, sizeof(usb_buffer_out), &numBytes, 60000);
            libusb_bulk_transfer(handle, 0x82,(BufferROM+i),64, &numBytes, 60000);
            address +=64; //next adr
            i+=64;
            fflush(stdout);
        }
        i=0;
        j=0;
        myfile = fopen("raw.srm","wb");
        fwrite(BufferROM,1,save_size, myfile);

        for (i=0; i<save_size; i++)
        {
            j=j+1;
            BufferSAVE[i+j]=BufferROM[i];
        }

        myfile = fopen("dump_smd.srm","wb");
        fwrite(BufferSAVE,1,save_size*2, myfile);
        fclose(myfile);
        SDL_Log("\n");
        SDL_Log("Save Data completed !\n");
        timer_end();
        timer_show();
        return 0;
}
