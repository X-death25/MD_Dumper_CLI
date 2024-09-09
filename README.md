# MD Dumper CLI

This software is used with the project here : https://www.tindie.com/products/xdeath/usb-megadrive-genesis-cartridge-readerwriter/

It allows you to dump Mega Drive cartridges and Master System cartridges (tested by using SEGA Master System converter, could work with others adaptaters). You can use the command line mode or the GUI mode according to your preferences.

<img src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/01.png" data-canonical-src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/01.png" width="49%" /> <img src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/card.jpg" data-canonical-src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/card.jpg" width="49%" />

## Commands

### CLI Mode

Read mode :
```
-read auto  :  Auto Mode
-read bankswitch  :  Bankswitch Mode
-read manual (32|64|128|256|512|1024|2048|4096) (gg|md|sms) :  Manual Mode
-backup auto  :  Auto Mode
-backup bankswitch  :  Bankswitch Mode
-backup manual (8192|32768) (serial_spi|serial_i2c|parallel_sram):  Read Save Data
```

Write mode :
```
-erase_flash  :  Erase Flash Memory
-write_flash  :  Write Flash Memory
-erase_memory (serial_spi|serial_i2c|parallel_sram) :  Erase Save Memory
-restore_memory (serial_spi|serial_i2c|parallel_sram)  :  Write Save Memory
```

### GUI Mode
```
-gui  : Using GUI Window 
```

## Notes

You will find license and links about the librairies and font used in this project in "Informations.txt" if interested.
