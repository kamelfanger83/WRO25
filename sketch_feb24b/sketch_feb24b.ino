#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

void setup() {
    Serial.begin(9600);
    Serial.println("<Arduino is ready>");
    myservo.attach(9);
}

void loop() {
    myservo.write(recvInt());
}

int recvInt() {
  const byte numChars = 32;
  char receivedChars[numChars]; 
    byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (true) {
      while (Serial.available() == 0) ;
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            return atoi(receivedChars);
        }
    }
    
}
