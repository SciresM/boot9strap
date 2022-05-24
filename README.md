# Boot9strap

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

Boot9/Boot11 code execution.

For more details, refer to the presentation [here](https://sciresm.github.io/33-and-a-half-c3/).

Install via [SafeB9SInstaller](https://github.com/d0k3/SafeB9SInstaller).

Launches "boot.firm" off of the SD card or CTRNAND.

## LED Status Codes
By holding `X + Start + Select` during boot, or if either FIRM file is corrupt, the notification LED will display the following:
- SD FIRM successfully loaded: green
- SD FIRM missing, CTRNAND FIRM successfully loaded: yellow
- SD FIRM corrupt, CTRNAND FIRM successfully loaded: orange
- SD FIRM missing, CTRNAND FIRM also missing: white
- SD FIRM missing, CTRNAND FIRM corrupt: magenta
- SD FIRM corrupt, CTRNAND FIRM also corrupt: red
- In addition to the above, the LED will blink if it is actually a ntrboot boot

## Credits

[Normmatt](https://github.com/Normmatt): Theorizing the NDMA overwite exploit.    
[TuxSH](https://github.com/TuxSH): Help implementing bootrom payloads.    
[Luma3DS](https://github.com/AuroraWright/Luma3DS): Codebase used in the stage 2 FIRM loader.    

## Licensing

This software is licensed under the terms of the GPLv3.  
You can find a copy of the license in the LICENSE file.
