#include <DHT.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Bounce2.h>

#define VELSERIALE 115200

#define DHTPIN 2  //Pin digitale sensore temperatura
#define TIPODHT DHT11  //Tipologia sensore temperatura

#define REVOLUTIONSENSORPIN 3  //Pin digitale sensore tracking IR

void(* resetFunc) (void) = 0; //funzione reset software per la board

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

//variabile menu
//scelta = 1 schermata principale
//scelta = 2 pausa
//scelta = 3 schermata info
int scelta = 0;
bool pausa = false;
bool start = false;
bool info = false;

//variabili cronometro
int secondi = 0;
int minuti = 0;
int ore = 0;
const unsigned long millisecInsecondi = 1000;
const unsigned long millisecInminuti = 60000;
const unsigned long millisecInore = 3600000;
unsigned long tempoInizioCronometro = 0;
unsigned long tempoInPausa = 0; //tempo in cui rimango in pausa

//pendenza terreno in percentuale
int pendenza = 0;

//variabili per ciclocomputer
//Circonferenza ruota bicicletta espressa in metri
const float circonferenzaRuota = 2.180;

//tempo corrente calcolato dall'inizio dell'allenamento
unsigned long tempoCorrente = 0;

//ultimo stato sensore IR
boolean ultimoStato = LOW;

//stato corrente sensore IR
boolean statoCorrente = LOW;

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
  Wire.begin(); //inizializza I2C per MPU6050
  Serial.begin(VELSERIALE); //inizializza seriale

  Serial.println("Inizializzazione modulo GY521");
  accelerometro.initialize();

  Serial.println("Inizializzazione sensore DHT11");
  sensTemperatura.begin();

  Serial.println("Inizializzazione sensore IR");
  pinMode(REVOLUTIONSENSORPIN, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  //leggo i pulsanti di controllo come input
  debStart.attach(pulsanteStart, INPUT);
  debPausa.attach(pulsantePausa, INPUT);
  debInfo.attach(pulsanteInfo, INPUT);
  debStart.interval(25);
  debPausa.interval(25);
  debInfo.interval(25);

  Serial.println("Inizializzazione display LCD");
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Avvio Completato");
  delay(700);
}

int calcoloPendenza()
{
  //valori accelerazione assi y e z rilevata da MPU6050
  int ay, az;
  
  ay = accelerometro.getAccelerationY();  //accelerazione su asse y
  az = accelerometro.getAccelerationZ();  //accelerazione su asse z
  
  float angoloRollio = atan2(ay, az); //calcolo arcotangente usando l'accelerazione degli assi y e z per trovare l'angolo di rollio (in radianti)
  int percentualeRollio = tan(angoloRollio) * 100; //calcolo tangente dell'angolo per trovare la pendenza in percentuale

  //pendenza massima 100% allora angolo di rollio 45 gradi
  //pendenza < 0 sto andando in discesa

  if(percentualeRollio > 100)
    percentualeRollio = 100;
  else if(percentualeRollio < -100)
    percentualeRollio = -100;

  return percentualeRollio;
}

//rilevamento dati sensore DHT11
float rilevaTemperatura()
{
  float temperatura = sensTemperatura.readTemperature();
  if(isnan(temperatura))
  {
    return -1;
  }
  return temperatura;
}

float rilevaUmidita()
{
  float umidita = sensTemperatura.readHumidity();
  if(isnan(umidita))
  {
    return -1;
  }
  return umidita;
}

void cronometro(unsigned long tempo)
{
  //se è passato 1 sec aggiorno il cronometro
  //variabile tempo output di millis()
  
  if (tempo >= 1000) {
    secondi = tempo / millisecInsecondi % 60;
    minuti = tempo / millisecInminuti % 60;
    ore = tempo / millisecInore % 24;
  }

  //mostro cronometro su display LCD
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

//schermata principale dove sono visibili distanza percorsa, velocità, cronometro e pendenza
void schermataPrincipale()
{
  tempoInizioCronometro = millis() - tempoInPausa;  //aggiorno il tempo del cronometro togliendo la pausa
  int pendenzaVecchia = pendenza;
  calcoloParametriAllenamento();
  lcd.setCursor(0, 0);
  lcd.print(distanza);
  lcd.print("Km");
  lcd.setCursor(7, 0);
  lcd.print(velocita);
  lcd.print("Km/h");
  cronometro(tempoInizioCronometro);
  pendenza = calcoloPendenza();
  if(pendenza != pendenzaVecchia)
    lcd.clear();
  lcd.setCursor(10, 1);
  lcd.print(pendenza);
  lcd.print("%");
}

//schermata informazioni aggiuntive dove sono visibili temperatura e umidita dell'ambiente
void schermataInfo()
{
  lcd.setCursor(0, 0);
  float temperatura = rilevaTemperatura();
  float umidita = rilevaUmidita();
  if(temperatura == -1)
  {
    Serial.println("Errore lettura temperatura");
    lcd.print("Errore lettura temperatura");
  }else if(umidita == -1)
  {
    Serial.println("Errore lettura umidita'");
    lcd.print("Errore lettura umidita'");
  }else
  {
    lcd.print("Temp:");
    lcd.print(temperatura);
    lcd.print(" gradi");
    lcd.setCursor(0, 1);
    lcd.print("Umidita:");
    lcd.print(umidita);
    lcd.print("%");
  }
}

void calcoloParametriAllenamento()
{
  tempoCorrente = millis(); //prendo il tempo corrente tramite funzione millis
  statoCorrente = debounce(ultimoStato, REVOLUTIONSENSORPIN);
  if(ultimoStato == LOW && statoCorrente == HIGH) //ho letto una rivoluzione della ruota
  {
    digitalWrite(LED_BUILTIN, HIGH);  //accendo led sulla board
    rivoluzioni++;
    if(rivoluzioni > 0)
      distanzaMetri = rivoluzioni * circonferenzaRuota; //calcolo la distanza in metri usando la ruota

    if(tempoInizioUltimaRivoluzione > 0)  //controllo che non sia la prima rivoluzione
    {
      tempoRivoluzione = tempoCorrente - tempoInizioUltimaRivoluzione;  //tempo di una rivoluzione della ruota
      velocita = (millisecInore / tempoRivoluzione) * circonferenzaRuota / 1000;  //calcolo velocita in km/h
    }

    tempoInizioUltimaRivoluzione = tempoCorrente; //aggiorno il tempo dell'ultima rivoluzione registrata
  }
  distanza = distanzaMetri / 1000;  //distanza in km
  ultimoStato = statoCorrente;  //aggiorno ultimo stato registrato dal sensore

  if(tempoCorrente >= (tempoInizioUltimaRivoluzione + 10000) && velocita > 0)
  {
    velocita = 0; //se non rilevo più il sensore dopo 10 secondi significa che sono fermo
  }
  digitalWrite(LED_BUILTIN, LOW); //spengo led sulla board
}

//funzione di debouncing per il sensore IR
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

  //rilevata pressione pulsante di start
  if(debStart.rose())
  {
    if(!pausa)  //controllo di non essere in pausa
    {
      lcd.clear();
      scelta = 1;
      start = !start;
    }

    if(!start)
    {
      schermataIniziale();
      resetFunc();
    }
  }

  //rilevata pressione pulsante di pausa
  if(debPausa.rose())
  {
    if(start)
    {
      scelta = 2;
      pausa = !pausa;
      schermataPausa();
      if(!pausa)  //uscito dalla schermata pausa torno su quella principale
      {
        scelta = 1;
        lcd.clear();
      }
    }
  }

  //rilevata pressione pulsante di info
  if(debInfo.rose())
  {
    if(start)
    {
      if(!pausa)
      {
        scelta = 3;
        lcd.clear();
        if(info)
        {
          scelta = 1;
          lcd.clear();
        }
        info = !info;
      }
    }
  }

  switch(scelta)
  {
    case 1:
      schermataPrincipale();
      break;
    case 2:
      tempoInPausa = millis() - tempoInizioCronometro; //calcolo il tempo in pausa
      break;
    case 3:
      schermataInfo();
      break;
  }
}
