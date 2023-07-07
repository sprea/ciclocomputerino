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
  delay(700);
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

void schermataIniziale()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inizia il giro");
  lcd.setCursor(0, 1);
  lcd.print("premendo start!");
}

void schermataPrincipale()
{
  lcd.clear();
  lcd.print("Principale");
}

void schermataPausa()
{
  lcd.clear();
  lcd.print("PAUSA");
}

void schermataInfo()
{
  lcd.clear();
  lcd.print("Schermata Info");
}


void setup()
{
  inizializzazioneMCU();
  schermataIniziale();
}

int scelta = 0;
bool start = false;
bool pausa = false;
bool info = false;

//se scelta = 0 allora schermata iniziale
//scelta = 1 schermata principale
//scelta = 2 pausa
//scelta = 3 schermata info

void sceltaMenu(int scelta)
{
  switch (scelta)
  {
    case 0:
      schermataIniziale();
      Serial.println("Iniziale");
      break;
    case 1:
      schermataPrincipale();
      Serial.println("Principale");
      break;
    case 2:
      schermataPausa();
      Serial.println("Pausa");
      break;
    case 3:
      schermataInfo();
      Serial.println("Info");
      break;
  }
}

void loop()
{
  int statopulsanteStart = digitalRead(pulsanteStart);
  int statopulsantePausa = digitalRead(pulsantePausa);
  int statopulsanteInfo = digitalRead(pulsanteInfo);

  int sceltaPrecedente = scelta;
  if(statopulsanteStart == HIGH && start == false)
  {
    scelta = 1;
    start = true;
  }else if(statopulsanteStart == HIGH && start == true)
  {
    scelta = 0;
    start = false;
    pausa = false;
    info = false;
  }

  if(statopulsantePausa == HIGH && start == true && pausa == false)
  {
    scelta = 2;
    pausa = true;
  }else if(statopulsantePausa == HIGH && start == true && pausa == true)
  {
    scelta = 1;
    pausa = false;
  }

  if(statopulsanteInfo == HIGH && start == true && info == false && pausa == false)
  {
    scelta = 3;
    info = true;
  }else if(statopulsanteInfo == HIGH && start == true && info == true && pausa == false)
  {
    scelta = 1;
    info = false;
  }

  if(sceltaPrecedente != scelta)
  {
      sceltaMenu(scelta);
      delay(500);
  }
}
