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
int sweeper = 20;

int movePrev = 0;
int pathColorsSize = 3;
int pathColors[3];
int falls = 0;
int findRed = 0;

float Sensing[3];

int colorFindYellow = 0;
bool foundWallYellow = false;

int prevColor = 0;
int delay90 = 1350;
int delay180 = 2*delay90;
int sameCount = 0;
int robustcolor = 0;
String ColorNamesString[4] = {"Yellow","Black","Red","Blue"};



// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600);
  mySerial.begin(9600);
  
  MP3player.begin(mySerial);

  MP3player.volume(30);
  delay(500);
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println("starting WebSocket client");
  client.begin();
  client.beginMessage(TYPE_TEXT);
  client.print(clientID);
  client.endMessage();

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
      Serial.print("stateVariable: ");
      Serial.println(stateVariable);
  if (stateVariable == 0) {
      // fwd until wall
      forward();
      if (wallSensor()) {
        stopAll();
        delay(100);
        stateVariable = 1;
      }
  } 
  else if (stateVariable == 1){
  // flip around
      leftTurn();
      delay(delay180 + 800);
      stopAll();
      stateVariable = 2;
  } 
  else if (stateVariable == 2){
    // fwd until red
      forward();
      int redcolor = robustColorSensing(currIndex);

      if (redcolor == 4) {
        // stopAll();
        // delay(100);
        playBlue();
        Serial.println("TURNING...");
        leftTurn();
        delay(delay90);
        stopAll();
        delay(100);
        stateVariable = 3;
        // return;
        //signal to bot 2 that red is found here
      }
    } 
    else if (stateVariable == 3){// lane follow red
      Serial.println("LANE FOLLOWING RED");
      laneFollow(4); 
      if (wallSensor()) {
        rightTurn();
        delay(delay90 + 100);
        stopAll();
        delay(300);
        stateVariable = 4;
      }
    }
    else if (stateVariable == 4){  // go fwd to find yellow
      forward();
      //stopAll();
      //int colorFindYellow = robustColorSensing(currIndex);
      
      colorSensing(Sensing, currIndex);
      int colorFindYellow = pathColors[currIndex];
      // Serial.println(colorFindYellow);
      if (colorFindYellow == 1) {
        forward();
        delay(500);
        playYellow();
        rightTurn();
        delay(delay90 + 800);
        stopAll();
        delay(100);
        forward();
        delay(50);
        stopAll();
        delay(50);
        stateVariable = 5;
      }
    }
    else if (stateVariable == 5){  // lane follow yellow
      // WAIT UNTIL RECIEVED SIGNAL FROM BOT 2
      Serial.println("LANE FOLLOWING YELLOW");
      laneFollow(1); 
      if (wallSensor()) {
        rightTurn();
        delay(delay90);
        stopAll();
        delay(100);
        stateVariable = 6;
        // tell bot 2 we have returned
      }
    }
    else if (stateVariable == 6){
    forward();
    if (wallSensor()) {
      stopAll();
      stateVariable = 7;
      }
    }
    else if (stateVariable == 7){ // idle
      // WAIT to ACKNOWLEDGE BOT2 RETURN
      stopAll();
    }
    else {
      stopAll();
    }
  // // Serial.println("COLOR: ");
  // // Serial.println(r);
  // // Serial.println(robustColorSensing(currIndex));
  // // robustColorSensing(currIndex);
  //colorSensing(Sensing, currIndex);
  //   for (int i = 0; i < 3; i++){
  
  //     Serial.print(Sensing[i]);
  //     Serial.print("  :  ");
  //   }
  //   Serial.print(" diff: ");
  // Serial.println(Sensing[0]-Sensing[1]);
  //Serial.println(ColorNamesString[pathColors[currIndex] - 1]);
  // for (int i = 0; i < pathColorsSize; i++){
  
  // Serial.print(pathColors[i]);
  // Serial.print("  :  ");
  // }
  //Serial.print("\n");
  currIndex++;
  if (currIndex > 3) {
    currIndex = 0;
  }
 //delay(200);
}

bool wallSensor() {
  float wallSense = analogRead(A5) * (5.0/1023);
  bool wall = false;
  if (wallSense >= 1.2) {
    wall = true;
    Serial.println("Detected wall (in wall sensor function)");
  }

  return wall;
}

void colorSensing(float* Sensing, int currIndex) {
  // Sensing stores [red,blue,yellow]
  int plex[2] = {1,0};
  
  // delay(50);
  // float ambientValue = analogRead(A0) * (5.0/1023);
  // delay(10);
 // Sensing[2] = ambientValue;
  for (int i = 0; i < 2; i++) {
    
    digitalWrite(RedLEDpinTest1, plex[i]);
    digitalWrite(BlueLEDpinTest1, plex[(i+1)%2]);
      
    delay(50);
    float sensorValue = analogRead(A0) * (5.0/1023);
    
    Sensing[i] = sensorValue;///ambientValue;
    delay(10);    
    digitalWrite(BlueLEDpinTest1, 0);
    digitalWrite(RedLEDpinTest1, 0);   
  }
  

  float diff = Sensing[0] - Sensing[1];
  int color = 0;
  // Serial.print("Diff Value: ");
  // Serial.println(diff);
  if (fabs(diff) < 1) {
    if (Sensing[0] < 2 && Sensing[1] < 2){
      color = 2;//black
    } else {
    color = 1; //yellow
    }
  } else if (diff > 0 ) {
    color = 3; //red
  } else if (diff < 0) {
    color = 4;//blue 
  }

  pathColors[currIndex] = color;
}

int robustColorSensing(int currIndex) {
  robustcolor = 0;
  sameCount = 0;
  colorSensing(Sensing, currIndex);


  for (int j = 0; j < pathColorsSize - 1; j++) {
    // Serial.println(pathColors[j]);
    // Serial.println(pathColors[j + 1]);
    if (pathColors[j + 1] == pathColors[j]) {
      sameCount++;
      // Serial.println(sameCount);
    }
  }
  
  if (sameCount >= pathColorsSize - 1) {

    // currIndex = 0;
    //Serial.println("Robust Sensed");
    if (pathColors[0] == 2) {
      robustcolor = 2; //black
    } else if (pathColors[0] == 1) {
      robustcolor = 1; //yellow
    } else if (pathColors[0] == 3) {
      robustcolor = 3;//red 
    } else if (pathColors[0] == 4) {
      robustcolor = 4; //blue
    }

  }

  // Serial.println(ColorNamesString[robustcolor-1]);

  return robustcolor;
}


int sweeperNumber = 6;

void laneFollow(int colorOfLane) {
  // float Sensing[3];
  colorSensing(Sensing, currIndex);
  int color = pathColors[currIndex];
  // Serial.print("COLOR LANE FOLLOWING: ");
  // Serial.println(color);
  if (color == colorOfLane) {
    sweeper = sweeperNumber;
    forward();
  } else {
      if(color != prevColor) {
        falls++;
      }
      if(sweeper > sweeperNumber*1/2) {
        if (falls % 2 == 0) {
          leftTurn();
        }
        else {
          rightTurn();
        }
      }
      else {
        if (falls % 2 == 0) {
          rightTurn();
        }
        else {
          leftTurn();
        }
      }
      sweeper -= 1;
  }
  prevColor = color;
}

// void changeStateVariable() {
//   if (stateVariable == 6) {
//     stateVariable = 0;
//   } else {
//     stateVariable++;
//   }
//   Serial.println(stateVariable);
//   changeState();
// }
//180 degree flip time delay
// void changeState1(int state) {
//   if (state == 0) {
//       // fwd until wall
//       Serial.print("state: ");
//       Serial.println(state);
//       forward();
//       if (wallSensor()) {
//         stopAll();
//         state = 1;
//       }
//   } else if (state == 1){
//   // flip around
//       rightTurn();
//       delay(delay180);
//       stopAll();
//       state = 2;
//   }
//     case 2:  // fwd until red
//       forward();
//       int color = colorSensing(Sensing);
//       if (color == 3) {
//         leftTurn();
//         delay(delay90);
//         stopAll();
//         //signal to bot 2 that red is found here
//         state = 3;
//       }
//     case 3:  // lane follow red
//       laneFollow(3); 
//       if (wallSensor()) {
//         leftTurn();
//         delay(delay90);
//         stopAll();
//         state = 4;
//       }
//     case 4:  // go fwd to find yellow
//       forward();
//       colorFindYellow = colorSensing(Sensing);
//       if (colorFindYellow == 1) {
//         leftTurn();
//         delay(delay90);
//         stopAll();
//         state = 5;
//       }
//     case 5:  // lane follow yellow
//       // WAIT UNTIL RECIEVED SIGNAL FROM BOT 2
//       laneFollow(1); 
//       if (wallSensor()) {
//         leftTurn();
//         delay(delay90);
//         stopAll();
//         state = 6;
//         // tell bot 2 we have returned
//       }
//     case 6:
//     forward();
//     if (wallSensor()) {
//       stopAll();
//       state = 7;
//     }
//     case 7:  // idle
//       // WAIT to ACKNOWLEDGE BOT2 RETURN
//       stopAll();

//   }
// }

void changeState2() {
  switch (stateVariable) {
    case 0:  // fwd until wall
      // wait for bot 1
      forward();
      bool foundWall = wallSensor();
      if (foundWall) {
        stopAll();
        stateVariable++;
      }
    case 1:  // flip around
      leftTurn();
      stopAll();
      stateVariable++;
    case 2:  // fwd until blue
      forward();
      int color = robustColorSensing(currIndex);
      if (color == 4) {
        stopAll();
        //signal to bot 1 that blue is found here
        stateVariable++;
      }
    case 3:  // lane follow blue
      laneFollow(4); 
      bool foundWallBlue = wallSensor();
      if (foundWallBlue) {
        rightTurn();
        stopAll();
        stateVariable++;
      }
    case 4:  // go fwd to find yellow
      forward();
      colorFindYellow = robustColorSensing(currIndex);
      if (colorFindYellow == 1) {
        rightTurn();
        stopAll();
        stateVariable++;
      }
    case 5:  // lane follow yellow
      // WAIT UNTIL RECIEVED SIGNAL FROM BOT 2
      laneFollow(1); 
      bool foundWallYellow = wallSensor();
      if (foundWallYellow) {
        rightTurn();
        forward();
        stopAll();
        stateVariable = 0;
        // tell bot 1 we have returned
      }
    case 6:  // idle
      stopAll();
  }
}


int motorSpeed = 70;
void forwardRight() {
  analogWrite(10, motorSpeed);
  digitalWrite(3, LOW);
  //digitalWrite(4, HIGH);
}

void backwardRight() {
  // digitalWrite(3, HIGH);
  analogWrite(3, motorSpeed);
  digitalWrite(10, LOW);
}

void stopMotorRight() {
  digitalWrite(3, LOW);
  digitalWrite(10, LOW);
}

void forwardLeft() {
  analogWrite(6, motorSpeed);
  digitalWrite(5, LOW);
  //digitalWrite(6, HIGH);
}

void backwardLeft() {
  analogWrite(5, motorSpeed);
  //digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
}

void stopMotorLeft() {
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}

void leftTurn() {
  // Serial.println("LEFT");
  backwardLeft();
  forwardRight();
  //delay(100);
}

void rightTurn() {
  // /Serial.println("RIGHT");
  backwardRight();
  forwardLeft();
  //delay(100);
}

void forward() {
  // Serial.println("FORWARD");
  forwardRight();
  forwardLeft();
  //delay(100);
}

void backward() {
  // Serial.println("BACKWARD");
  backwardRight();
  backwardLeft();
  //delay(100);
}

void stopAll() {
  //Serial.println("STOP");
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

  // rightTurn();
  // delay(delay90);
  // stopAll();
  
  // delay(delay90);
  // leftTurn();
  // delay(delay180);
  // stopAll();
  
  // delay(delay90);
  
    //digitalWrite(2, HIGH);
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



  //pathColors[currIndex % pathColorsSize] = colorSensing(Sensing);

  //  Serial.print("Difference: ");
  //  Serial.println(Sensing[0] - Sensing[1]);
   //Serial.print("RED: ");
   //Serial.print(Sensing[0]);
   //Serial.print("  BLUE: ");
   //Serial.print(Sensing[1]);
   //Serial.print("  AMBIENT VALUE: ");
   //Serial.print(Sensing[2]);
   //Serial.print("\n");
  //float diff = Sensing[0] - Sensing[1];
  // 
  // if ((fabs(diff) < 3 && fabs(diff) > 0.3)) {
  //   pathColors[currIndex % pathColorsSize] = 2; //black
  // } else if (fabs(diff) < 0.3) {
  //   pathColors[currIndex % pathColorsSize] = 1; //yellow
  // } else if (diff > 3.1) {
  //   pathColors[currIndex % pathColorsSize] = 3;//red blue
  // } else if (diff < -8) {
  //   pathColors[currIndex % pathColorsSize] = 4;
  // }

    // int sameCount = 0;
    // for (int j = 0; j < pathColorsSize; j++) {
    //   // Serial.println(pathColors[j]);
    //   // Serial.println(pathColors[j + 1]);
    //   if (pathColors[j + 1] == pathColors[j]) {
    //     sameCount++;
    //   }
    // }

    // Serial.println(sameCount);

    // if (sameCount >= pathColorsSize-1) {
    //   move = true;
    //   //Serial.println("HEEEEEEEERE");
    //   //Serial.println("Robust Sensed");
    // }
    // else {
    //   move = false;
    // }
    //Serial.println(move);
  // float wallSense = analogRead(A5) * (5.0/1023);
  // Serial.print("wallSenseValue: ");
  // Serial.println(wallSense);
  // put your main code here, to run repeatedly:

  // if (wallSensor()) {
  //   Serial.println("WALL:");
  //  stopAll();
  // }
  // else {
  //   laneFollow(3); 
  //   }
    // if (move == true) {
    //  if (pathColors[0] == 1) {
    //    if (movePrev != pathColors[0]) {
    //      playYellow();
    //    }
    //    Serial.println("YELLOW");
    //    movePrev = 1;
    //    leftTurn();
    //  }
    //  else if (pathColors[0] == 2) {
    //    if (movePrev != pathColors[0]) {
    //      playBlack();
    //    }
    //    Serial.println("BLACK");
    //    movePrev = 2;
    //    stopAll();
    //  }
    //  else if (pathColors[0] == 3) {
    //    if (movePrev != pathColors[0]) {
    //      playRed();
    //    }
    //    Serial.println("RED");
    //    movePrev = 3;
    //    rightTurn();
    //  }
    //  else if (pathColors[0] == 4) {
    //    if (movePrev != pathColors[0]) {
    //      playBlue();
    //    }
    //    Serial.println("BLUE");
    //    movePrev = 4;
    //    forward();
    //  }
    //}
    // if (move == true) {
    //   if (pathColors[0] == 1) {
    //     if (movePrev != pathColors[0]) {
    //       playYellow();
    //     }
    //     Serial.println("YELLOW");
    //     movePrev = 1;
    //     leftTurn();
    //   }
    //   else if (pathColors[0] == 2) {
    //     if (movePrev != pathColors[0]) {
    //       playBlack();
    //     }
    //     Serial.println("BLACK");
    //     movePrev = 2;
    //     stopAll();
    //   }
    //   else if (pathColors[0] == 3) {
    //     if (movePrev != pathColors[0]) {
    //       playRed();
    //     }
    //     Serial.println("RED");
    //     movePrev = 3;
    //     rightTurn();
    //   }
    //   else if (pathColors[0] == 4) {
    //     if (movePrev != pathColors[0]) {
    //       playBlue();
    //     }
    //     Serial.println("BLUE");
    //     movePrev = 4;
    //     forward();
    //   }
    // }
 
