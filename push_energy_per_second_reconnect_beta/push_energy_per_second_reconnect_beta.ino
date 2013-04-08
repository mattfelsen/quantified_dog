/*

 Quantified Dog v0.1
 Matt Felsen
 Sensemakers Collaboration Studio
 Spring 2013
 
 */

#include <WiFlyHQ.h>

// WiFi & network settings
WiFly wifly;
//const char wifiSSID[] = "babymatt";
//const char wifiPassword[] = "lookyhere6";

const char wifiSSID[] = "Comfort_Zone";
const char wifiPassword[] = "1441dekalbno8";

const char site[] = "api.cosm.com";
const int port = 8081;

// COSM settings
char feedId[] = "119738";
//char feedId[] = "104810";
char cosmKey[] = "a9qsdmAkt7Fm1ffn9BGCzbE2RRaSAKxaN2N1M3pVMkxCRT0g";
//char cosmKey[] = "ewTCG0qri8i6jXsXxwXxnrAZpnKSAKxHL0tnbndNeEpPdz0g";
char sensorId[] = "accelerometer_energy";

// Set up accelerometer pins
const int accelerometerXPin = A2;
const int accelerometerYPin = A3;
const int accelerometerZPin = A4;

float xEnergy;
float yEnergy;
float zEnergy;
float accelerometerEnergy;

// Use analog pins as ground for accelerometer
const int ground = A5;

// Counter for which position of the array is being written to
int counter = 0;

// Timing settings
//#define SAMPLE_FREQUENCY 85
#define SAMPLE_FREQUENCY 10

// Frequency of sampling sensor
long interval = ceil(1000 / SAMPLE_FREQUENCY);
long prevMillis = 0;

// Frequency of sending data
long intervalPush = 1000;
long prevMillisPush = 0;

// Sample storage
float accelerometerX[SAMPLE_FREQUENCY];
float accelerometerY[SAMPLE_FREQUENCY];
float accelerometerZ[SAMPLE_FREQUENCY];

void setup() {

  // Begin serial for WiFly
  // MUST BE 9600!
  Serial.begin(9600);
  wifly.begin(&Serial);

  // Join wifi network if not already associated
  if (!wifly.isAssociated()) {

    wifly.setSSID(wifiSSID);
    wifly.setPassphrase(wifiPassword);
    wifly.enableDHCP();

    if (wifly.join()) {
      // Joined
    } 
    else {
      // Couldn't join
    }
  } 
  else {
    // Already joined
  }

  wifly.setDeviceID("Wifly-WebClient");
  if (wifly.isConnected()) {
    // Already connected, close & make sure we're connected to correct server/port
    wifly.close();
  }

  //  if (wifly.open(site, port)) {
  //    // Connected
  //  } 
  //  else {
  //    // Couldn't connect
  //  }


  // Set up pin for accelerometer ground
  pinMode(ground, OUTPUT);
}

void loop() {

  // Check sonnection
  if (wifly.isConnected() == false) {
    if (wifly.open(site, port)) {
      // Connected
    } 
    else {
      // Failed to open
    }

    delay(1000);
  }


  // Set state for accelerometer ground pin
  digitalWrite(ground, LOW);

  // Grab current time so we can check if we need to sample or calculate anything
  unsigned long currentMillis = millis();

  // Run this code n times per second according to delay
  if ((currentMillis - prevMillis) > interval) {

    // Store current sensor values in arrays
    accelerometerX[counter] = analogRead(accelerometerXPin);
    accelerometerY[counter] = analogRead(accelerometerYPin);
    accelerometerZ[counter] = analogRead(accelerometerZPin);

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


    counter++;

    // If we've filled up the sample array, then calculate energy
    if (counter == SAMPLE_FREQUENCY) {

      xEnergy = 0;
      yEnergy = 0;
      zEnergy = 0;

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

      }

      accelerometerEnergy = xEnergy + yEnergy + zEnergy;
      //pushToCOSM();

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
//
//      Serial.print("total: ");
//      Serial.print(accelerometerEnergy);
//      Serial.print("\n");


      // Reset counter to begin storing at beginning of array
      counter = 0;
    }

    prevMillis = currentMillis;

  }

  // Send to COSM if it's time
  if ((currentMillis - prevMillisPush) > intervalPush) {
    pushToCOSM();
    prevMillisPush = currentMillis;
  }

}

void pushToCOSM() {
  wifly.print("{");
  wifly.print("\"method\" : \"put\",");
  wifly.print("\"resource\" : \"/feeds/");
  wifly.print(feedId);
  wifly.print("\", ");
  wifly.print("\"headers\" : {");
  wifly.print("\"X-ApiKey\" : \"");
  wifly.print(cosmKey);
  wifly.print("\"},");
  wifly.print("\"body\" : { ");
  wifly.print("\"version\" : \"1.0.0\",");
  wifly.print("\"datastreams\" : [");

  wifly.print("{\"id\" : \"");
  wifly.print("accelerometer_x");
  wifly.print("\", \"current_value\" : \"");
  wifly.print(xEnergy);
  wifly.print("\"},");

  wifly.print("{\"id\" : \"");
  wifly.print("accelerometer_y");
  wifly.print("\", \"current_value\" : \"");
  wifly.print(yEnergy);
  wifly.print("\"},");

  wifly.print("{\"id\" : \"");
  wifly.print("accelerometer_z");
  wifly.print("\", \"current_value\" : \"");
  wifly.print(zEnergy);
  wifly.print("\"},");

  wifly.print("{\"id\" : \"");
  wifly.print("accelerometer_energy");
  wifly.print("\", \"current_value\" : \"");
  wifly.print(accelerometerEnergy);
  wifly.print("\"}");

  wifly.print("]}}");
  wifly.println();
}




