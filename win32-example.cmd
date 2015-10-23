@ECHO OFF
TITLE Windows Command Shell:  Zelda Majora's Mask

REM NOTE:  This is an example shell script to run ZS.EXE on Windows with a
REM        successful set of parameters for hacking the save file.

SET SAVE_FILE="%USERPROFILE%\project64\Bin\Release64\Save\ZELDAM~1.FLA"
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
 -= 3 "Force 32-bit byte order swap OUTPUT (not input) (most N64 emulators)."^
 -@ 0^
 "-@ 0 means File 1 receives all the below changes."^
 ^
 "Write on the console which mask Link is currently wearing."^
 -mask^
 ^
 -mask 0x00 "Sets our preference:  Link is wearing no mask on his face."^
 -Fairy^
 -Fairy 0^
 -Name^
 -Name 0x0A0A0A0A0A0A0A0A "File 1's name:  AAAAAAAA"^
 -ClockTownReached 1^
 -Day^
 -Day 2 "Read the GitHub wiki if you want to know what this does. :)"^
 -zeldaTime^
 -zeldaTime 0x7FFF^
 -ZeldaVectorTimeRate^
 -ZeldaVectorTimeRate -3^
 ^
 -fairies 0 13 "Woodfall:  13 stray fairies"^
 -keys 0 9^
 -dungeonProgress 0 7^
 -fairies 1 14 "Snowhead:  14"^
 -keys 1 9^
 -dungeonProgress 1 7^
 -fairies 2 15^
 -keys 2 9^
 -dungeonProgress 2 7^
 -fairies 3 255 "Ikana:  too many stray fairies :)"^
 -keys 3 9^
 -dungeonProgress 3 7^
 -keys 9 19 "bug in ROM that reads keys[9] as double-defense heart containers"^
 ^
 -rupees 500^
 -RazorSwordHP 9001^
 -playAsForm^
 -playAsForm 4 "Change this to 0 for Fierce Deity Link.  Again, RTFM on wiki."^
 -LifeHP(current) 1 0x140^
 -LifeHP(maximal) 0 0x140^
 -MagicNow 1 0^
 -MagicMax 0 +127^
 -M 1^
 -M 0^
 -owlStatues 1000001111111111^
 -Upgrades^
 -Upgrades 0xFFFFE01B^
 ^
 -lottery 0 4 2 0^
 -lottery 1 4 2 0^
 -lottery 2 4 2 0^
 ^
 -EquipSwordAndShield 0x0034^
 ^
 -I it 0 0 00 -I it 1 0 01 -I it 2 0 02 -I it 3 0 03 -I it 4 0 04 -I it 5 0 05^
 -I it 0 1 06 -I it 1 1 07 -I it 2 1 08 -I it 3 1 09 -I it 4 1 0A -I it 5 1 0B^
 -I it 0 2 0C -I it 1 2 0D -I it 2 2 0E -I it 3 2 0F -I it 4 2 10 -I it 5 2 11^
 -I it 0 3 12 -I it 1 3 16 -I it 2 3 20 -I it 3 3 25 -I it 4 3 27 -I it 5 3 15^
 -Inventory masks^
 ^
 -i 1 50 "Have 50 arrows."^
 -i 6 40 -i 7 40 "Have 40 bombs and bombchu."^
 -i 8 10 "Have 10 deku sticks.  (I hate how only OOT has an upgrade for 20.)"^
 -i 9 20 "Have 20 deku nuts.  (Again, only OOT seems to have upgrades, here.)"^
 -i 10 20 "Have 20 magic beans."^
 -i 12 2 "Have...TWO powder kegs!  Though the max is still 1. :("^
 -i 13 0^
 -i 0 1 -i 2 1 -i 3 1 -i 4 1 -i 5 1 -i 11 1 -i 14 1 -i 15 1 -i 16 1 -i 17 1^
 -i 18 1 -i 19 1 -i 20 1 -i 21 1 -i 22 1 -i 23 1

pause
