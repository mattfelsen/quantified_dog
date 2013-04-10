/*

 Quantified Dog v0.1
 Matt Felsen
 Sensemakers Collaboration Studio
 Spring 2013
 
 */

#include <WiFlyHQ.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(8,9); // RX, TX

//#include <AltSoftSerial.h>
//AltSoftSerial wifiSerial(8,9);

WiFly wifly;

// Network settings
const char mySSID[] = "babymatt";
const char myPassword[] = "lookyhere6";

// Cosm settings
char site[] = "api.cosm.com";
char feedId[] = "119738";
char cosmKey[] = "ewTCG0qri8i6jXsXxwXxnrAZpnKSAKxHL0tnbndNeEpPdz0g";
char sensorId[] = "accelerometer_energy";

int counter = 0;

// Timing settings
#define SAMPLE_FREQUENCY 85

long interval = ceil(1000 / SAMPLE_FREQUENCY);
long prevMillis = 0;

// Sample storage
float accelerometerX[SAMPLE_FREQUENCY];
float accelerometerY[SAMPLE_FREQUENCY];
float accelerometerZ[SAMPLE_FREQUENCY];
float accelerometerEnergy = 0;

// Sensor I/O
int accelerometerXPin = 0;
int accelerometerYPin = 1;
int accelerometerZPin = 2;

void terminal();

void setup()
{
    char buf[32];

    Serial.begin(115200);
    Serial.println("Starting");
    Serial.print("Free memory: ");
    Serial.println(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println("Failed to start wifly");
	terminal();
    }

    /* Join wifi network if not already associated */
    if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	Serial.println("Joining network");
	wifly.setSSID(mySSID);
	wifly.setPassphrase(myPassword);
	wifly.enableDHCP();

	if (wifly.join()) {
	    Serial.println("Joined wifi network");
	} else {
	    Serial.println("Failed to join wifi network");
	    terminal();
	}
    } else {
        Serial.println("Already joined network");
    }

    //terminal();

    Serial.print("MAC: ");
    Serial.println(wifly.getMAC(buf, sizeof(buf)));
    Serial.print("IP: ");
    Serial.println(wifly.getIP(buf, sizeof(buf)));
    Serial.print("Netmask: ");
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    Serial.print("Gateway: ");
    Serial.println(wifly.getGateway(buf, sizeof(buf)));

    wifly.setDeviceID("Wifly-WebClient");
    Serial.print("DeviceID: ");
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    if (wifly.isConnected()) {
        Serial.println("Old connection active. Closing");
	wifly.close();
    }

    if (wifly.open(site, 8081)) {
        Serial.print("Connected to ");
	Serial.println(site);

    } else {
        Serial.println("Failed to connect");
    }
}

void loop()
{
    unsigned long currentMillis = millis();
    if ((currentMillis - prevMillis) > interval) {
      
      float xEnergy, yEnergy, zEnergy = 0;
      
      for (int i = 1; i <= SAMPLE_FREQUENCY; i++) {
       
        // high pass
        // for i from 1 to n
        //   y[i] := α * (y[i-1] + x[i] - x[i-1])
        
        
        xEnergy += abs(accelerometerX[i] - accelerometerX[i-1]);
        yEnergy += abs(accelerometerY[i] - accelerometerY[i-1]);
        zEnergy += abs(accelerometerZ[i] - accelerometerZ[i-1]);
        
      }
      
      accelerometerEnergy = xEnergy + yEnergy + zEnergy;
      
      counter = 0;

      sendDataToCosm();
      prevMillis = currentMillis;
      
    }
    
    // Store current sensor values in arrays
    accelerometerX[counter] = analogRead(accelerometerXPin);
    accelerometerY[counter] = analogRead(accelerometerYPin);
    accelerometerZ[counter] = analogRead(accelerometerZPin);


    if (wifly.available() > 0) {
	char ch = wifly.read();
	Serial.write(ch);
	if (ch == '\n') {
	    /* add a carriage return */ 
	    Serial.write('\r');
	}
    }

    if (Serial.available() > 0) {
	wifly.write(Serial.read());
    }
    
    // Increase array index counter 
    counter++;
}

void sendDataToCosm() {
  wifly.print("{\"method\" : \"put\",");
  wifly.print("\"resource\" : \"/feeds/");
  wifly.print(feedId);
  wifly.print("\", \"headers\" :{\"X-ApiKey\" : \"");
  wifly.print(cosmKey);
  wifly.print("\"},\"body\" :{ \"version\" : \"1.0.0\",\"datastreams\" : [{\"id\" : \"");
  wifly.print(sensorId);
  wifly.print("\",\"current_value\" : \"");
  wifly.print(accelerometerEnergy);
  wifly.print("\"}]}}");
  wifly.println();
}
/* Connect the WiFly serial to the serial monitor. */
void terminal()
{
    while (1) {
	if (wifly.available() > 0) {
	    Serial.write(wifly.read());
	}


	if (Serial.available() > 0) {
	    wifly.write(Serial.read());
	}
    }
}
