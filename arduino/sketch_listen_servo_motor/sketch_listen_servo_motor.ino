#include <Servo.h>
#include <util/atomic.h>

#define PWM_PIN 11
#define FG_PIN 2
#define DIR_PIN 9
#define SERVO_PIN 6
#define START_PIN 3

int actualServo = 84;
int targetServo = 84;
int actualSpeed = 0;
int targetSpeed = 0;

// Positioning variables. Everything is in cm.
float x = 0, y = 0, theta = 0;
const float wheelDiam = 5.13;
volatile unsigned int pulses = 0;
bool forward = true;

unsigned long lastPosePrint = 0;
unsigned long lastServoUpdate =0;
unsigned long lastSpeedUpdate =0;

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
  pinMode(START_PIN, INPUT_PULLUP);
  myservo.attach(SERVO_PIN);
  myservo.write(actualServo);

  digitalWrite(DIR_PIN, LOW);
  analogWrite(PWM_PIN, 255);

  attachInterrupt(digitalPinToInterrupt(FG_PIN), registerPulse, RISING);

  myservo.write(70);
  delay(1000);
  myservo.write(actualServo);

  // Send a ready signal after initialization
  Serial.println("READY");

  while(digitalRead(START_PIN) == HIGH);

  Serial.println("GO!");
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
      targetSpeed = num / 2;
      Serial.println(targetSpeed);
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
  if(micros() > lastSpeedUpdate + 3000 && actualSpeed != targetSpeed) {
    if (abs(actualSpeed) > abs(targetSpeed)) actualSpeed = targetSpeed;
    else if (actualSpeed > targetSpeed) --actualSpeed;
    else if (actualSpeed < targetSpeed) ++actualSpeed;
    digitalWrite(DIR_PIN, actualSpeed >= 0 ? LOW : HIGH);
    if (actualSpeed < 0) {
       forward = false;
    }
    if (actualSpeed > 0) {
       forward = true;
    }
    analogWrite(PWM_PIN, 255 - abs(actualSpeed));
    lastSpeedUpdate = micros();
  }
  int npulses;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    npulses = pulses;
    pulses = 0;
  }

  float shaftD = float(npulses) / 268.;

  float turnR = predictedTurnRadius(actualServo);

  // TODO: consider which back wheel slips how much

  double con = 0;
  if (turnR > 0) {
    con = 8;
  }

  float dTheta = (shaftD * wheelDiam * M_PI * (forward ? 1 : -1)) / (turnR + con*(turnR>0 ? 1 : -1));

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
