# MD Dumper CLI

This software is used with the project here : https://www.tindie.com/products/xdeath/usb-megadrive-genesis-cartridge-readerwriter/

It allows you to dump Mega Drive cartridges and Master System cartridges (tested by using SEGA Master System converter, could work with others adaptaters). You can use the command line mode or the GUI mode according to your preferences.

<img src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/01.png" data-canonical-src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/01.png" width="49%" /> <img src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/card.jpg" data-canonical-src="https://github.com/X-death25/MD_Dumper_CLI/blob/main/github_gfx/card.jpg" width="49%" />

## Commands

### CLI Mode

Read mode :
```
-read a  :  Auto Mode
-read b  :  Bankswitch Mode
-read m (32|64|128|256|512|1024|2048|4096) (md|sms) :  Manual Mode
-read s (0|8192|32768) :  Read Save Data
```

Write mode :
```
-write f e  :  Erase Flash Memory
-write f w  :  Write Flash Memory
-write s e  :  Erase Save Memory
-write s w  :  Write Save Memory
```

### GUI Mode
```
-gui  : Using GUI Window 
```

## Notes

You will find license and links about the librairies and font used in this project in "Informations.txt" if interested.
