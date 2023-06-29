#include <DHT.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <Wire.h>


#define DHTPIN 2  //Pin sensore temperatura
#define TIPODHT DHT11  //Tipologia sensore temperatura


//oggetto che controlla il modulo GY-521
MPU6050 accelerometro;

//oggetto che controlla il senosore di temperatura e umidita'
DHT sensTemperatura(DHTPIN, TIPODHT);


void inizializzazioneMCU()
{
  Wire.begin();
  Serial.begin(115200);

  Serial.println("Inizializzazione modulo GY521");
  accelerometro.initialize();

  Serial.println("Inizializzazione sensore DHT11");
  sensTemperatura.begin();

  //Serial.println("Inizializzazione sensore Hall");
  
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
