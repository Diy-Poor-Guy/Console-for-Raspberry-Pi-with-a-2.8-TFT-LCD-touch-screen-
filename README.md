# Console-for-Raspberry-Pi-with-a-2.8-TFT-LCD-touch-screen-
Bash console for Raspberry Pi with a 2.8" TFT LCD with on-screen keyboard &amp; terminal display - NO X-Windows

<a href="https://github.com/Diy-Poor-Guy/Console-for-Raspberry-Pi-with-a-2.8-TFT-LCD-touch-screen-/tree/main/Images"><img src="IMG_2760.JPG" height="300"/></a>


This is a console app for Raspberry Pi with a 2.8" TFT LCD touch 
screen (ADS784)

It runs the bash shell and implements a on screen touch keyboard 
and a small terminal display to service stdin and stdout of bash 
respectively so you can execute all basic Linux commands in text 
mode (and manualy start your own apps).

This does not use X-Windows, so no overhead!!!

Three font sizes are available.

Do "console -h" to see options.

Just add this to the bottom of the .bashrc file in the pi home directory:
(this will run it at boot time and ensure that only one instanse is started)

        ps -C console
        if [[ $? -eq 0 ]] ;
        then
            echo "FOUND"
        else 
            echo "NOT FOUND"
            /Your_Own_Install_Directory_Name/console -f3 > /dev/null 2> /dev/null
        fi

This is derived from work done in 2013 by Mark Williams - THANKS MAN !
   https://github.com/mwilliams03/Pi-Touchscreen-basic/blob/master/buttonExample.c

Note: If it breaks - you get to keep all the pieces.
