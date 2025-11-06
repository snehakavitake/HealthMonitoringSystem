#include <Wire.h> //enable i2c communication
#include <LiquidCrystal_I2C.h> //to operate lcd
#include <DHT.h> //library for temp sensor

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  //initialize lcd 16x2 at add 0x27

// Pulse sensor setup
int pulsePin = A0; //pulse sensor connected to analog pin A0
int blinkPin = 13; //led will blink when heartbeat detected

// DHT11 sensor setup    
#define DHTPIN 2       // DHT11 data pin connected to D2
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

// Pulse sensor variables 
volatile int BPM; 
volatile int Signal; 
volatile int IBI = 600; 
volatile boolean Pulse = false; 
volatile boolean QS = false;

static boolean serialVisual = true;  

volatile int rate[10]; 
volatile unsigned long sampleCounter = 0; 
volatile unsigned long lastBeatTime = 0; 
volatile int P = 512; 
volatile int T = 512; 
volatile int thresh = 525; 
volatile int amp = 100; 
volatile boolean firstBeat = true; 
volatile boolean secondBeat = false;

unsigned long lastDisplayChange = 0;
bool showHeartbeat = true;

void setup() { 
  pinMode(blinkPin, OUTPUT);
  Serial.begin(115200);
  interruptSetup();

  lcd.init();
  lcd.backlight();

  dht.begin();
}

void loop() {  
  if (QS == true) { 
    serialOutputWhenBeatHappens(); 
    QS = false;
  }

  // Change LCD display every 2.5 seconds
  if (millis() - lastDisplayChange > 2500) {
    lcd.clear();
    
    if (showHeartbeat) {
      lcd.setCursor(0,0);
      lcd.print("Heartbeat: ");
      lcd.print(BPM);
      lcd.print(" BPM");
    } else {
      float temp = dht.readTemperature();
      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.print(temp);
      lcd.print(" C");
    }

    showHeartbeat = !showHeartbeat;
    lastDisplayChange = millis();
  }

  delay(20);
}

void interruptSetup() { 
  TCCR2A = 0x02;
  TCCR2B = 0x06;
  OCR2A = 0X7C;
  TIMSK2 = 0x02;
  sei(); 
}
/*void serialOutputWhenBeatHappens() {
  float temp = dht.readTemperature();

  // Send data in JSON format
  Serial.print("{\"bpm\":");
  Serial.print(BPM);
  Serial.print(",\"temp\":");
  Serial.print(temp);
  Serial.println("}");
}*/
void serialOutputWhenBeatHappens() {
  float temp = dht.readTemperature();

  // --- Spike Removal Logic ---
  // Ignore BPM if it's too sudden compared to last reading
  if (abs(BPM - lastBPM) > 25) {  
    // If sudden jump > 25 BPM, assume it's noise → keep previous BPM
    BPM = lastBPM;
  }

  // --- Clamp BPM to realistic human range ---
  if (BPM < 50 || BPM > 130) {  
    BPM = lastBPM; // ignore unrealistic values
  }

  // --- Smooth the BPM using averaging ---
  stableBPM = (stableBPM * 0.7) + (BPM * 0.3);  
  lastBPM = stableBPM;

  // Send data in JSON format
  Serial.print("{\"bpm\":");
  Serial.print(stableBPM);
  Serial.print(",\"temp\":");
  Serial.print(temp);
  Serial.println("}");
}

/*void serialOutputWhenBeatHappens() { 
  
  if (serialVisual) {
    Serial.print(" Heart-Beat Found ");
    Serial.print("BPM: ");
    Serial.println(BPM);
  }
  else {
    sendDataToSerial('B', BPM);
    sendDataToSerial('Q', IBI);
  }
}*/

void sendDataToSerial(char symbol, int data) {
  Serial.print(symbol);
  Serial.println(data);
}

ISR(TIMER2_COMPA_vect) {
  cli();

  Signal = analogRead(pulsePin);
  sampleCounter += 2; 
  int N = sampleCounter - lastBeatTime;

  if (Signal < thresh && N > (IBI / 5) * 3) {
    if (Signal < T) { T = Signal; }
  }

  if (Signal > thresh && Signal > P) {
    P = Signal;
  }

  if (N > 250) {
    if ((Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3)) {
      Pulse = true;
      digitalWrite(blinkPin, HIGH);
      IBI = sampleCounter - lastBeatTime;
      lastBeatTime = sampleCounter;

      if (secondBeat) {
        secondBeat = false;
        for (int i = 0; i <= 9; i++) rate[i] = IBI;
      }

      if (firstBeat) {
        firstBeat = false;
        secondBeat = true;
        sei();
        return;
      }

      word runningTotal = 0;
      for (int i = 0; i <= 8; i++) {
        rate[i] = rate[i + 1];
        runningTotal += rate[i];
      }
      rate[9] = IBI;
      runningTotal += rate[9];
      runningTotal /= 10;
      BPM = 60000 / runningTotal;
      QS = true;
    }
  }

  if (Signal < thresh && Pulse == true) {
    digitalWrite(blinkPin, LOW);
    Pulse = false;
    amp = P - T;
    thresh = amp / 2 + T;
    P = thresh;
    T = thresh;
  }

  if (N > 2500) {
    thresh = 512;
    P = 512;
    T = 512;
    lastBeatTime = sampleCounter;
    firstBeat = true;
    secondBeat = false;
  }

  sei();
}
