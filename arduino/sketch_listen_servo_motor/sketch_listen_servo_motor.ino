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

//returns the secant point of 2 circles
float holderFrontY(float fusionYSteer) {
    // Parameters of the wheel holder circle
    const float x1 = -6.0, y1 = -6.0, r1 = 2.6;
    // Parameters of the connector piece circle
    const float x2 = -7.8, r2 = 2.8105;
    const float y2 = fusionYSteer;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float d = sqrt(dx * dx + dy * dy);

    float a = (r1*r1 - r2*r2 + d*d) / (2 * d);
    float x3 = x1 + a * dx / d;
    float y3 = y1 + a * dy / d;

    float h = sqrt(r1*r1 - a*a);

    float rx = -dy * (h / d);
    float ry = dx * (h / d);

    float xi1 = x3 + rx;
    float yi1 = y3 + ry;
    float xi2 = x3 - rx;
    float yi2 = y3 - ry;

    // Take point with smaller x (the other one we don't care about)
    return (xi1 < xi2) ? yi1 : yi2;
}

/// Returns the angle the wheels are tilted given the angle of the servo.
float servoAngleToWheelAngle(int actualServo) {
  float fusionYSteer = -3.3058 + float(84.5 - actualServo) * 0.020944;
  float yHolderFront = holderFrontY(fusionYSteer);
  return asin((-6 - yHolderFront) / 2.6);
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
  Serial.println(servoAngleToWheelAngle(84.5));
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
  //float wheelAngle = servoAngleToWheelAngle(actualServo);

  float turnR = predictedTurnRadius(actualServo);
  //float turnR = 17.5 / tan(wheelAngle);
  // TODO: consider which back wheel slips how much
  
  float dTheta = (shaftD * wheelDiam * M_PI) / (turnR + 5*(turnR>0 ? 1 : -1));
 
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
    Serial.println(servoAngleToWheelAngle(actualServo));
  }
  /*if (theta <= -M_PI_2 ) {
    analogWrite(PWM_PIN, 255 - abs(0));
  }*/
}
