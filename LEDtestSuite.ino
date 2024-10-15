/*
Color sensing, 2 tests
test 1: Multiplexed LED's, 1 cone 
photoDiode input: A0

test 2: 3 cones, each with its own LED and photo receptor
PhotoDiode input Red: A1
PhotoDiode input Blue: A2
PhotoDiode input Yellow: A3


*/
#include <String.h>

int RedLEDpinTest1 = 9;
int BlueLEDpinTest1 = 8;


void setup() {
  Serial.begin(9600);
  pinMode(RedLEDpinTest1, OUTPUT);
  pinMode(BlueLEDpinTest1, OUTPUT);
}

void loop() {
  String Colors[] = {"Red","Blue"};

  //Test 1 sensing multiplex circuit
  
  float Sensing[2]; //stores [red,blue,yellow]
  int plex[2] = {1,0};

  for (int i = 0; i < 2; i++) {
    
    digitalWrite(RedLEDpinTest1, plex[i]);
    digitalWrite(BlueLEDpinTest1, plex[(i+1)%2]);
    // digitalWrite(BlueLEDpinTest1, plex[(i+2)%3]);
    
    delay(50);
    float sensorValue = analogRead(A0) * (5.0/1023);
    Sensing[i] = sensorValue;
    delay(10);
      }
  //Then save Sensing to a file
  //test 2
    for (int i = 0; i < 2; i++) {
      Serial.print(Colors[i]);
      Serial.print(" : ");
    Serial.print(Sensing[i]);
    Serial.print("   ");
  }
Serial.print("\n");
}




