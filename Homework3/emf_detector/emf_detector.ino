const int buzzerPin = 11;
const int pinA = 2;
const int pinB = 3;
const int pinC = 4;
const int pinD = 5;
const int pinE = 6;
const int pinF = 7;
const int pinG = 8;
const int pinDP = 9;
const int antennaPin = A0;

const int segSize = 8;
const int noOfDigits = 10;
const int minDigit = 0;
const int noOfSamples = 300;
const int minAnalogRead = 0;
const int maxAnalogRead = 1023;

const int baseFrequency = 1000;
const float initTime = -1.0f;

const int minThreshold = 0;
const int maxThreshold = 200;

float lastTime = 0.0f;
float startTime = initTime;
int lastValue = 0;

const int delayTime = 200; // ms

int index = 0;
bool state = HIGH;

// store the pins in an array for easier access
int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

byte digitMatrix[noOfDigits][segSize - 1] = {
  // a  b  c  d  e  f  g
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

void displayNumber(byte digit) {

  for (int i = 0; i < segSize - 1; i++) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }

}

void setup() {

  pinMode(buzzerPin, OUTPUT);
  pinMode(antennaPin, INPUT);

  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }

  Serial.begin(9600);
}

// calculate the average electromagnetic field over 300 reads
unsigned long getAverageEMF(int antennaPin) {

  float average = 0.0f;
  for (int i = 0; i < noOfSamples; i++) {

    unsigned int reading = analogRead(antennaPin);
    average += reading;
  }

  unsigned long answer = average / noOfSamples;

  return answer;
}

// sets the frequency for the buzzer
void toneBuzzer(unsigned long value) {

  if (value == 0) {
    
    noTone(buzzerPin);
  }
  else {

    tone(buzzerPin, baseFrequency * value);
  }
}

void loop() {

  unsigned long currentEMF = getAverageEMF(antennaPin);

  // map the current electromagnetic field to a digit
  currentEMF = constrain(currentEMF, minThreshold, maxThreshold);
  currentEMF = map(currentEMF, minThreshold, maxThreshold, minDigit, noOfDigits - 1);

  // useful for avoiding flickering on the 7 segment display
  if (lastValue != currentEMF) { // if there are 2 consecutive different values

    if (startTime == initTime) {

      startTime = millis(); // start a timer
    }

    if (millis() - startTime >= delayTime) { // if the timer exceeded the delay time
      // print new value
      displayNumber(currentEMF);
      toneBuzzer(currentEMF);
      lastValue = currentEMF;
      startTime = initTime;
    }
    else {
      
      displayNumber(lastValue);
      toneBuzzer(lastValue);
    }
  }
}
