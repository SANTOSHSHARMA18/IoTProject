// Add these includes if not already present
#include <Arduino.h>

// Declare global variables
boolean serialVisual = true;
int thresh = 512;
int T = 512;
int P = 512;
boolean secondBeat = false;
int rate[10];
int amp = 100;
boolean Pulse = false;
int IBI = 600; // Interval between beats; IBI stands for Inter-Beat Interval
int Signal;
int PulsePin = 0; // Pulse sensor purple wire connected to analog pin 0
int LED13 = 13; // The onboard Arduino LED
int BlinksPerMinute;
volatile int BPM; // Used to hold the pulse rate
volatile int QS = 0; // Quantified Self
unsigned long lastBeat = 0;
boolean firstBeat = true;

void setup() {
  pinMode(LED13, OUTPUT); // Pin 13 LED
  Serial.begin(115200); // Set up serial communication at 115200 bps

  interruptSetup(); // Sets up to read Pulse Sensor signal every 2mS
}

void loop() {
  serialOutput();

  if (QS == true) {
    serialOutputWhenBeatHappens();
    QS = false;
  }

  delay(20); // Adjust this delay if needed
}

void serialOutput() {
  if (serialVisual == true) {
    arduinoSerialMonitorVisual();
  } else {
    sendDataToSerial('S', Signal);
  }
}

void serialOutputWhenBeatHappens() {
  sendDataToSerial('B', BPM);
  sendDataToSerial('Q', QS);
}

void arduinoSerialMonitorVisual() {
  for (int i = 0; i < 9; i++) {
    if (rate[i] <= 25 && rate[i] > 0) {
      BlinksPerMinute = 60000 / rate[i];
      QS = true; // Detecting a beat
    }
  }
}

void sendDataToSerial(char symbol, int data) {
  Serial.print(symbol);
  Serial.println(data);
}

void interruptSetup() {
  // Initializes Timer2 to throw an interrupt every 2mS
  TCCR2A = 0x02;
  TCCR2B = 0x06;
  OCR2A = 0x7C;
  TIMSK2 = 0x02;
  sei(); // Enable interrupts
}

ISR(TIMER2_COMPA_vect) {
  cli(); // Disable interrupts when you are reading from sensors

  Signal = analogRead(PulsePin); // Read the PulseSensor's value
  QS = true; // A beat was detected

  // Calculate the time between beats in milliseconds
  IBI = millis() - lastBeat;

  if (secondBeat) {
    secondBeat = false;
    for (int i = 0; i <= 9; i++) {
      rate[i] = IBI;
    }
  }

  if (firstBeat) {
    firstBeat = false;
    secondBeat = true;
    sei(); // Enable interrupts when you are done reading from the sensor
    return;
  }

  int runningTotal = 0;

  for (int i = 0; i <= 8; i++) {
    rate[i] = rate[i + 1];
    runningTotal += rate[i];
  }

  rate[9] = IBI;
  runningTotal += rate[9];
  runningTotal /= 10;
  BPM = 60000 / runningTotal;

  // Quantified Self
  QS = true; // Set the Quantified Self flag (we detected a beat)
  sei(); // Enable interrupts when you are done reading from the sensor
  lastBeat = millis(); // Keep track of the time of the last beat
}
