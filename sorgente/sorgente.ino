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

//variabili cronometro
int secondi = 0;
int minuti = 0;
int ore = 0;

//pendenza terreno in percentuale
int pendenza = 0;

//variabili per ciclocomputer
//Circonferenza ruota bicicletta espressa in metri
const float circonferenzaRuota = 2.180;

//tempo corrente calcolato dall'inizio dell'allenamento
unsigned long tempoCorrente = 0;

//ultimo stato sensore
boolean ultimoStatoHall = LOW;

//stato corrente sensore
boolean statoCorrenteHall = LOW;

//tempo inizio ultima rivoluzione ruota
unsigned long tempoInizioUltimaRivoluzione = 0;

//tempo di una rivoluzione
unsigned long tempoRivoluzione = 0;

//contatore rivoluzioni ruota
unsigned long rivoluzioni = 0;

//distanza in metri
float distanzaMetri = 0;

//distanza percorsa in km
float distanza = 0;

//velocita' espressa in km/h
float velocita = 0;

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

  pinMode(LED_BUILTIN, OUTPUT);

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
  else if(percentualeRollio > 100)
    percentualeRollio = 100;

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

void cronometro(unsigned long tempo)
{
  unsigned long millisecInsecondi = 1000;
  unsigned long millisecInminuti = 60000;
  unsigned long millisecInore = 3600000;

  if (tempo >= 1000) {
    secondi = tempo / millisecInsecondi % 60;
    minuti = tempo / millisecInminuti % 60;
    ore = tempo / millisecInore % 24;
  }

  lcd.setCursor(0, 1);
  if(ore < 10)
    lcd.print("0");
  lcd.print(ore);
  lcd.print(":");
  if(minuti < 10)
    lcd.print("0");
  lcd.print(minuti);
  lcd.print(":");
  if(secondi < 10)
    lcd.print("0");
  lcd.print(secondi);
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
  calcoloParametriAllenamento();
  lcd.setCursor(0, 0);
  lcd.print(distanza);
  lcd.print("Km");
  lcd.setCursor(7, 0);
  lcd.print(velocita);
  lcd.print("Km/h");
  unsigned long tempoTotale = millis();
  cronometro(tempoTotale);
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

void calcoloParametriAllenamento()
{
  tempoCorrente = millis();
  statoCorrenteHall = debounce(ultimoStatoHall, HALLPIN);
  if(ultimoStatoHall == LOW && statoCorrenteHall == HIGH)
  {
    rivoluzioni++;
    if(rivoluzioni > 0)
      distanzaMetri = rivoluzioni * circonferenzaRuota;

    if(tempoInizioUltimaRivoluzione > 0)
    {
      tempoRivoluzione = tempoCorrente - tempoInizioUltimaRivoluzione;
      velocita = (3600000 / tempoRivoluzione) * circonferenzaRuota / 1000;
    }

    tempoInizioUltimaRivoluzione = tempoCorrente;
  }
  distanza = distanzaMetri / 1000;  //distanza in km
  ultimoStatoHall = statoCorrenteHall;

  if(tempoCorrente >= (tempoInizioUltimaRivoluzione + 10000) && velocita > 0)
  {
    velocita = 0; //se non rilevo più il sensore dopo 10 secondi significa che sono fermo
  }
}

boolean debounce(boolean ultimo, int pin)
{
  boolean corrente = digitalRead(pin);
  if (ultimo != corrente) {
    delay(5);
    corrente = digitalRead(pin);
  }
  return corrente;
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
