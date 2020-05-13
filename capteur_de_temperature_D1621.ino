#include <Wire.h>

#define A0_DS1621  0
#define A1_DS1621  0
#define A2_DS1621  0
#define ADDRESS_DS1621  (0x48 | A2_DS1621<<2 | A1_DS1621<<1 | A0_DS1621)

#define ONESHOT 1   // bit 1SHOT=1 si conversion au coup par coup
#define POL     0   // bit POL, non utilisé ici
#define NVB     0   // bit NVB, non utilisé ici
#define TLF     0   // bit TLF, non utilisé ici
#define THF     0   // bit THF, non utilisé ici

/* Configuration du registre */
#define REGISTER_CONFIG   (THF<<6 | TLF<<5 | NVB<<4 | POL<<1 | ONESHOT)

#define DONE_MASK   0x80   // Masque pour bit DONE

/* Commandes du DS1621 */
#define READ_TEMPERATURE  0xAA
#define ACCESS_CONFIG     0xAC
#define START_CONVERT     0xEE
#define STOP_CONVERT      0x22

byte endConversion = 0;
byte temperatureMSB = 0;
byte temperatureLSB = 0;
float temperature;

void setup() {
  Serial.begin(9600);  // Initialisation Terminal Série
  Wire.begin();        // Initialisation I2C

  /* Configuration du DS1621 */
  Wire.beginTransmission(ADDRESS_DS1621);
  Wire.write(STOP_CONVERT);
  Wire.endTransmission();

  Wire.beginTransmission(ADDRESS_DS1621);
  Wire.write(ACCESS_CONFIG);   // Accès au registre de configuration
  Wire.write(REGISTER_CONFIG); // écriture dans le registre de configuration
  Wire.endTransmission();
}

void loop() {

  /* Lancement de la conversion */
  Wire.beginTransmission(ADDRESS_DS1621);
  Wire.write(START_CONVERT);
  Wire.endTransmission();

  /* Attendre la fin de la conversion */
  do {
    Wire.beginTransmission(ADDRESS_DS1621);
    Wire.write(ACCESS_CONFIG);
    Wire.endTransmission(false);  // Condition RESTART
    Wire.requestFrom(ADDRESS_DS1621, 1); // Un octet est requis
    if (1 <= Wire.available()) endConversion = Wire.read() & DONE_MASK;
  } while (!endConversion);

  /* Récupérer les deux octets de température */
  Wire.beginTransmission(ADDRESS_DS1621);
  Wire.write(READ_TEMPERATURE);
  Wire.endTransmission(false);  // Condition RESTART
  Wire.requestFrom(ADDRESS_DS1621, 2); // Deux octets sont requis
  if (2 <= Wire.available()) {
    temperatureMSB = Wire.read();  // Octet de poids fort
    temperatureLSB = Wire.read();  // Octet de poids faible
  }

  /* Faire les calculs et afficher la température en degrés Celcius */
  temperature = (float) temperatureMSB;
  if (temperatureLSB & 0x80) temperature += 0.5;  // 1/2 °C à prendre en compte
  if (temperatureMSB & 0x80) temperature -= 256;  // Température négative

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" degres Celcius");

  delay(5000);  // Attendre 5 s avant de recommencer

}
