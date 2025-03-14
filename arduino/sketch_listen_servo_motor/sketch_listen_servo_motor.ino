#include <Servo.h>

#define PWM_PIN 11
#define DIR_PIN 9
#define SERVO_PIN 6

Servo myservo;

void setup() {
  Serial.begin(115200);
  pinMode(DIR_PIN, OUTPUT); // direction control PIN 10 with direction wire
  pinMode(PWM_PIN, OUTPUT); // PWM PIN 11 with PWM wire
  myservo.attach(SERVO_PIN);

  digitalWrite(DIR_PIN, LOW);
  analogWrite(PWM_PIN, 255);

  // Send a ready signal after initialization
  Serial.println("READY");
}

void loop() {
  if (Serial.available())  {
    // Read the full line until newline
    String input = Serial.readStringUntil('\n');
    int num = input.toInt();  // convert the string to an integer
    
    if (num % 2 == 0) {
      Serial.print("New motor speed: ");
      Serial.println(num / 2);
      digitalWrite(DIR_PIN, num >= 0 ? LOW : HIGH);
      analogWrite(PWM_PIN, 255 - abs(num / 2));
    } else {
      Serial.print("New servo angle:");
      Serial.println(num / 2);
      myservo.write(num / 2);
    }
  }
}
