int Write_Flash()
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
}
