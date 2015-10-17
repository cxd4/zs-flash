@ECHO OFF
TITLE Windows Command Shell:  Zelda Majora's Mask

REM NOTE:  This is an example shell script to run ZS.EXE on Windows with a
REM        successful set of parameters for hacking the save file.

SET SAVE_FILE="%USERPROFILE%\emulators\N64\Save\ZELDA MAJORA'S MASK.fla"
REM NOTE:  Adjust this as needed, to where your save file is.

REM NOTE:  `^` is a special character to the Windows shell for absorbing the
REM        upcoming line break so that we can split big commands into lines.
REM
REM        Example:  zs.exe %SAVE_FILE% -f 0 15 -f 1 15 -f 2 15 -f 3 15
REM        ... versus ...
REM        zs.exe %SAVE_FILE%^
REM         -f 0 15 "Have fifteen Woodfall stray fairies."^
REM         -f 1 15 "Have fifteen Snowhead stray fairies."^ ... and so on

zs %SAVE_FILE%^
 -@ 0^
 "-@ 0 means File 1 receives all the below changes."^
 ^
 "Write on the console which mask Link is currently wearing."^
 -m^
 "-&" "Ask to wipe the save file to a blank slate."^
 "-|" "Initialize basic data for a new game, like the ROM does."^
 ^
 -m 0x00 "Sets our preference:  Link is wearing no mask on his face."^
 -F^
 -F 0^
 -N^
 -N 0x152C312E3E3E3E3E "File 1's name:  Link"^
 -D^
 -D 2 "Read the GitHub wiki if you want to know what this does. :)"^
 -z^
 -z 0x7FFF^
 -Z^
 -Z -3^
 -f 0 13 "Woodfall:  13 stray fairies"^
 -f 1 14 "Snowhead:  14"^
 -f 2 15^
 -f 3 255 "Ikana:  too many :)"^
 -k 0 63^
 -k 1 127^
 -k 2 128^
 -k 3 255^
 -r 500^
 -R 9001^
 -d 0 0xC0^
 -d 1 0x30^
 -d 2 0x0C^
 -d 3 0x03^
 -p^
 -p 4 "Change this to 0 for Fierce Deity Link.  Again, RTFM on GitHub."^
 -L 0^
 -L 1^
 -L 1 0x130^
 -L 0 0x140^
 -M 1^
 -M 0^
 -M 1 0^
 -M 0 0x7F^
 -o 1000001111111111^
 -U^
 -U 0xFFFFE01B^
 ^
 -l 0 4 2 0 "Day 1 winning lottery ticket numbers:  420"^
 -l 1 4 2 0 "Day 2 winning lottery ticket numbers:  420"^
 -l 2 4 2 0^
 ^
 -I it 0 0 00 -I it 1 0 01 -I it 2 0 02 -I it 3 0 03 -I it 4 0 04 -I it 5 0 05^
 -I it 0 1 06 -I it 1 1 07 -I it 2 1 08 -I it 3 1 09 -I it 4 1 0A -I it 5 1 0B^
 -I it 0 2 0C -I it 1 2 0D -I it 2 2 0E -I it 3 2 0F -I it 4 2 10 -I it 5 2 11^
 -I it 0 3 12 -I it 1 3 16 -I it 2 3 20 -I it 3 3 25 -I it 4 3 27 -I it 5 3 15^
 -I masks^
 ^
 -i 1 50 "Have 50 arrows."^
 -i 6 40 -i 7 40 "Have 40 bombs and bombchu."^
 -i 8 10 "Have 10 deku sticks.  (I hate how only OOT has an upgrade for 20.)"^
 -i 9 20 "Have 20 deku nuts.  (Again, only OOT seems to have upgrades, here.)"^
 -i 10 20 "Have 20 magic beans."^
 -i 12 2 "Have...TWO powder kegs!  Though the max is still 1. :("^
 -i 13 0^
 -i 0 1 -i 2 1 -i 3 1 -i 4 1 -i 5 1 -i 11 1 -i 14 1 -i 15 1 -i 16 1 -i 17 1^
 -i 18 1 -i 19 1 -i 20 1 -i 21 1 -i 22 1 -i 23 1^
 -@ 0

pause
