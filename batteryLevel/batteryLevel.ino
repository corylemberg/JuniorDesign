int greenLED = 4;
int yellowLED = 12;
int redLED = 13;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  float batteryValue = analogRead(A2) * (5.0/1023);
  
  Serial.println(batteryValue);

  if (batteryValue >= 3.33) {
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
  }
  else if (batteryValue < 3.33 && batteryValue >= 1.66) {
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
  }
  else {
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, HIGH);
  }
}
