#include <Servo.h>


#define PWM_PIN 11
#define FG_PIN 2
#define DIR_PIN 9
#define SERVO_PIN 6

int actualServo = 84;
int targetServo = 84;

// Positioning variables. Everything is in cm.
float x = 0, y = 0, theta = 0;
const float wheelDiam = 5.13;
volatile unsigned long pulses = 0;
unsigned long lastUpdatePulses = 0;

unsigned long lastPosePrint = 0;
unsigned long lastServoUpdate =0;
void registerPulse () {
  ++pulses;
}


Servo myservo;
float predictedTurnRadius(int currentServo){
 if(currentServo == 84) return 10000000;
 float a = 0.000372471;
 float b = -5.71154;
 int angle = abs(currentServo-84);
 if(currentServo < 84){
  return -(1./(a*angle) + b);
 }
 return 1./(a*angle) + b;
}



void setup() {
  Serial.begin(115200);
  pinMode(DIR_PIN, OUTPUT); // direction control PIN 10 with direction wire
  pinMode(PWM_PIN, OUTPUT); // PWM PIN 11 with PWM wire
  myservo.attach(SERVO_PIN);
  myservo.write(actualServo);

  digitalWrite(DIR_PIN, LOW);
  analogWrite(PWM_PIN, 255);
  
  attachInterrupt(digitalPinToInterrupt(FG_PIN), registerPulse, RISING);

  // Send a ready signal after initialization
  Serial.println("READY");
  
}

void loop() {
  if (Serial.available())  {
    // Read the full line until newline
    String input = Serial.readStringUntil('\n');
    int num = input.toInt();  // convert the string to an integer
    if(num==6969){
      x = 0;
      y = 0;
      theta = 0;
    }
    else if (num % 2 == 0) {
      Serial.print("New motor speed: ");
      Serial.println(num / 2);
      digitalWrite(DIR_PIN, num >= 0 ? LOW : HIGH);
      analogWrite(PWM_PIN, 255 - abs(num / 2));
    } else  {
      targetServo = num / 2;
      Serial.print("New servo angle:");
      Serial.println(targetServo);
      
    }
  }
  
  if(micros() > lastServoUpdate + 5200 && actualServo != targetServo) {
    if (actualServo > targetServo) --actualServo;
    if (actualServo < targetServo) ++actualServo;
    myservo.write(actualServo);
    lastServoUpdate = micros();
  }
  float shaftD = float(pulses - lastUpdatePulses) / 268.;
  lastUpdatePulses = pulses;
  

  float turnR = predictedTurnRadius(actualServo);
  
  // TODO: consider which back wheel slips how much
  
  float dTheta = (shaftD * wheelDiam * M_PI) / (turnR + 5.2*(turnR>0 ? 1 : -1));
 
  // Coordinates are in ICR system. Idk coordinate systems here are a bit of a
  // mess.
  float xC = turnR;
  float yC = 12.5;
  float xCp = xC * cos(dTheta) - yC * sin(dTheta);
  float yCp = xC * sin(dTheta) + yC * cos(dTheta);
  x += (xCp - xC) * sin(theta) + (yCp - yC) * cos(theta);
  y += -(xCp - xC) * cos(theta) + (yCp - yC) * sin(theta);
  theta += dTheta;
  if (micros() > lastPosePrint + 10000) {
    Serial.print("[x=");
    Serial.print(x);
    Serial.print(",y=");
    Serial.print(y);
    Serial.print(",t=");
    Serial.print(theta);
    Serial.println("]");
    lastPosePrint = micros();
  }
  
}
