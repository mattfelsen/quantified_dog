/*

 Quantified Dog v0.1
 Matt Felsen
 Sensemakers Collaboration Studio
 Spring 2013
 
 */

#include <WiFlyHQ.h>


WiFly wifly;

// Network settings
//const char mySSID[] = "babymatt";
//const char myPassword[] = "lookyhere6";

const char mySSID[] = "Comfort_Zone";
const char myPassword[] = "1441dekalbno8";

// Cosm settings
char site[] = "api.cosm.com";
char feedId[] = "123145";
char cosmKey[] = "a9qsdmAkt7Fm1ffn9BGCzbE2RRaSAKxaN2N1M3pVMkxCRT0g";
char sensorId[] = "counter";

int counter = 0;

long interval = 500;
long prevMillis = 0;


void terminal();

void setup()
{
  char buf[32];

  Serial.begin(9600);
  Serial.println("Starting");
  Serial.print("Free memory: ");
  Serial.println(wifly.getFreeMemory(),DEC);

  if (!wifly.begin(&Serial)) {
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
    } 
    else {
      Serial.println("Failed to join wifi network");
      terminal();
    }
  } 
  else {
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

}

void loop()
{

  if (wifly.isConnected() == false) {
    if (wifly.open(site, 8081)) {
      Serial.print("Connected to ");
      Serial.println(site);

    } 
    else {
      Serial.println("Failed to connect");
    }
    
    delay(1000);
  }


  unsigned long currentMillis = millis();
  if ((currentMillis - prevMillis) > interval) {

    sendDataToCosm();
    prevMillis = currentMillis;

  }



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
  wifly.print(counter++);
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

