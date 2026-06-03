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


// Create an instance of the Spotify class (optional: specify retry count)
Spotify sp(CLIENT_ID, CLIENT_SECRET,REFRESH_TOKEN);
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
 tft.setFont();
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
for (int i=0; i<=10; i++) {

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

// 1. On dessine la pochette d'album en premier plan
afficherImageDepuisURL(url_image);

// --- CONFIGURATION DU BANDEAU INFÉRIEUR ---
int ecranTaille = 240;       // Le GC9A01 fait 240x240 pixels
int hauteurBandeau = 55;     // Hauteur de la zone noire en bas
int yBandeau = 120; // Position Y de départ (185)

// 2. On dessine le bandeau noir opaque pour masquer le bas de l'image
tft.fillRect(0, yBandeau, ecranTaille, hauteurBandeau, GC9A01A_BLACK);

// 3. Configuration globale du texte (Blanc, Petite taille)
tft.setTextColor(GC9A01A_WHITE); 
tft.setTextSize(1); // Taille 1 pour un rendu fin, propre et discret

// --- AFFICHAGE DU TITRE ---
// Sur un écran rond, plus on descend, plus l'écran se rétrécit. 
// On commence donc un peu plus vers la droite (X=35) pour éviter que le texte soit coupé par le bord arrondi.
tft.setCursor(35, yBandeau + 12); 
tft.print(removeSpecial(current_play));

// --- AFFICHAGE DE L'ARTISTE ---
tft.setCursor(35, yBandeau + 32);
tft.print(removeSpecial(artists));
}



} 

String removeSpecial(String str) {
  str.replace("é", "e");
  str.replace("è", "e");
  str.replace("ê", "e");
  str.replace("à", "a");
  str.replace("ç", "c");
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


// --- Fonction de rappel (Callback) pour l'affichage ---
// Cette fonction est appelée automatiquement par JPEGDEC pour chaque bloc de l'image décodée.
int JPEGDraw(JPEGDRAW *pDraw) {
  // Envoi du bloc de pixels à la bibliothèque Adafruit_GFX
  tft.drawRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1; // Retourne 1 pour indiquer au décodeur de continuer
}
void afficherImageDepuisURL(String url) {
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

    // Allocation de la mémoire RAM
    uint8_t * imageBuffer = (uint8_t *) malloc(tailleFichier);
    if (!imageBuffer) {
      Serial.println("[Erreur] Pas assez de mémoire RAM pour stocker cette image.");
      http.end();
      return;
    }

    // Lecture sécurisée du flux (évite les coupures prématurées)
    WiFiClient * stream = http.getStreamPtr();
    int totalRead = 0;
    unsigned long timeout = millis();

    while (totalRead < tailleFichier && (millis() - timeout < 5000)) {
      if (stream->available()) {
        int read = stream->readBytes(&imageBuffer[totalRead], tailleFichier - totalRead);
        totalRead += read;
        timeout = millis(); // Reset du timeout à chaque bloc reçu
      }
      delay(10);
    }

    Serial.printf("[HTTP] Octets réellement téléchargés : %d / %d\n", totalRead, tailleFichier);

    if (totalRead == tailleFichier) {
      // Tentative d'ouverture de l'image
      if (jpeg.openRAM(imageBuffer, tailleFichier, JPEGDraw)) {
        jpeg.setPixelType(RGB565_LITTLE_ENDIAN); 
        
        int xOffset = (240 - jpeg.getWidth()) / 2;
        int yOffset = (240 - jpeg.getHeight()) / 2;
        
        Serial.printf("[JPEG] Image valide trouvée ! Résolution : %dx%d\n", jpeg.getWidth(), jpeg.getHeight());
        Serial.println("[JPEG] Début du décodage...");
        
        // Execution et VERIFICATION du décodage
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