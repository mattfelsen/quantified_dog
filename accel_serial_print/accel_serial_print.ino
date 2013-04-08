/*

 Quantified Dog v0.1
 Matt Felsen
 Sensemakers Collaboration Studio
 Spring 2013
 
 */

const int accelXPin = A0;
const int accelYPin = A1;
const int accelZPin = A2;

int accelX = 0;
int accelY = 0;
int accelZ = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
}

void loop() {
  // read the analog in value:
  accelX = analogRead(accelXPin);
  accelY = analogRead(accelYPin);
  accelZ = analogRead(accelZPin);

  accelX -= 512;

  // map it to the range of the analog out:
  //outputValue = map(sensorValue, 0, 1023, 0, 255);  
  // change the analog out value:
  //analogWrite(analogOutPin, outputValue);           

  // print the results to the serial monitor:
  Serial.print("X: " );
  Serial.print(accelX);
  Serial.print("\t");

  Serial.print("Y: " );
  Serial.print(accelY);
  Serial.print("\t");

  Serial.print("Z: " );
  Serial.print(accelZ);
  Serial.print("\n");

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(500);                     
}


