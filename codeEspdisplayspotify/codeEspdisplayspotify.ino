#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyEsp32.h"
#include <HTTPClient.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>
#include <JPEGDEC.h> 


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


Spotify sp(CLIENT_ID, CLIENT_SECRET,REFRESH_TOKEN);//remove refresh token the first time
String last_play;

JPEGDEC jpeg;
volatile bool skipRequested = false;
volatile bool prevRequested = false;
volatile bool pauseRequested = false;


void IRAM_ATTR int_skip() { skipRequested = true; }
void IRAM_ATTR int_prev() { prevRequested = true; }
void IRAM_ATTR int_pause() { pauseRequested = true; }

void setup() {
 Serial.begin(115200);
 Serial.println("Start");

 pinMode(SKIP_PIN,INPUT_PULLUP);
  pinMode(PREV_PIN,INPUT_PULLUP);
 pinMode(PAUSE_PIN,INPUT_PULLUP);

  attachInterrupt(SKIP_PIN, int_skip, FALLING);
  attachInterrupt(PREV_PIN, int_prev, FALLING);
  attachInterrupt(PAUSE_PIN, int_pause, FALLING);

 SPI.begin(SCL_PIN, -1, SDA_PIN, CS_PIN); 

 tft.begin();
 tft.setRotation(0);
 tft.fillScreen(GC9A01A_BLACK);
 tft.setTextSize(1);
 tft.setCursor(20,120);
 tft.print("Start");
 delay(2000);
 tft.fillScreen(GC9A01A_BLACK);
 tft.setCursor(20,120);
 tft.print("Wifi connection");
 connect_to_wifi();
 tft.fillScreen(GC9A01A_BLACK);
 tft.setCursor(20,120);
 tft.print("Connection successful");
 delay(2000);

 sp.begin();
//Uncomment the first time
//  while (!sp.is_auth()) {
//      sp.handle_client(); // Required for receiving the authorization code
//  }

//  Serial.printf("Authenticated! Refresh token: %s\n", sp.get_user_tokens().refresh_token);

}

 void loop() {
for (int i=0; i<=10; i++) {

if (skipRequested) {
        Serial.println("Bouton SKIP pressé");
        sp.skip_to_next();
        skipRequested = false; 
        last_play = ""; 
        i=0;
        tft.fillScreen(GC9A01A_BLACK);
        tft.setCursor(20,120);
        tft.print("Loading");
    }

    if (prevRequested) {
        sp.skip_to_previous();
        prevRequested = false;
        last_play = "";
        i=0;
        tft.fillScreen(GC9A01A_BLACK);
        tft.setCursor(20,120);
        tft.print("Loading");

    }

    if (pauseRequested) {
        if (sp.is_playing()){
            Serial.println("Pause");
            sp.pause_playback(); 
        }else{
            Serial.println("Play");
            sp.start_a_users_playback();
        }
        pauseRequested = false;
    }
delay(300);
}

String current_play=sp.current_track_name();
Serial.println("Get name");

if (current_play!=last_play){

Serial.println("New Track:update data");

last_play=current_play;

String artists= sp.current_artist_names();
String url_image=sp.get_current_album_image_url(1);

Serial.println(current_play);
Serial.println(artists);
Serial.println(url_image);

tft.fillScreen(GC9A01A_BLACK);

ShowIMG(url_image);

int yStrip = 160; 

tft.fillRect(0, yStrip, 240, 55, GC9A01A_BLACK);
tft.setTextColor(GC9A01A_WHITE); 
tft.setTextSize(1);
tft.setCursor(35, yStrip + 12); 
tft.print(removeSpecial(current_play));
tft.setCursor(35, yStrip + 32);
tft.print(removeSpecial(artists));
}



} 

String removeSpecial(String str) {
  str.replace("é", "e");
  str.replace("è", "e");
  str.replace("ê", "e");
  str.replace("à", "a");
  str.replace("ä", "a");
  str.replace("ç", "c");
  str.replace("ô", "o");

  int par = str.indexOf('(');
  str.remove(par,256);
  int hyphen = str.indexOf('-');
  str.remove(hyphen,256);
  return str;
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



int JPEGDraw(JPEGDRAW *pDraw) {
  tft.drawRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}
void ShowIMG(String url) {
  HTTPClient http;
  
  Serial.println("\n[HTTP] Connexion au serveur...");
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int tailleFichier = http.getSize();
    Serial.printf("[HTTP] Taille annoncée par le serveur : %d octets\n", tailleFichier);

    if (tailleFichier <= 0) {
      Serial.println("[Erreur] Le serveur n'a pas renvoyé une taille de fichier valide.");
      http.end();
      return;
    }

    uint8_t * imageBuffer = (uint8_t *) malloc(tailleFichier);
    if (!imageBuffer) {
      Serial.println("[Erreur] Pas assez de mémoire RAM pour stocker cette image.");
      http.end();
      return;
    }

    WiFiClient * stream = http.getStreamPtr();
    int totalRead = 0;
    unsigned long timeout = millis();

    while (totalRead < tailleFichier && (millis() - timeout < 5000)) {
      if (stream->available()) {
        int read = stream->readBytes(&imageBuffer[totalRead], tailleFichier - totalRead);
        totalRead += read;
        timeout = millis(); 
      }
      delay(10);
    }

    Serial.printf("[HTTP] Octets réellement téléchargés : %d / %d\n", totalRead, tailleFichier);

    if (totalRead == tailleFichier) {
      if (jpeg.openRAM(imageBuffer, tailleFichier, JPEGDraw)) {
        jpeg.setPixelType(RGB565_LITTLE_ENDIAN); 
        
        int xOffset = (240 - jpeg.getWidth()) / 2;
        int yOffset = (240 - jpeg.getHeight()) / 2;
        
        Serial.printf("[JPEG] Image valide trouvée ! Résolution : %dx%d\n", jpeg.getWidth(), jpeg.getHeight());
        Serial.println("[JPEG] Début du décodage...");
        int resultat = jpeg.decode(xOffset, yOffset, 0); 
        
        if (resultat == 1) {
          Serial.println("[Succès] L'image a été entièrement décodée et envoyée à l'écran !");
        } else {
          Serial.printf("[Erreur] Le décodage a échoué. Code erreur JPEGDEC : %d\n", jpeg.getLastError());
          Serial.println("-> Note : Si l'erreur est 3 ou 4, votre image est un JPEG 'Progressif'. Il faut un JPEG 'Standard/Baseline'.");
        }
        jpeg.close();
      } else {
        Serial.printf("[Erreur] openRAM a échoué. Le fichier n'est pas un JPEG valide. Code : %d\n", jpeg.getLastError());
      }
    } else {
      Serial.println("[Erreur] Téléchargement incomplet (Time out de la connexion).");
    }
    
    free(imageBuffer); // Libération de la mémoire
  } else {
    Serial.printf("[Erreur HTTP] Code de réponse du serveur : %d\n", httpCode);
  }
  http.end();
}