/*

 Quantified Dog v0.1
 Matt Felsen
 Sensemakers Collaboration Studio
 Spring 2013
 
 */

// Set up accelerometer pins
const int accelerometerXPin = A0;
const int accelerometerYPin = A1;
const int accelerometerZPin = A2;

// Use analog pins as ground for accelerometer
const int ground = 9;
const int vcc = 8;

// Counter for which position of the array is being written to
int counter = 0;

// Timing settings
//#define SAMPLE_FREQUENCY 85
#define SAMPLE_FREQUENCY 10
#define SECONDS_TO_AVERAGE 30

// Frequency of sampling sensor
long interval = ceil(1000 / SAMPLE_FREQUENCY);
long prevMillis = 0;

// Sample storage
float accelerometerX[SAMPLE_FREQUENCY];
float accelerometerY[SAMPLE_FREQUENCY];
float accelerometerZ[SAMPLE_FREQUENCY];

float rawX[SAMPLE_FREQUENCY];
float rawY[SAMPLE_FREQUENCY];
float rawZ[SAMPLE_FREQUENCY];

float energyAverage[SECONDS_TO_AVERAGE];

float prevAccelX;
float prevAccelY;
float prevAccelZ;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200); 

  // Set up pin for accelerometer ground
  pinMode(vcc, OUTPUT);
  pinMode(ground, OUTPUT);
}

void loop() {

  // Set state for accelerometer ground pin
  digitalWrite(ground, LOW);
  digitalWrite(vcc, HIGH);

  // Grab current time so we can check if we need to sample or calculate anything
  unsigned long currentMillis = millis();

  // Run this code n times per second according to delay
  if ((currentMillis - prevMillis) > interval) {

    float sampleX = analogRead(accelerometerXPin);
    float sampleY = analogRead(accelerometerYPin);
    float sampleZ = analogRead(accelerometerZPin);

    // Store current sensor values in arrays
    accelerometerX[counter] = prevAccelX = sampleX * 0.5 + prevAccelX * 0.5;
    accelerometerY[counter] = prevAccelY = sampleY * 0.5 + prevAccelY * 0.5;
    accelerometerZ[counter] = prevAccelZ = sampleZ * 0.5 + prevAccelZ * 0.5;

    rawX[counter] = sampleX;
    rawY[counter] = sampleY;
    rawZ[counter] = sampleZ;
    
//    prevAccelX = sampleX;
//    prevAccelY = sampleY;
//    prevAccelZ = sampleZ;

    // wait 2 milliseconds before the next loop
    // for the analog-to-digital converter to settle
    // after the last reading:
    delay(2);

    //    Serial.print(counter);
    //    Serial.print(": accel x: ");
    //    Serial.print(accelerometerX[counter]);

    //if (counter != 0) {
    //Serial.print("\tdelta: ");
    //Serial.print(accelerometerX[counter] - accelerometerX[counter-1]);
    //}

    //    Serial.print("\n");


    //    Serial.print("i: ");
    //    Serial.print(counter);
    //    Serial.print("\n");

    counter++;

    // If we've filled up the sample array, then calculate energy
    if (counter == SAMPLE_FREQUENCY) {

      float xEnergy = 0;
      float yEnergy = 0;
      float zEnergy = 0;

      float xEnergyRaw = 0;
      float yEnergyRaw = 0;
      float zEnergyRaw = 0;

      for (int i = 1; i < SAMPLE_FREQUENCY; i++) {

        // high pass
        // for i from 1 to n
        //   y[i] := α * (y[i-1] + x[i] - x[i-1])

        //        Serial.print(i);
        //        Serial.print(": old xEnergy: ");
        //        Serial.print(xEnergy);
        //        Serial.print("\tdelta: ");
        //        Serial.print(abs(accelerometerX[i] - accelerometerX[i-1]));
        //
        //        Serial.print("\tcurrent: ");
        //        Serial.print(accelerometerX[i]);
        //
        //        Serial.print("\tprevious: ");
        //        Serial.print(accelerometerX[i-1]);

        xEnergy += abs(accelerometerX[i] - accelerometerX[i-1]);
        yEnergy += abs(accelerometerY[i] - accelerometerY[i-1]);
        zEnergy += abs(accelerometerZ[i] - accelerometerZ[i-1]);

        xEnergyRaw += abs(rawX[i] - rawX[i-1]);
        yEnergyRaw += abs(rawY[i] - rawY[i-1]);
        zEnergyRaw += abs(rawZ[i] - rawZ[i-1]);

      }

      float accelerometerEnergy = xEnergy + yEnergy + zEnergy;
      float accelerometerEnergyRaw = xEnergyRaw + yEnergyRaw + zEnergyRaw;

      float currentAverage = 0;
      for (int i = 0; i < SECONDS_TO_AVERAGE; i++) {
        if (i < SECONDS_TO_AVERAGE-1) {
          energyAverage[i] = energyAverage[i+1];
        }
        currentAverage += energyAverage[i];
      }
      energyAverage[SECONDS_TO_AVERAGE-1] = accelerometerEnergy;
      currentAverage += accelerometerEnergy;
      currentAverage /= SECONDS_TO_AVERAGE;

//      Serial.print("x: ");
//      Serial.print(xEnergy);
//      Serial.print("\t");
//
//      Serial.print("y: ");
//      Serial.print(yEnergy);
//      Serial.print("\t");
//
//      Serial.print("z: ");
//      Serial.print(zEnergy);
//      Serial.print("\t");

//      Serial.print("total: ");
      Serial.print(accelerometerEnergy);
      Serial.print(",");

//      Serial.print("raw: ");
      Serial.print(accelerometerEnergyRaw);
      Serial.print(",");

//      Serial.print("avg: ");
      Serial.print(currentAverage);

      Serial.print("\n");


      // Reset counter to begin storing at beginning of array
      counter = 0;
    }

    prevMillis = currentMillis;

  }


}














