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

  Serial.println("Inizializzazione display LCD");
  lcd.begin(16, 2);
  lcd.print("Avvio...");
}

void calcoloPendenza()
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

  Serial.print("Pendenza: ");
  Serial.print(percentualeRollio);
  Serial.println("%");

  delay(500);

  //STAMPA SU DISPLAY LCD
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

void rilevazioneTemp()
{
  float temperatura = rilevaTemperatura();
  float umidita = rilevaUmidita();

  Serial.println("Rilevazione temperatura e umidità");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C, Umidita': ");
  Serial.print(umidita);
  Serial.println(" %");

  delay(500);
  //STAMPA SU LCD
}

void setup()
{
  inizializzazioneMCU();
}

void loop()
{
  calcoloPendenza();
  rilevazioneTemp();
}
