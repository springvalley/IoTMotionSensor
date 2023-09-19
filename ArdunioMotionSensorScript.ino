#include <SPI.h>
#include <WiFiS3.h>

//Disse må endres til nettverket man skal koble til evt bruke secrets.h fila som Gustav har. 
char ssid[] = "";
char pass[] = "";

//WiFi statusen til Arduinoen
int status = WL_IDLE_STATUS;


//Denne må endres til IPen til serveren, hvis du bruker xampp er det den samme som pcen din som du finner ved å bruke IPconfig i kommandolinjen.
char server[] = ""; 

//Klienten vi kobler til serveren med.
WiFiClient client;


//Variabler til sensoren.
int pirPin = 7;  //the digital pin connected to the PIR sensor's output
int ledPin = 5;  //the digital pin connected to the LED output
int Buzzer = 6;  //the digital pin connected to the BUZZER output

//Kalibreringstid i sekunder for sensoren, denne må visst være mellom 30 og 60 for optimal sensor sensorering. 
int calibrationTime = 5;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//Dette er hvor lenge det må være stille foran sensoren før den avjør at en bevegelse har avsluttet.
long unsigned int pause = 5000;

//Dette er for å sikre at pausen fungerer. 
boolean lockLow = true;
boolean takeLowTime;


//Siden sensoren hele tiden looper må vi ha et variabel for å vite om vi har sett noe bevegelse så vi kan få riktige timestamps. 
boolean highWritten = false;

//Når bevegelsen startet og når den sluttet. 
int motionStart = 0;
int motionEnd = 0;

void setup() {
  //Initialiserer Serial monitoren på Arduinoen
  Serial.begin(9600);


  //Pin oppsettet for pinnene på ardunioen
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(pirPin, LOW);


  //Gir sensoren tid til å kalibrere seg
  Serial.print("calibrating sensor ");
  for (int i = 0; i < calibrationTime; i++) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);

  //Prøver å koble til WiFi nettverket. 
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  //Hvis vi kommer så langt har vi koblet til WiFi nettverket så vi printer dette til Serial Monitoren. 
  Serial.print("You're connected to the network");
}


void loop() {

  //Les sensordata, hvis vi leser bevegelse eller "HIGH" så gjør vi følgende.
  if (digitalRead(pirPin) == HIGH) {
    //Skrur på lampa
    digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
    //Skrur på buzzeren
    tone(Buzzer,500);
    if (lockLow) {
      //Her sikrer vi at vi venter til vi har hatt en fase uten bevegelse før vi begynner å registrere en ny bevegelse
      lockLow = false;
      //Hvis vi har startet en ny bevegelse og ikke skrevet noe (highWritten == false) så lagrer vi tiden bevegelsen begynte på.
      if(highWritten == false){
      motionStart = int(millis() / 1000);
      highWritten = true;
      Serial.println(motionStart);
      }
      //Printer informasjon til serial monitor
      Serial.println("---");
      Serial.print("motion detected at ");
      Serial.print(millis() / 1000);
      Serial.println(" sec");
      delay(50);
    }
    takeLowTime = true;
  }


  //Les sensordata, hvis bevegelse stopper eller "LOW" så gjør vi følgende.
  if (digitalRead(pirPin) == LOW) {
    //1. Skru av lampa
    digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state
    //2. Skru av buzzeren. 
    noTone(Buzzer);

    //Lagrer tiden mellom bevegelsene
    if (takeLowTime) {
      lowIn = millis();     //save the time of the transition from high to LOW
      takeLowTime = false;  //make sure this is only done at the start of a LOW phase
    }
    //Hvis tiden det har vært ingen bevegelse er større en pausetiden vi har satt over (5 sekunder) så gjør vi følgende
    if (!lockLow && millis() - lowIn > pause) {
      //lockLow sørger for at denne koden blir bare kjørt i 1 loop. 
      lockLow = true;
      //finner tiden når bevegelsen stoppet. 
      motionEnd = (millis() - pause) / 1000;
      
      //Skriver debug informasjon til Serial monitor
      Serial.print("motion ended at ");  //output
      Serial.print((millis() - pause) / 1000);
      Serial.println(" sec");

      //Regner ut hvor lenge bevegelsen varte. 
      int motionDuration = motionEnd - motionStart;
      Serial.println(motionDuration);
      //Skriver denne informasjonen til databasen via en POST request. 
      writeDataToDB(motionDuration);

      //Gjør oss klare for å registrere en ny bevegelse. 
      highWritten = false;
      delay(50);
    }
  }
  delay(200);
}


//Denne funksjonen har ansvaret for å koble til databasen og skrive data til den via en POST request.
void writeDataToDB(int motionDuration) {
  //Vi kobler til serveren
  if (client.connect(server, 80)) {
    //Dette er dataen vi skal skrive til databasen. time er navnet på POST variabelet og vi gjør om tiden til en streng så det kan sendes i en POST request. 
    String postData = "time=" + String(motionDuration);

    //Vi sender en HTTP POST request til serveren sånn at den kan sende det sensoren har lest til databasen.
    //En POST request må settes opp på en spesiell måte med en HEADER og en BODY. (Dette er IS-105 greier men hvis du googler POST request så er det godt forklart).
   
    //HTTTP POST Header informasjon
   
    client.println("POST /IS311/db_insert.php HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("User-Agent: Arduino/1.0");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length:");
    client.println(postData.length());
    client.println("Connection: close");
    client.println("");  // end HTTP header

    //HTTP POST  Body Informasjon
    client.println(postData);

    //Hvis vi fortsatt er koblet til serveren på slutten av denne funksjonen så kobler vi fra. 
    if (client.connected()) {
      client.stop();
    }
  //Debug informasjon for at vi har skrevet noe nytt til databasen på serveren
  Serial.println("new Recording!");
  } else {
    //Hvis vi ikke kan koble til serveren så skriver vi en feilmelding. 
    Serial.println("Connection failed");
  }
  delay(1000);
}