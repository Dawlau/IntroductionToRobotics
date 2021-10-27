const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

const int potRedPin = A2;
const int potGreenPin = A1;
const int potBluePin = A0;

const int minAnalogInput = 0;
const int maxAnalogInput = 1023;

const int minAnalogOutput = 0;
const int maxAnalogOutput = 255;

void setup() {

  // set mode for declared pins
  
   pinMode(redPin, OUTPUT);
   pinMode(greenPin, OUTPUT);
   pinMode(bluePin, OUTPUT);

   pinMode(potRedPin, INPUT);
   pinMode(potGreenPin, INPUT);
   pinMode(potBluePin, INPUT);
}

void loop() {

  // read the resistance from the potentiometers (these values will be between 0 and 1023)
  unsigned int potRedResistance = analogRead(potRedPin);
  unsigned int potGreenResistance = analogRead(potGreenPin);
  unsigned int potBlueResistance = analogRead(potBluePin);

  // map them from 0-1023 to 0-255 for outputing
  unsigned int redSignal = map(potRedResistance, minAnalogInput, maxAnalogInput, minAnalogOutput, maxAnalogOutput);
  unsigned int greenSignal = map(potGreenResistance, minAnalogInput, maxAnalogInput, minAnalogOutput, maxAnalogOutput);
  unsigned int blueSignal = map(potBlueResistance, minAnalogInput, maxAnalogInput, minAnalogOutput, maxAnalogOutput);

  // output signals to the led pins
  analogWrite(redPin, redSignal);
  analogWrite(greenPin, greenSignal);
  analogWrite(bluePin, blueSignal);
}
