const int buttonPin = 2;

const int pedestrianGreenLed = 3;
const int pedestrianRedLed = 5;

const int carRedLed = 11;
const int carYellowLed = 10;
const int carGreenLed = 9;

const int maxWriteAnalogValue = 255;

const int buzzerPin = 6;
const unsigned int buzzerFrequency = 3500;

const float initStartTime = -1.0f;

const float firstStateCountdown = 10000.0f; // first state lasts 10 seconds
const float secondStateCountdown = firstStateCountdown + 3000.0f;
const float thirdStateCountdown = secondStateCountdown + 10000.0f;
const float fourthStateCountdown = thirdStateCountdown + 5000.0f;

const unsigned long slowTickDuration = 200;
const unsigned long fastTickDuration = 100;

const float slowTickRate = 1000.0f;

// states of the LEDs
int pedestrianGreenLedState = LOW;
int pedestrianRedLedState = HIGH;

int carGreenLedState = HIGH;
int carYellowLedState = LOW;
int carRedLedState = LOW;

bool isRunning = false; // controls whether the traffic lights logic is running or not

float startTime = initStartTime;

float flickeringInterval = 100.0f;
float lastChange = 0.0f;

float lastTick = 0.0f;

bool startedThirdState = false; // checks whether I have enter the third stage or not
bool startedFourthState = false;

// for debounce
const unsigned int debounceDelay = 50;
unsigned int lastDebounceTime = 0;
bool lastButtonState = LOW;
bool buttonState = HIGH;
int previousButtonState = HIGH; 


void setup() {

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(pedestrianGreenLed, OUTPUT);
  pinMode(pedestrianRedLed, OUTPUT);
  pinMode(carRedLed, OUTPUT);
  pinMode(carYellowLed, OUTPUT);
  pinMode(carGreenLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  Serial.begin(9600);
}

void state2(float currentTime){

  carGreenLedState = LOW;
  carYellowLedState = HIGH;
  analogWrite(carGreenLed, carGreenLedState * maxWriteAnalogValue);
  analogWrite(carYellowLed, carYellowLedState * maxWriteAnalogValue);

  lastTick = currentTime; // start tick timer for buzzer from third stage
}

void state3(float currentTime){

   if(!startedThirdState){ // check if it is the first time entering first stage
     startedThirdState = true;
     lastTick = millis();
     tone(buzzerPin, buzzerFrequency, slowTickDuration);
   }
  
   carYellowLedState = LOW;
   carRedLedState = HIGH;
   pedestrianRedLedState = LOW;
   pedestrianGreenLedState = HIGH;
   
   analogWrite(carYellowLed, carYellowLedState * maxWriteAnalogValue); 
   analogWrite(carRedLed, carRedLedState * maxWriteAnalogValue);
   analogWrite(pedestrianRedLed, pedestrianRedLedState * maxWriteAnalogValue);
   analogWrite(pedestrianGreenLed, pedestrianGreenLedState* maxWriteAnalogValue);
  
   if(currentTime - lastTick > slowTickRate) {
      lastTick = currentTime;
      tone(buzzerPin, buzzerFrequency, slowTickDuration);
   }
}

void state4(float currentTime){

   if(!startedFourthState){
      startedFourthState = true;
      lastTick = millis();
      noTone(buzzerPin);
      tone(buzzerPin, buzzerFrequency, fastTickDuration);
   }

   if(currentTime - lastChange > flickeringInterval){ // flickering and sound
      
     lastChange = currentTime;
     pedestrianGreenLedState = !pedestrianGreenLedState;
     
     analogWrite(pedestrianGreenLed, pedestrianGreenLedState * maxWriteAnalogValue);
     tone(buzzerPin, buzzerFrequency, fastTickDuration);
   }
}

void reInit(){

   noTone(buzzerPin);

   startedThirdState = false;
   startedFourthState = false;

   pedestrianGreenLedState = LOW;
   pedestrianRedLedState = HIGH;

   carRedLedState = LOW;
   carGreenLedState = HIGH;

   analogWrite(carRedLed, carRedLedState * maxWriteAnalogValue);
   analogWrite(pedestrianGreenLed, pedestrianGreenLedState * maxWriteAnalogValue);
    
   isRunning = false;
   startTime = initStartTime;
   buttonState = HIGH;
}


void loop() {

  // debouncing for button state

  int reading = digitalRead(buttonPin);

  if(reading != lastButtonState) {
    
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > debounceDelay) {

    if(reading != buttonState) {

      buttonState = reading;
      if(buttonState == LOW) {
        isRunning = true;
      }
    }
  }

  // start running the traffic lights logic
  if(isRunning) {

    if(startTime == initStartTime){ // save the starting time of the traffic lights logic
      startTime = millis();
    }

    float currentTime = millis();
    float diffTime = currentTime - startTime; // how much time has passed since the beggining of the logic

    // for the first state just wait in the default state for 10 seconds
    if(diffTime >= firstStateCountdown){ // after the first stage
      if(diffTime < secondStateCountdown){ // enter stage 2
        
        state2(currentTime);
      }
      else if(diffTime < thirdStateCountdown){

        state3(currentTime);
      }
      else if(diffTime < fourthStateCountdown) {

        state4(currentTime);

      }
      else { // re-initialize relevant variables

        reInit();
      }
    }
  }
  else { // default lights
    
    analogWrite(pedestrianRedLed, pedestrianRedLedState * maxWriteAnalogValue);
    analogWrite(carGreenLed, carGreenLedState * maxWriteAnalogValue);
  }

  previousButtonState = reading; // for debouncing
}
