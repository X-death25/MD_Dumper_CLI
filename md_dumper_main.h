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
