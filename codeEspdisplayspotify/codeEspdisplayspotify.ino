#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyEsp32.h"

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>

#define DISABLE_PLAYLISTS
#define DISABLE_LIBRARY
#define DISABLE_USER
#define DISABLE_METADATA

#include "myAccess.h" // Contains  ssid password and API id and tokens

#define RST_PIN 4
#define DC_PIN  5
#define CS_PIN  6
#define SDA_PIN 7 
#define SCL_PIN 3 

#define SKIP_PIN 13
#define PAUSE_PIN 12
#define PREV_PIN 11


//USB CDC on boot enabled

Adafruit_GC9A01A tft = Adafruit_GC9A01A(CS_PIN, DC_PIN,RST_PIN);


// Create an instance of the Spotify class (optional: specify retry count)
Spotify sp(CLIENT_ID, CLIENT_SECRET,REFRESH_TOKEN);
String last_play;
volatile bool skipRequested = false;
volatile bool prevRequested = false;
volatile bool pauseRequested = false;


void IRAM_ATTR int_skip() { skipRequested = true; }
void IRAM_ATTR int_prev() { prevRequested = true; }
void IRAM_ATTR int_pause() { pauseRequested = true; }

void setup() {
 Serial.begin(115200);
 Serial.println("Lancement");

 pinMode(SKIP_PIN,INPUT_PULLUP);
  pinMode(PREV_PIN,INPUT_PULLUP);
 pinMode(PAUSE_PIN,INPUT_PULLUP);

  attachInterrupt(SKIP_PIN, int_skip, FALLING);
  attachInterrupt(PREV_PIN, int_prev, FALLING);
  attachInterrupt(PAUSE_PIN, int_pause, FALLING);

// On force l'initialisation du bus SPI avec tes broches spécifiques de l'S3
 SPI.begin(SCL_PIN, -1, SDA_PIN, CS_PIN); 

 tft.begin();
 tft.setRotation(0);
 tft.fillScreen(GC9A01A_BLUE);
 connect_to_wifi();

 // Optionally set custom scopes the available scopes are listed below
 // sp.set_scopes("user-read-playback-state user-modify-playback-state");

 sp.begin();
//  while (!sp.is_auth()) {
//      sp.handle_client(); // Required for receiving the authorization code
//  }

//  Serial.printf("Authenticated! Refresh token: %s\n", sp.get_user_tokens().refresh_token);

}

 void loop() {

if (skipRequested) {
        Serial.println("Bouton SKIP pressé");
        sp.skip_to_next();
        skipRequested = false; // On baisse le drapeau
        last_play = ""; // On force le rafraîchissement de l'écran au prochain tour
    }

    if (prevRequested) {
        sp.skip_to_previous();
        prevRequested = false;
        last_play = "";
    }

    if (pauseRequested) {
        // Optionnel : tu peux alterner entre pause et play ici
        sp.pause_playback(); 
        pauseRequested = false;
    }
String current_play=sp.current_track_name();
Serial.println("Get name");

if (current_play!=last_play){

Serial.println("New Track:update data");

last_play=current_play;

String artists= sp.current_artist_names();
String url_image=sp.get_current_album_image_url(0);

Serial.println(current_play);
Serial.println(artists);
Serial.println(url_image);

tft.fillScreen(GC9A01A_BLUE);
tft.setCursor(20,100);
tft.setTextColor(GC9A01A_WHITE); // Ne pas oublier la couleur du texte
tft.setTextSize(2);
tft.print(current_play);
tft.setCursor(20,150);
tft.print(artists);

//TODO load image get_current_album_image_url(int image_size_idx);

//TODO if bouton skip_to_previous();

//TODO if bouton2 skip_to_next();

//TODO if bouton3 pause_playback();

}

delay(5000);

} 

void connect_to_wifi() {
 WiFi.begin(SSID, PASSWORD);
 Serial.print("Connecting to WiFi...");
 while (WiFi.status() != WL_CONNECTED) {
     delay(1000);
     Serial.print(".");
 }
 Serial.println("\nConnected to WiFi!");
}