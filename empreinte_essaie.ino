// Définitions des bibliothèques nécessaires aux composants
#include <Adafruit_Fingerprint.h>  
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Définitions des caractéristiques de l'éran d'affichage OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Définition des broches pour les LED
const int LED_SUCCES = 13;
const int LED_ECHEC = 4;


// Définition des broches de communication Série logicielle (SoftwareSerial)
// Broche 2 de l'Arduino connectée au TX du capteur (Fil Vert)
// Broche 3 de l'Arduino connectée au RX du capteur (Fil Jaune)
SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{
  // Initialisation des broches des LED en sortie
  pinMode(LED_SUCCES, OUTPUT);
  pinMode(LED_ECHEC, OUTPUT);
  
  // Par sécurité, on éteint les LED au démarrage
  digitalWrite(LED_SUCCES, LOW);
  digitalWrite(LED_ECHEC, LOW);

  Serial.begin(9600);
  while (!Serial); // Attente de l'ouverture du moniteur série
  delay(100);
  Serial.println("\n\nTest de démarrage du capteur d'empreintes AS608");

  // Initialisation du capteur à la vitesse par défaut (57600 bauds)
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Capteur d'empreintes détecté avec succès !");
  } else {
    Serial.println("Erreur : Impossible de trouver le capteur d'empreintes. Vérifie le câblage.");
    while (1) { delay(1); } // Bloque le programme si le capteur n'est pas trouvé
  }

  // Affiche le nombre d'empreintes actuellement stockées dans le module
  finger.getTemplateCount();
  Serial.print("Le capteur contient "); Serial.print(finger.templateCount); Serial.println(" empreintes enregistrées.");
  Serial.println("En attente d'un doigt valide...");

  Serial.begin(9600);

  // Initialisation de l'écran (adresse I2C 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur OLED");
    while (true);
  }

  display.clearDisplay();

  // Taille du texte
  display.setTextSize(2);

  // Couleur du texte
  display.setTextColor(SSD1306_WHITE);

  // Position du curseur
  display.setCursor(10, 20);

  // Texte à afficher
  display.println("Entrez votre empreinte");

  // Envoi à l'écran
  display.display();
}

void loop()                     
{
  int fingerprintID = getFingerprintIDez();
  
  // Si une empreinte a été analysée (-1 signifie qu'aucun doigt n'est posé)
  if (fingerprintID >= 0) {
    
    if (fingerprintID > 0) {
      // ÉTAPE 1 : L'empreinte correspond ! (ID trouvé)
      Serial.print("Empreinte valide trouvée ! ID correspondant : ");
      Serial.println(fingerprintID);
      display.println("Accès autorisé");  // l'écran affiche accès autorisé
      
      digitalWrite(LED_SUCCES, HIGH); // Allume la LED 13
      digitalWrite(LED_ECHEC, LOW);   // Éteint la LED 4
      delay(3000);                    // Reste allumée pendant 3 secondes
      digitalWrite(LED_SUCCES, LOW);  // Éteint la LED
      
    } else if (fingerprintID == 0) {
      // ÉTAPE 2 : Un doigt a été posé mais il n'est pas reconnu
      Serial.println("Accès refusé : Empreinte inconnue.");  //l'écran affiche un accès refusé
      display.println("Accès refusé!");
      digitalWrite(LED_ECHEC, HIGH);   // Allume la LED 4
      digitalWrite(LED_SUCCES, LOW);   // Éteint la LED 13
      delay(3000);                     // Reste allumée pendant 3 secondes
      digitalWrite(LED_ECHEC, LOW);    // Éteint la LED
    }
  }
  
  delay(50); // Petit délai pour ne pas surcharger le processeur
}

// Fonction optimisée pour vérifier rapidement la présence d'un doigt
// Retourne : 
//   -1 si aucun doigt n'est détecté
//    0 si un doigt est détecté mais NON reconnu
//   >0 (l'ID du doigt) si le doigt est reconnu
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1; // Pas de doigt sur le capteur

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1; // Erreur de conversion de l'image

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    return finger.fingerID; // Succès : retourne l'ID trouvé
  } else if (p == FINGERPRINT_NOTFOUND) {
    return 0; // Doigt détecté mais inconnu au bataillon
  } else {
    return -1; // Autre type d'erreur système
  }
}
