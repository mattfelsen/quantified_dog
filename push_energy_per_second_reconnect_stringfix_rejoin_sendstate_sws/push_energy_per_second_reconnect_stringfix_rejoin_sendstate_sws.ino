/*

 Quantified Dog v0.1
 Matt Felsen
 Sensemakers Collaboration Studio
 Spring 2013
 
 */

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>

SoftwareSerial wifiSerial(8,9); // RX, TX

// WiFi & network settings
WiFly wifly;
//const char wifiSSID[] = "babymatt";
//const char wifiPassword[] = "lookyhere6";

const char wifiSSID[] = "Comfort_Zone";
const char wifiPassword[] = "1441dekalbno8";

//const char wifiSSID[] = "internetz";
//const char wifiPassword[] = "1nt3rn3tz";

const char site[] = "api.cosm.com";
const int port = 8081;

// COSM settings
//char feedId[] = "119738";
//char feedId[] = "104810";
char feedId[] = "124246";

//char cosmKey[] = "a9qsdmAkt7Fm1ffn9BGCzbE2RRaSAKxaN2N1M3pVMkxCRT0g";
//char cosmKey[] = "ewTCG0qri8i6jXsXxwXxnrAZpnKSAKxHL0tnbndNeEpPdz0g";
char cosmKey[] = "lRRvQT0QkYTbTz1vGMCvavaG0uiSAKxaZU9JYW5LdUNDST0g";

// Set up accelerometer pins
const int accelerometerXPin = A0;
const int accelerometerYPin = A1;
const int accelerometerZPin = A2;

float xEnergy;
float yEnergy;
float zEnergy;
float accelerometerEnergy;

int currentState = 1;
int prevState = 1;

const int thresholdLowToMed = 100;
const int thresholdMedtoHigh = 1000;

static String accX = "x";
static String accY = "y";
static String accZ = "z";
static String accE = "e";

// Use analog pins as ground for accelerometer
const int ground = 15;
const int power = 14;

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
  wifiSerial.begin(9600);
  wifly.begin(&wifiSerial, &Serial);

  // Join wifi network if not already associated
  if (!wifly.isAssociated()) {

    wifly.setSSID(wifiSSID);
    wifly.setPassphrase(wifiPassword);
    wifly.setJoin(WIFLY_WLAN_JOIN_AUTO);
    wifly.enableDHCP();

    if (wifly.join()) {
      Serial.println("Joined network.");
    } 
    else {
      Serial.println("Could not associate with network.");
    }
  } 
  else {
    Serial.println("Already associated with network.");
  }

  wifly.setDeviceID("Wifly-WebClient");
  if (wifly.isConnected()) {
    Serial.println("Closing existing connection.");
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
  pinMode(power, OUTPUT);
}

void loop() {

  // Check sonnection
  if(wifly.isAssociated()){
    if (wifly.isConnected() == false) {
      Serial.println("Not connected to network, trying to connect...");
      if (wifly.open(site, port)) {
        Serial.println("Opened network connection.");
      } 
      else {
        Serial.println("Failed to open connection.");
      }

      delay(1000);
    }
  } else {
    Serial.println("Not associated within run loop.");
  }


  // Set state for accelerometer ground pin
  digitalWrite(ground, LOW);
  digitalWrite(power, HIGH);

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

      if (accelerometerEnergy < thresholdLowToMed) {
        currentState = 1;
      }
      if (accelerometerEnergy >= thresholdLowToMed && accelerometerEnergy < thresholdMedtoHigh) {
        currentState = 2;
      }
      if (accelerometerEnergy >= thresholdMedtoHigh) {
        currentState = 3;
      }

      if (currentState != prevState) {
        sendStateToCOSM();
      }

      prevState = currentState;

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
  //  wifly.print("{");
  //  wifly.print("\"method\" : \"put\",");
  //  wifly.print("\"resource\" : \"/feeds/");
  //  wifly.print(feedId);
  //  wifly.print("\", ");
  //  wifly.print("\"headers\" : {");
  //  wifly.print("\"X-ApiKey\" : \"");
  //  wifly.print(cosmKey);
  //  wifly.print("\"},");
  //  wifly.print("\"body\" : { ");
  //  wifly.print("\"version\" : \"1.0.0\",");
  //  wifly.print("\"datastreams\" : [");
  //
  //  wifly.print("{\"id\" : \"");
  //  wifly.print("accelerometer_x");
  //  wifly.print("\", \"current_value\" : \"");
  //  wifly.print(xEnergy);
  //  wifly.print("\"},");
  //
  //  wifly.print("{\"id\" : \"");
  //  wifly.print("accelerometer_y");
  //  wifly.print("\", \"current_value\" : \"");
  //  wifly.print(yEnergy);
  //  wifly.print("\"}");
  //
  //  wifly.print("{\"id\" : \"");
  //  wifly.print("accelerometer_z");
  //  wifly.print("\", \"current_value\" : \"");
  //  wifly.print(zEnergy);
  //  wifly.print("\"},");
  //
  //  wifly.print("{\"id\" : \"");
  //  wifly.print("accelerometer_energy");
  //  wifly.print("\", \"current_value\" : \"");
  //  wifly.print(accelerometerEnergy);
  //  wifly.print("\"}");
  //
  //  wifly.print("]}}");
  //  wifly.println();

  wifly.print("{\"method\":\"put\",\"resource\":\"/feeds/");
  wifly.print(feedId);
  wifly.print("\",\"headers\":{\"X-ApiKey\":\"");
  wifly.print(cosmKey);
  wifly.print("\"},\"body\":{\"version\":\"1.0.0\",\"datastreams\":[");
  wifly.print("{\"id\":\"");
  wifly.print(accX);
  wifly.print("\",\"current_value\":\"");
  wifly.print(xEnergy);
  wifly.print("\"},{\"id\":\"");
  wifly.print(accY);
  wifly.print("\",\"current_value\":\"");
  wifly.print(yEnergy);
  wifly.print("\"},{\"id\":\"");
  wifly.print(accZ);
  wifly.print("\",\"current_value\":\"");
  wifly.print(zEnergy);
  wifly.print("\"},{\"id\":\"");
  wifly.print(accE);
  wifly.print("\",\"current_value\":\"");
  wifly.print(accelerometerEnergy);
  wifly.print("\"}]}}");
  wifly.println();

}

void sendStateToCOSM() {

  wifly.print("{\"method\":\"put\",\"resource\":\"/feeds/");
  wifly.print(feedId);
  wifly.print("\",\"headers\":{\"X-ApiKey\":\"");
  wifly.print(cosmKey);
  wifly.print("\"},\"body\":{\"version\":\"1.0.0\",\"datastreams\":[");
  wifly.print("{\"id\":\"state\",\"current_value\":\"");
  wifly.print(currentState);
  wifly.print("\"}]}}");
  wifly.println();

}












