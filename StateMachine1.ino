#include <ArduinoHttpClient.h>
#include <WiFi.h>
#include <String.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

static const uint8_t D9 = 9;
static const uint8_t D11 = 11;

SoftwareSerial mySerial(D9, D11);
DFRobotDFPlayerMini MP3player;

int buttonState = 0;
int previousButtonState = 0;
int stateVariable = 0;

int RedLEDpinTest1 = 7;
int BlueLEDpinTest1 = 8;

char ssid[] = "tufts_eecs";
char pass[] = "foundedin1883";

char serverAddress[] = "34.28.153.91";  // server address
int port = 80;

WiFiClient wifi;
WebSocketClient client = WebSocketClient(wifi, serverAddress, port);
String clientID = "DCF2BCAB6F0B"; //Insert your Server ID Here!;

int status = WL_IDLE_STATUS;

int currIndex = 0;
bool move = false;

int soundCount = 0;

int movePrev = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  
  MP3player.begin(mySerial);

  MP3player.volume(10);
  // delay(500);
  // while (status != WL_CONNECTED) {
  //   Serial.print("Attempting to connect to Network named: ");
  //   Serial.println(ssid);                   // print the network name (SSID);

  //   // Connect to WPA/WPA2 network:
  //   status = WiFi.begin(ssid, pass);
  // }
  // // print the SSID of the network you're attached to:
  // Serial.print("SSID: ");
  // Serial.println(WiFi.SSID());

  // // print your WiFi shield's IP address:
  // IPAddress ip = WiFi.localIP();
  // Serial.print("IP Address: ");
  // Serial.println(ip);

  // Serial.println("starting WebSocket client");
  // client.begin();
  // client.beginMessage(TYPE_TEXT);
  // client.print(clientID);
  // client.endMessage();

  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(RedLEDpinTest1, OUTPUT);
  pinMode(BlueLEDpinTest1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  previousButtonState = digitalRead(2);
  String Colors[] = {"Red","Blue", "Ambient"};
}


void loop() {
  //int sensorValue = analogRead(A0);
  //float voltage = sensorValue * (5.0 / 1023.0);
  // Serial.print("Wifi Status: ");
  // Serial.println(WiFi.status());
 // buttonState = digitalRead(2);
  // if ((buttonState == HIGH) && (previousButtonState == LOW)) {
  //   Serial.println("STATE CHANGE");
  //   changeStateVariable();
  //   delay(50);
  // }
  // forward();
 // previousButtonState = buttonState;
  
  // int messageSize = client.parseMessage();
  //   if (messageSize > 0) {
  //     Serial.println("Received a message:");
  //     String message = client.readString();
  //     Serial.println(message);
  //     if (message == "WebClient_4A9EDB0160D5.motors.255.-255") {
  //       forward();
  //     }
  //     else if (message == "WebClient_4A9EDB0160D5.motors.-255.255") {
  //       backward();
  //     }
  //     else if (message == "WebClient_4A9EDB0160D5.motors.0.200") {
  //       leftTurn();
  //     }
  //     else if (message == "WebClient_4A9EDB0160D5.motors.0.0") {
  //       stopAll();
  //     }
  //     else {
  //       stopAll();
  //     }
  //   }
  //   // wait 10ms
  // delay(10);
  float Sensing[2]; //stores [red,blue,yellow]
  int plex[2] = {1,0};
  for (int i = 0; i < 2; i++) {
    digitalWrite(BlueLEDpinTest1, 0);
    digitalWrite(RedLEDpinTest1, 0);

    delay(50);
    float ambientValue = analogRead(A0) * (5.0/1023);
    delay(10);

    digitalWrite(RedLEDpinTest1, plex[i]);
    digitalWrite(BlueLEDpinTest1, plex[(i+1)%2]);
      
    delay(50);
    float sensorValue = analogRead(A0) * (5.0/1023);
    Sensing[i] = sensorValue/ambientValue;
    delay(10);
  // Serial.println("disconnected");         
  }
    int pathColorsSize = 4;
    int pathColors[pathColorsSize] = {};
    Serial.print("sensor1: ");
    Serial.print(Sensing[0]);
    Serial.print("   sensor2: ");
    Serial.print(Sensing[1]);
    Serial.print("\n");
    if ((fabs(Sensing[0] - Sensing[1]) < 0.25)) {
      // Serial.println(Sensing[0]);
      if (Sensing[0] > 1.4) {
        pathColors[currIndex % 5] = 1;
        //rightTurn();
        //forward();
      }
      else {
        
        pathColors[currIndex % 5] = 2;
        //stopAll();
      }
    }
    else {
      if (Sensing[0] > Sensing[1]) {
        pathColors[currIndex % 5] = 3;
        //forward();
      } 
      else if (Sensing[0] < Sensing[1]) {
        pathColors[currIndex % 5] = 4;
        //stopAll();
      }
    }

    int sameCount = 0;
    for (int j = 0; j < pathColorsSize; j++) {
      // Serial.println(pathColors[j]);
      // Serial.println(pathColors[j + 1]);
      if (pathColors[j + 1] == pathColors[j]) {
        sameCount++;
      }
    }

    //Serial.println(sameCount);

    if (sameCount >= pathColorsSize-1) {
      move = true;
      //Serial.println("HEEEEEEEERE");
      //Serial.println("Robust Sensed");
    }
    else {
      move = false;
    }
    //Serial.println(move);
  float wallSense = analogRead(A1) * (5.0/1023);
  //Serial.print("wallSenseValue: ");
  //Serial.println(wallSense);
  // put your main code here, to run repeatedly:
  if (wallSense >= 0.0) {
   // Serial.println(wallSense);
   // stopAll();
  }
  //else {
    if (move == true) {
      if (pathColors[0] == 1) {
        if (movePrev != pathColors[0]) {
          playYellow();
          forward();
        }
        Serial.println("YELLOW");
        movePrev = 1;
      }
      else if (pathColors[0] == 2) {
        if (movePrev != pathColors[0]) {
          playBlack();
          forward();
        }
        Serial.println("BLACK");
        movePrev = 2;
      }
      else if (pathColors[0] == 3) {
        if (movePrev != pathColors[0]) {
          playRed();
          forward();
        }
        Serial.println("RED");
        movePrev = 3;
      }
      else if (pathColors[0] == 4) {
        if (movePrev != pathColors[0]) {
          playBlue();
          forward();
        }
        Serial.println("BLUE");
        movePrev = 4;
      }
    }
 // }

  currIndex++;
}

void changeStateVariable() {
  if (stateVariable == 6) {
    stateVariable = 0;
  } else {
    stateVariable++;
  }
  Serial.println(stateVariable);
  changeState();
}

void changeState() {
  switch (stateVariable) {
    case 0:  // your hand is on the sensor
      Serial.println("Stop");
      // forward();
      break;
    case 1:  // your hand is close to the sensor
      Serial.println("Go Forward");
      // backward();
      break;
    case 2:  // your hand is a few inches from the sensor
      Serial.println("Go backward");
      // forward();
      break;
    case 3:  // your hand is nowhere near the sensor
      Serial.println("Pivot in place clockwise");
      // backward();
      break;
    case 4:  // your hand is nowhere near the sensor
      Serial.println("Pivot in place counterclockwise");
      // forward();
      break;
    case 5:  // your hand is nowhere near the sensor
      Serial.println("Right turn with desired radius");
      // backward();
      break;
    case 6:  // your hand is nowhere near the sensor
      Serial.println("Left turn with desired radius");
      // forward();
      break;
  }
}

void forwardRight() {
    analogWrite(10, 85);
  digitalWrite(3, LOW);
  //digitalWrite(4, HIGH);
}

void backwardRight() {
  // digitalWrite(3, HIGH);
  analogWrite(3, 85);
  digitalWrite(10, LOW);
}

void stopMotorRight() {
  digitalWrite(3, LOW);
  digitalWrite(10, LOW);
}

void forwardLeft() {
  analogWrite(6, 85);
  digitalWrite(5, LOW);
  //digitalWrite(6, HIGH);
}

void backwardLeft() {
  analogWrite(5, 85);
  //digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
}

void stopMotorLeft() {
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}

void leftTurn() {
  Serial.println("LEFT");
  backwardLeft();
  forwardRight();
  //delay(100);
}

void rightTurn() {
  Serial.println("RIGHT");
  backwardRight();
  forwardLeft();
  //delay(100);
}

void forward() {
  Serial.println("FORWARD");
  forwardRight();
  forwardLeft();
  //delay(100);
}

void backward() {
  Serial.println("BACKWARD");
  backwardRight();
  backwardLeft();
  //delay(100);
}

void stopAll() {
  Serial.println("STOP");
  stopMotorRight();
  stopMotorLeft();
  //delay(100);
}

void playBlue() {
  MP3player.play(2);
 // delay(500);
}

void playRed() {
  MP3player.play(3);
  //delay(500);
}

void playYellow() {
  MP3player.play(4);
  //delay(500);
}

void playBlack() {
  MP3player.play(5);
  //delay(500);
}

