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
REM         -f 0 15 "Have fifteen Woodfall stray fairies."
REM         -f 1 15 "Have fifteen Snowhead stray fairies." ... and so on

zs %SAVE_FILE%^
 -@ 0^
 "-@ 0 means File 1 receives all the below changes."^
 ^
 "Write on the console which mask Link is currently wearing."^
 -m^
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
 ^
 -l 0 4 2 0 "Day 1 winning lottery ticket numbers:  420"^
 -l 1 4 2 0 "Day 2 winning lottery ticket numbers:  420"^
 -l 2 4 2 0^
 -@ 0

pause
