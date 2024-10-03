/*
Color sensing, 2 tests
test 1: Multiplexed LED's, 1 cone 
photoDiode input: A0

test 2: 3 cones, each with its own LED and photo receptor
PhotoDiode input Red: A1
PhotoDiode input Blue: A2
PhotoDiode input Yellow: A3


*/

int RedLEDpinTest1 = 8;
int BlueLEDpinTest1 = 9;
int YellowLEDpinTest1 = 10;
int Test2LEDs = 11;


void setup() {
  pinMode(RedLEDpinTest1, OUTPUT);
  pinMode(BlueLEDpinTest1, OUTPUT);
  pinMode(YellowLEDpinTest1, OUTPUT);
  pinMode(Test2LEDs, OUTPUT);
}

void loop() {

  //Test 1 sensing multiplex circuit
  int Sensing[3]; //stores [red,blue,yellow]
  int plex[3] = {1,0,0};

  for (int i = 0; i < 3; i++) {
    
    digitalWrite(RedLEDpinTest1, plex[i]);
    digitalWrite(YellowLEDpinTest1, plex[(i+1)%3]);
    digitalWrite(BlueLEDpinTest1, plex[(i+2)%3]);
    
    delay(10);
    int sensorValue = analogRead(A0);
    Sensing[i] = sensorValue * (5.0 / 1023.0);
  }
  //Then save Sensing to a file
  //test 2


}




