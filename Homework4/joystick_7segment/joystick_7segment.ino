#include <EEPROM.h>

const int pinSW = 3;
const int pinX = A0;
const int pinY = A1;

const unsigned long debounceInterval = 200; // debounce for joystick button

const int dataPin = 12;
const int latchPin = 11;
const int clockPin = 10;

const int segD1 = 7;
const int segD2 = 6;
const int segD3 = 5;
const int segD4 = 4;

const int maxAnalogRead = 1023;

int displayDigits[] = {
  segD1, segD2, segD3, segD4
};

const int displayCount = 4;

const byte byteMask = 0xFF; // B11111111
const int byteLength = 8;

int digitArray[16] = {
  //A B C D E F G DP
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B11101110, // A
  B00111110, // b
  B10011100, // C
  B01111010, // d
  B10011110, // E
  B10001110  // F
};

const int numberSize = 4;
int number[numberSize]; // array of digits

int digitIndex; // which digit on the 7-segment display is selected
bool dpState;

const int dpFlickeringInterval = 500;
unsigned long lastDpFlick = 0;

volatile bool swState = HIGH;

int xValue = 0;
int yValue = 0;

// jotstick thresholds
int minThreshold = 300;
int maxThreshold = 800;

bool joyMoved = false;


void setup() {

  pinMode(pinSW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSW), changeSwState, FALLING); // set interrupt for joystick button

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  for (int i = 0; i < displayCount; i++) {

    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  setCurrentNumber(readFromEEPROM()); // initialize the number from EEPROM memory

  Serial.begin(9600);
}

void changeSwState() {

  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  if (interruptTime - lastInterruptTime >= debounceInterval) {

    swState = !swState;
  }

  lastInterruptTime = interruptTime;
}

// write byte to shift registry
void writeReg(int digit) {

  digitalWrite(latchPin, LOW);

  shiftOut(dataPin, clockPin, MSBFIRST, digit);

  digitalWrite(latchPin, HIGH);
}

void showDigit(int displayNumber) {

  for (int i = 0; i < 4; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }

  digitalWrite(displayDigits[displayNumber], LOW);
}

// write to current number to the 7-segment display
void writeNumber() {

  for (int i = 0; i < numberSize; i++) {

    

    if (i == digitIndex) {                       // if this digit is the selected one
      writeReg(digitArray[number[i]] ^ dpState); // add the decimal point (set/unset the last bit of the number from digitArray)
    }
    else {
      writeReg(digitArray[number[i]]);
    }

    showDigit(displayCount - i - 1); // map index to the corresponding display

    delay(5);
  }
}

int changeDigit() {

  xValue = maxAnalogRead - analogRead(pinX); // invert analog input

  // On Ox axis, if the value is lower than a chosen min threshold, then
  // decrease by 1 the digit value.

  int newDigit = number[digitIndex];

  if (xValue < minThreshold && joyMoved == false) {
    if (newDigit > 0) {
      newDigit--;
    }
    else {
      newDigit = 9;
    }
    joyMoved = true;
  }

  // On Ox axis, if the value is bigger than a chosen max threshold, then
  // increase by 1 the digit value

  if (xValue > maxThreshold && joyMoved == false) {
    if (newDigit < 9) {
      newDigit++;
    }
    else {
      newDigit = 0;
    }
    joyMoved = true;
  }

  if (xValue >= minThreshold && xValue <= maxThreshold) {
    joyMoved = false;
  }

  number[digitIndex] = newDigit; // update the digit with the new value

}

void cycleDigits() {

  yValue = maxAnalogRead - analogRead(pinY);

  // On Oy axis, if the value is lower than a chosen min threshold, then
  // decrease by 1 the digit index.


  if (yValue < minThreshold && joyMoved == false) {
    if (digitIndex > 0) {
      digitIndex--;
    }
    else {
      digitIndex = numberSize - 1;
    }
    joyMoved = true;
  }

  // On Oy axis, if the value is bigger than a chosen max threshold, then
  // increase by 1 the digit value

  if (yValue > maxThreshold && joyMoved == false) {
    if (digitIndex + 1 < numberSize) {
      digitIndex++;
    }
    else {
      digitIndex = 0;
    }
    joyMoved = true;
  }

  if (yValue >= minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }
}

int getCurrentNumber() {

  int answer = 0;

  for (int i = 0; i < numberSize; i++) {

    answer = answer * 10 + number[i];
  }

  return answer;
}

void setCurrentNumber(int currentNumber) {

  for (int i = numberSize - 1; i >= 0; i--) {

    number[i] = currentNumber % 10;
    currentNumber /= 10;
  }
}

void writeToEEPROM(int currentNumber) {

  // write the 2 least significant bytes of the currentNumber to the EEPROM
  // Note: 2 bytes are enough because the max value for currentNumber is 9999 which requires 14 bits

  byte firstByte = currentNumber & byteMask;
  byte secondByte = (currentNumber >> byteLength) & byteMask;

  // write in BIG Endian style
  EEPROM.update(0, secondByte);
  EEPROM.update(1, firstByte);
}

int readFromEEPROM() {

  // read the 2 bytes and combine them into the number

  int answer = 0;

  byte firstByte = EEPROM.read(1);
  byte secondByte = EEPROM.read(0);

  answer = secondByte;

  return (answer << byteLength) | firstByte;
}

void loop() {

  writeNumber(); // display the current number

  writeToEEPROM(getCurrentNumber()); // every frame save the number to EEPROM in case there is a power loss

  if (swState == HIGH) {
    cycleDigits();

    if (millis() - lastDpFlick >= dpFlickeringInterval) {

      dpState = !dpState;
      lastDpFlick = millis();
    }
  }
  else {
    changeDigit();
    dpState = HIGH;
  }
}
