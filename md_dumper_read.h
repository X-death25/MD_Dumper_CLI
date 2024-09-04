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
