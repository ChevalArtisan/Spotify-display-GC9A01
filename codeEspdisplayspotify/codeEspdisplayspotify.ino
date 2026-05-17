#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyEsp32.h"

#include "myAccess.h" // Contains  ssid password and API id and tokens


//USB CDC on boot enabled
//Flash mode DIO

// Adafruit_GC9A01A tft = Adafruit_GC9A01A(TFT_CS, TFT_DC, TFT_RST);


// Create an instance of the Spotify class (optional: specify retry count)
Spotify sp(CLIENT_ID, CLIENT_SECRET,REFRESH_TOKEN);
String current_play;

void setup() {
 Serial.begin(115200);
 pinMode(8, OUTPUT);
 digitalWrite(8, HIGH);

 connect_to_wifi();

 // Optionally set custom scopes the available scopes are listed below
 // sp.set_scopes("user-read-playback-state user-modify-playback-state");

 sp.begin();
 while (!sp.is_auth()) {
     sp.handle_client(); // Required for receiving the authorization code
 }

 Serial.printf("Authenticated! Refresh token: %s\n", sp.get_user_tokens().refresh_token);
 digitalWrite(8, HIGH);

}

void loop() {
 // Your code here
 current_play=sp.current_track_name();
 Serial.println(current_play);
 delay(2000);
}

void connect_to_wifi() {
 WiFi.begin(SSID, PASSWORD);
 Serial.print("Connecting to WiFi...");
 while (WiFi.status() != WL_CONNECTED) {
     delay(1000);
     Serial.print(".");
 }
 Serial.println("\nConnected to WiFi!");
 digitalWrite(8, LOW);
}