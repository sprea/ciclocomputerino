#include <DHT.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Bounce2.h>

#define VELSERIALE 115200

#define DHTPIN 2  //Pin digitale sensore temperatura
#define TIPODHT DHT11  //Tipologia sensore temperatura

#define HALLPIN 3  //Pin digitale sensore HALL

//variabile menu
//scelta = 1 schermata principale
//scelta = 2 schermata info
int scelta = 0;
bool pausa = false;

//Pin display LCD
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;

//Pin pulsanti
const int pulsanteStart = 4, pulsantePausa = 5, pulsanteInfo = 6;

//oggetti libreria bounce per debouncing dei pulsanti
Bounce debStart = Bounce();
Bounce debPausa = Bounce();
Bounce debInfo = Bounce();

//oggetto che controlla il modulo GY-521
MPU6050 accelerometro;

//oggetto che controlla il senosore di temperatura e umidita'
DHT sensTemperatura(DHTPIN, TIPODHT);

//Inizialiazzazione display LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//variabili per ciclocomputer
//Circonferenza ruota bicicletta espressa in metri
float circonferenzaRuota = 2.180;

//contatore rivoluzioni ruota
unsigned long contatoreRivoluzione = 0;

//distanza percorsa in km
float distanza = 0;

//velocita' espressa in km/h
float velocita = 0;

int pendenza = 0;

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
  debStart.attach(pulsanteStart, INPUT);
  debPausa.attach(pulsantePausa, INPUT);
  debInfo.attach(pulsanteInfo, INPUT);
  debStart.interval(25);
  debPausa.interval(25);
  debInfo.interval(25);
//  pinMode(pulsanteStart, INPUT);
//  pinMode(pulsantePausa, INPUT);
//  pinMode(pulsanteInfo, INPUT);

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

void schermataPausa()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Registrazione");
  lcd.setCursor(4, 1);
  lcd.print("in Pausa");
}

void schermataPrincipale()
{
  int pendenzaVecchia = pendenza;
  lcd.setCursor(0, 0);
  lcd.print(distanza);
  lcd.print("Km");
  lcd.setCursor(8, 0);
  lcd.print(velocita);
  lcd.print("Km/h");
  lcd.setCursor(0, 1);
  unsigned long tempoTotale = millis() / 1000;
  lcd.print(tempoTotale / 3600);
  lcd.print(":");
  lcd.print((tempoTotale % 3600) / 60);
  lcd.print(":");
  lcd.print(tempoTotale % 60);
  pendenza = calcoloPendenza();
  if(pendenza != pendenzaVecchia)
    lcd.clear();
  lcd.setCursor(10, 1);
  lcd.print(pendenza);
  lcd.print("%");
}

void schermataInfo()
{
  lcd.setCursor(0, 0);
  float temperatura = rilevaTemperatura();
  float umidita = rilevaUmidita();
  lcd.print("Temp:");
  lcd.print(temperatura);
  lcd.print(" gradi");
  lcd.setCursor(0, 1);
  lcd.print("Umidita:");
  lcd.print(umidita);
  lcd.print("%");
}

void setup()
{
  inizializzazioneMCU();
  schermataIniziale();
}

void loop()
{
  debStart.update();
  debPausa.update();
  debInfo.update();

  if(debStart.fell())
  {
    if(!pausa)
    {
      lcd.clear();
      scelta = 1;
    }
  }

  if(debPausa.fell())
  {
    scelta = 2;
    pausa = !pausa;
    schermataPausa();
    if(pausa == false)
    {
      scelta = 1;
      lcd.clear();
    }
  }

  if(debInfo.fell())
  {
    if(!pausa)
    {
      scelta = 3;
      lcd.clear();
    }
  }

  switch(scelta)
  {
    case 1:
      schermataPrincipale();
      break;
    case 2: 
      break;
    case 3:
      schermataInfo();
      break;
  }
}
