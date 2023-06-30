#include <DHT.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <Wire.h>

#define VELSERIALE 115200

#define DHTPIN 2  //Pin digitale sensore temperatura
#define TIPODHT DHT11  //Tipologia sensore temperatura

#define HALLPIN 4  //Pin digitale sensore HALL

//Pin display LCD
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;

//Pin pulsanti
const int pulsanteStart = 3, pulsantePausa = 5, pulsanteInfo = 6;

//oggetto che controlla il modulo GY-521
MPU6050 accelerometro;

//oggetto che controlla il senosore di temperatura e umidita'
DHT sensTemperatura(DHTPIN, TIPODHT);

//Inizialiazzazione display LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Circonferenza ruota bicicletta espressa in metri
float circonferenzaRuota = 2.180;

int contatoreLettureHall = 0;
typedef struct tempoAllenamento
{
  int ore;
  int minuti;
  int secondi;
}tempoAllenamento;
unsigned long durata = 0; //variabile che contiene l'output di millis
int velocita = 0;  //velocita in metri/secondo
double distanzaPercorsa = 0;
double velocitaMedia = 0;


void inizializzazioneMCU()
{
  Wire.begin();
  Serial.begin(VELSERIALE);

  Serial.println("Inizializzazione modulo GY521");
  accelerometro.initialize();

  Serial.println("Inizializzazione sensore DHT11");
  sensTemperatura.begin();

  Serial.println("Inizializzazione sensore Hall");
  pinMode(HALLPIN, INPUT);

  //leggo i pulsanti di controllo come input
  pinMode(pulsanteStart, INPUT);
  pinMode(pulsantePausa, INPUT);
  pinMode(pulsanteInfo, INPUT);

  Serial.println("Inizializzazione display LCD");
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Avvio Completato");
}

void calcoloparametriAllenamento()
{
  int valoreHall = digitalRead(HALLPIN);

  Serial.println(valoreHall);

  if(valoreHall == LOW)
    contatoreLettureHall++;

  durata = millis() / 1000; //converto tempo trascorso dall'inizio dello sketch in secondi

  //calcolo velocità
  velocita = (contatoreLettureHall * circonferenzaRuota) / durata;

  distanzaPercorsa = contatoreLettureHall * circonferenzaRuota;

  velocitaMedia = distanzaPercorsa / durata;
}

void primaSchermata()
{
  calcoloparametriAllenamento();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(distanzaPercorsa);
  lcd.print("Km");
  lcd.setCursor(8, 0);
  int pendenza = calcoloPendenza();
  lcd.print(pendenza);
  lcd.print("%");
  lcd.setCursor(0, 1);
  int velocitaKmOrari = velocita * 3.6;
  lcd.print(velocita);
  lcd.print("Km/h");
  lcd.setCursor(8, 1);
  float temperatura = rilevaTemperatura();
  lcd.print(temperatura);
  lcd.print("°C");
}

void schermataPausa()
{
  lcd.clear();
  lcd.print("PAUSA");
}

void secondaSchermata()
{
  lcd.clear();
  lcd.print("Schermata Info");
}

int calcoloPendenza()
{
  //valori assi x,y,z accelerometro
  int16_t ax, ay, az;

  ax = accelerometro.getAccelerationX();
  ay = accelerometro.getAccelerationY();
  az = accelerometro.getAccelerationZ();

  float angoloRollio = atan2(-ay, az) * 180 / PI;
  int percentualeRollio = tan(angoloRollio * PI / 180) * 100;

  if(percentualeRollio < 0)
    percentualeRollio = 0;

  return percentualeRollio;
}

float rilevaTemperatura()
{
  float temperatura = sensTemperatura.readTemperature();
  return temperatura;
}

float rilevaUmidita()
{
  float umidita = sensTemperatura.readHumidity();
  return umidita;
}

void setup()
{
  inizializzazioneMCU();
}

void loop()
{
  int statopulsanteStart = digitalRead(pulsanteStart);
  int statopulsantePausa = digitalRead(pulsantePausa);
  int statopulsanteInfo = digitalRead(pulsanteInfo);

  if(statopulsanteStart == HIGH)
  {
    primaSchermata();
  }

  if(statopulsantePausa == HIGH)
  {
    schermataPausa();
  }

  if(statopulsanteInfo == HIGH)
  {
    secondaSchermata();
  }
}
