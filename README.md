# VRChat-YT-DLP-Fix

> Archival notice: \
> Youtube's gotten more aggressive as of late and this is not gartuneed to work anymore if you live outside of the US. Head over to [VRCVideoCacher](https://github.com/EllyVR/VRCVideoCacher) for a still actively maintained solution!

fuyukiS' edit notice: this fork still works lol

Major edits:
1. Starts as a tray icon and keeps running until VRChat exits, because sometimes VRChat replaces their custom YT-DLP back when loading another world;
2. No longer forces `--cookies-from-browser` for those who want to use cookies outside of their default browser
3. No longer downloads latest `yt-dlp` every time as most of the time it is just a waste of internet traffic

_________
This program fixes VRChat's YT-DLP "Sign in to confirm you're not a bot" warning by replacing their stripped down version with the official version of YT-DLP, allowing VRChat to operate using your actual web-browser's sign in cookies.

This is an alternative to [VRCVideoCacher](https://github.com/EllyVR/VRCVideoCacher), which also tackles the same issue but through a different method:
- Mine utilizies YT-DLP's normal config file (in your AppData/Roaming/yt-dlp folder) to specify which browser YT-DLP's built in functionality should pull its cookies from, fixing the YT-DLP issue at a system level (as opposed to only fixing it for VRChat.)
- Theirs requires installing a browser extension which export your browser's cookies to their program.
_________

### **This requires [VRCX](https://github.com/vrcx-team/VRCX ) to load.**

________

### Usage: 
1) [Download](https://github.com/ShizCalev/VRChat-YT-DLP-Fix/releases) and extract the .exe file wherever you want.
1) Open VRCX, navigate to App Launcher.
1) Turn on the "Enable" slider.
1) Click the Auto-Launch Folder button.
1) Create a shortcut to VRChat-YT-DLP-Fix.exe in this folder. **(If you extract the .exe into this folder, you still have to create a shortcut.)**

And you're set! Everything's automatic from there.
   
![Screenshot 2025-05-31 225412](https://github.com/user-attachments/assets/850cc3a0-4e54-4e40-8b56-96a12c8157c3)

![image](https://github.com/user-attachments/assets/04cc563c-3b76-4e6a-802a-5944a0edb6c0)


- If you are using browser containers (if you don't know what that means, then you most likely aren't using them), you will have to manually edit AppData/Roaming/yt-dlp/config and specify what container you want to pull cookies from - ie
    - `--cookies-from-browser firefox::"youtube tabs"` (including the quotes.)

- If you don't want to use cookies from your browser or you want to use another YouTube account, see [this](https://github.com/yt-dlp/yt-dlp/wiki/FAQ#how-do-i-pass-cookies-to-yt-dlp) and [this](https://github.com/yt-dlp/yt-dlp/wiki/Extractors#exporting-youtube-cookies) to get your `cookies.txt`, and create file `%APPDATA%/yt-dlp/config` with content `--cookies "path\to\cookies.txt"`.

--------



#### For a bit more in depth explanation of what this program does;
1) Waits for VRChat to open.
2) Deletes VRChat's custom/stripped down YT-DLP from `AppData\LocalLow\VRChat\VRChat\Tools`
3) Checks if an existing yt-dlp config file exists in `AppData/Roaming/yt-dlp`

    a) If it does exist, ~~it checks if the config is properly set to pull cookies from the default web browser.~~ (this is deleted for those who doesn't want so)
    - It also checks if the sleep between downloads feature is enabled (and if it isn't, setting both the min & max sleep times randomizes the delay, which is critical to preventing youtube from flagging the traffic as a scraping bot. 
    
    b) If the config file does not exist, generate one with all the above parameters.
4) Downloads the latest release binary of YT-DLP from [YT-DLP's repo](https://github.com/yt-dlp/yt-dlp) and stores it for later use.
    - If you find this not to work one day, delete the `yt-dlp.exe` within the same folder of the executive, and try again.
5) Waits for VRChat to automatically regenerate its custom YT-DLP file during user login.
6) Delete VRChat's stripped down YT-DLP file (again.)
7) Places the official version of YT-DLP in `AppData\LocalLow\VRChat\VRChat\Tools`, fully replacing VRChat's stripped down version.
8) Waits for the next time VRChat tries to replace YT-DLP with their custom one (this may happen when loading another world).

--------------
### Build

> i don't like visual studio - fuyukiS

Simply run `build.bat` if you have MinGW (gcc) installed.

All Visual Studio project files are from the [original repo](https://github.com/ShizCalev/VRChat-YT-DLP-Fix), and i have no idea whether this code still works in Visual Studio.

--------------

Made by Afevis, released under GPL V3. 

Edit by fuyukiS

------------
No warranties are made for this software. Using YT-DLP is against Youtube's TOS, which you're already violating by using video players in VRChat - usage may or may not present additional risk by tying your video activity to your actual YouTube account (really it depends on if YouTube feels like being a dick and actually caring.)

