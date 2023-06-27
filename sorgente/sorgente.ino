#include <DHT.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//oggetto che controlla il modulo GY-521
MPU6050 accelerometro;

void inizializzazioneMCU()
{
  Wire.begin();
  Serial.begin(115200);

  Serial.println("Inizializzazione modulo GY521");
  accelerometro.initialize();

  //Serial.println("Inizializzazione sensore DHT11");


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

void setup()
{
  inizializzazioneMCU();
}

void loop()
{
  calcoloPendenza();
}
