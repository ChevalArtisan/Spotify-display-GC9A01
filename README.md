# Spotify-display-GC9A01
Linking an esp32 S3 and a GC9A01 display for remote Spotify display and control .

# Features
* Show the currently playing song (Album cover + Song title + Artist's name)
* Play next song
* Pause Play
* Play previous song

# Setup

1. Go to the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/applications).
2. Create a new application and copy your **Client ID** and **Client Secret**
3. Add the following redirect URI: <https://spotifyesp32.vercel.app/api/spotify/callback>
4. Enable the **Web API** option.
5. Create a new tab **myAccess.h** in ArduinoIDE 
6. Copy your **SSID**, **PASSWORD**, **CLIENT_ID** and **CLIENT_SECRET** in **myAccess.h** as **const char***
7. Run the program one time to get your **REFRESH_TOKEN** (uncomment the code after **sp.begin()** and remove REFRESH_TOKEN in the instance sp to get it through the serial monitor) and copy the **REFRESH_TOKEN** in **myAccess.h**
8. Comment the refresh token part

# Wiring :

* RST     →   GPIO 4 
* CS      →   GPIO 6
* DC      →   GPIO 5
* SDA     →   GPIO 7
* SCL     →   GPIO 3
* GND     →   GND
* VCC     →   3.3V
* SKIP    →   GPIO 13
* PAUSE    →   GPIO 12
* PREV   →   GPIO 11

# Credit
**FinianLandes/SpotifyEsp32:** https://github.com/FinianLandes/SpotifyEsp32/tree/main
