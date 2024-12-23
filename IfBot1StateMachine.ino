#include <ArduinoHttpClient.h>
#include <WiFi.h>
#include <String.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <FastLED.h>

#define LED_PIN   12
#define NUM_LEDS  12

// Battery level LEDs and input pin initialization
CRGB leds[NUM_LEDS];
float input;

// Speaker/DFplayer pin initializations
static const uint8_t D9 = 9;
static const uint8_t D11 = 11;
SoftwareSerial mySerial(D9, D11);
DFRobotDFPlayerMini MP3player;

// Variable is incremented to drive state machine
int stateVariable = -1;

// Color sensing LED pin variables (red and blue), list to store raw color values
int RedLEDpinTest1 = 7;
int BlueLEDpinTest1 = 8;
float Sensing[3];

// WebSocket variables
char ssid[] = "tufts_eecs";
char pass[] = "foundedin1883";

char serverAddress[] = "34.28.153.91";  // server address
int port = 80;

WiFiClient wifi;
WebSocketClient client = WebSocketClient(wifi, serverAddress, port);
String clientID = "DCF2BCAB6F0B"; //Insert your Server ID Here!;

int status = WL_IDLE_STATUS;

// WebSocket encryption/information partner bot variables
String partnerID = "WebClient_MAC_N_CHEESE.";
String bot1foundRed = "bot_1_red_lane_found";
String startMessage = partnerID + bot1foundRed;
String bot2BlueLaneFound = "bot_2_blue_lane_found";
String bot1acknowledgeMessage = "bot_1_acknowledge_blue_lane_found";
String bot1acknowledgeMessageFull = partnerID + bot1acknowledgeMessage;
String bot1returned = "bot_1_returned";
String bot1returnedFull = partnerID + bot1returned;
String Bot1finishAcknowledge = "bot_1_acknowledge_bot_2_returned";
String Bot1finishAcknowledgeFull = partnerID + Bot1finishAcknowledge;

String Bot2finish = "bot_2_returned";

// Partner bot signals
bool playRedLaneFound = false;
bool BlueLaneAcknowledged = false;
bool Bot1finished = false;
bool Bot2finished = false;

// Robust color sensing counter, list initialization with size, turn "randomizer", previous color sensing to only save color if it doesn't change, and counter to store how many times same color is detected, color variable used to store color value
int currIndex = 0;
int pathColorsSize = 3;
int pathColors[3];
int falls = 0;
int prevColor = 0;
int sameCount = 0;
int robustcolor = 0;

// Lane following sweep amount, yellow lane following variables
int sweeper = 20;
int colorFindYellow = 0;
bool foundWallYellow = false;

// Turn calibrated delays
int delay90 = 2000;
int delay180 = 2*delay90;

// the setup function runs once when you press reset or power the board
void setup() {
  // Begins serial terminal and speaker driver
  Serial.begin(9600);
  mySerial.begin(9600);
  
  // Battery value input reset
  input = 0;

  // Battery LED strip display initialization and DFplayer waits for command
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  MP3player.begin(mySerial);
  MP3player.volume(30); // Sets volume
  delay(500);

  // Connects bot to wifi to communicate with other bot
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    // print the network name (SSID);
    Serial.println(ssid);                   

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

  // Prints client server ID
  Serial.println("starting WebSocket client");
  client.begin();
  client.beginMessage(TYPE_TEXT);
  client.print(clientID);
  client.endMessage();

  // Pins initialized depending on function
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(RedLEDpinTest1, OUTPUT);
  pinMode(BlueLEDpinTest1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  String Colors[] = {"Red","Blue", "Ambient"};
  
  Serial.print("printing Start Message: ");
  Serial.println(startMessage);
}


void loop() {
  // Displays battery level
  processBattery();
  
  // Tells bot 2 when red lane is found or blue path found acknowledged
  if (playRedLaneFound){
    sendMessage(bot1foundRed);
  } else if (BlueLaneAcknowledged) {
    sendMessage(bot1acknowledgeMessage);
  } else if (Bot1Finished) {
    sendMessage(bot1returned);
  }

  if (stateVariable == 0) {
      // fwd until wall
      forward();
      if (wallSensor()) {
        stopAll();
        delay(100);
        // Advance to next state
        stateVariable = 1;
      }
  } 
  else if (stateVariable == 1){
      // flip around
      rightTurn();
      delay(delay180);
      stopAll();
      stateVariable = 2;
  } 
  else if (stateVariable == 2){
      // fwd until red
      forward();
      int redcolor = robustColorSensing(currIndex);
      // When red is found...
      if (redcolor == 4) {
        // Reorient self to turn towards red line
        delay(200);
        playRed();
        leftTurn();
        delay(delay90-delay90/5);
        stopAll();
        delay(100);
        stateVariable = 3;
      }
  } 
  else if (stateVariable == 3){
    // lane follow red
    Serial.println("LANE FOLLOWING Red");
    //Send message that the red lane is found 
    playRedLaneFound = true;
    // Follow red lane
    laneFollow(3); 

    if (wallSensor()) {
      leftTurn();
      delay(delay90+delay90/8);
      stopAll();
      delay(100);
      stateVariable = 4;
    }
  }
  else if (stateVariable == 4){  
    // go fwd to find yellow
    forward();

    // Find yellow lane
    colorSensing(Sensing, currIndex);
    int colorFindYellow = pathColors[currIndex];

    if (colorFindYellow == 1) {
      // Turn so that bot is positioned in the middle of the lane
      forward();
      delay(500);
      playYellow();
      leftTurn();
      delay(delay90-delay90/4);
      stopAll();
      stateVariable = 5;
    }
  }
  else if (stateVariable == 5){ 
    // WAIT UNTIL RECIEVED SIGNAL FROM BOT 2
    if (parseMessage(bot2BlueLaneFound))
    {
      BlueLaneAcknowledged = true;
    } 
    if (BlueLaneAcknowledged) {
      // Lane follow the yellow path
      robustLaneFollow(1); 
      if (wallSensor()) {
        // When wall is found, turn 90 degrees
        leftTurn();
        delay(delay90);
        stopAll();
        delay(100);
        stateVariable = 6;
      }
    }
  }
  else if (stateVariable == 6){
    // Returns to original position by going forward
    forward();
    if (wallSensor()) {
      stopAll();
      //returned home
      Bot1finished = true;
      stateVariable = 7;
    }
  }
  else if (stateVariable == 7){ 
    // idle state
    // WAIT to ACKNOWLEDGE BOT2's return
    stopAll();
    if (parseMessage(Bot2finish)) {
      sendMessage(Bot1finishAcknowledge);
    }
  } 

  // Increments counter for robust color sensing
  currIndex++;
  if (currIndex > 3) {
    currIndex = 0;
  }
}

// Logic to display level of battery
void processBattery() {
  // Read in voltage from battery
  float input = analogRead(A2) * 5.0/1023;

  // Convert to comparison values
  int charge = (input - 3.75) / 0.75 * 12.0;
  
  if(charge <= 0) {
    charge = 0;
  }
  else if(charge >= 12) {
    charge = 12;
  }

  // Set LED strip values to corresponding charge value- Green, yellow, red
  for (int i = 0; i <= NUM_LEDS-1; i++) {
      if (i <= charge) {
        leds[i] = CRGB (80 - (charge * 6), charge * 6, 0);
      }
      else {
        leds[i] = CRGB (0, 0, 0);
      }
    // Display color
    FastLED.show();
  }
}

// Sends a message to WebSocket server
void sendMessage(String Message) {
  client.beginMessage(TYPE_TEXT);
  client.print(Message);
  client.endMessage();
}

// Reads messages in server sent by other bot
bool parseMessage(String messageToLookFor) {
  // Store current message on server
  int messageSize = client.parseMessage();
  
  // If message is present...
  if (messageSize > 0) {
    // Read message, report that message has been obtained if obtained, otherwise false
    String message = client.readString();
    Serial.println("Received the message: ");
    Serial.print(message);
    if (message = messageToLookFor) {
      Serial.println("returned true");
      return true;
    } 
    else 
    {
      Serial.println("returned false");
      return false;
    }
  } 
  else {
    return false;
  }
}

// Senses wall in front of bot using IR LED and sensor
bool wallSensor() {
  float wallSense = analogRead(A5) * (5.0/1023);
  bool wall = false;
  // Calibrated sensor amount
  if (wallSense >= 1.2) {
    wall = true;
    Serial.println("Detected wall (in wall sensor function)");
  }

  return wall;
}

// Finds current color under bot
void colorSensing(float* Sensing, int currIndex) {
  // multiplexer list that switches between red and blue 
  int plex[2] = {1,0};
  
  // Iterates through red and blue LEDs
  for (int i = 0; i < 2; i++) {
    // Turns on either blue or red at once depending on current MUX
    digitalWrite(RedLEDpinTest1, plex[i]);
    digitalWrite(BlueLEDpinTest1, plex[(i+1)%2]);
      
    delay(50);
    float sensorValue = analogRead(A0) * (5.0/1023);
    // Stores analog color reading in sensing, one index is blue reading, next is red
    Sensing[i] = sensorValue;
    delay(10);    
    digitalWrite(BlueLEDpinTest1, 0);
    digitalWrite(RedLEDpinTest1, 0);   
  }
  
  // Compute difference between red and blue color readings
  float diff = Sensing[0] - Sensing[1];
  
  // Initialize color value to be output
  int color = 0;

  // Float absolute value of difference between two color readings- correct values found through large set of experimental data
  if (fabs(diff) < 1) {
    if (Sensing[0] < 2 && Sensing[1] < 2){
      color = 2;
      //black
    } else {
      color = 1; 
      //yellow
    }
  } else if (diff > 0 ) {
    color = 3; 
    //red
  } else if (diff < 0) {
    color = 4;
    //blue 
  }
  // Fill robust list with current color
  pathColors[currIndex] = color;
}

// Robustly find color value- check if value is the same for multiple iterations of color sensing over time
int robustColorSensing(int currIndex) {
  robustcolor = 0;
  sameCount = 0;
  // Sense and store current color
  colorSensing(Sensing, currIndex);

  // Compare each value in list of colors to check if all colors match
  for (int j = 0; j < pathColorsSize - 1; j++) {
    if (pathColors[j + 1] == pathColors[j]) {
      // Increase same counter if pair of colors spotted
      sameCount++;
    }
  }
  
  // Checks that all colors are the same in the list
  if (sameCount >= pathColorsSize - 1) {
    // Outputs color
    if (pathColors[0] == 2) {
      robustcolor = 2; 
      //black
    } else if (pathColors[0] == 1) {
      robustcolor = 1; 
      //yellow
    } else if (pathColors[0] == 3) {
      robustcolor = 3;
      //red 

    } else if (pathColors[0] == 4) {
      robustcolor = 4; 
      //blue
    }
  }

  return robustcolor;
}


// Amount sweep occurs for
int sweeperNumber = 10;

void laneFollow(int colorOfLane) {
  // Finds which color is under bot
  colorSensing(Sensing, currIndex);
  int color = pathColors[currIndex];

  // Compares current color and color bot is supposed to be following
  if (color == colorOfLane) {
    // Go forward when colors are matched
    sweeper = sweeperNumber;
    forward();
  } else {
    // If colors do not match and the bot is veering off path
    if(color != prevColor) {
      // Increment falls variable 
      falls++;
    }

    // This logic is a way to make our lane following more intelligent- alternate which way is turned first during our sweeps
    // This way, there is a chance that we detect which way the bot drove off before we have to turn the other way
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
  // Save previous color for next iteration
  prevColor = color;
}

// Same lane following function, but instead uses more accurate color values
void robustLaneFollow(int colorOfLane) {
  int color = robustColorSensing(currIndex);

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

// Speed of motors- driven by PWM pins
int motorSpeed = 40;

// Single motor controls
void forwardRight() {
  analogWrite(10, motorSpeed);
  digitalWrite(3, LOW);
}

void backwardRight() {
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

// Puts motor controls together to move whole bot
void leftTurn() {
  backwardLeft();
  forwardRight();
}

void rightTurn() {
  backwardRight();
  forwardLeft();
}

void forward() {
  forwardRight();
  forwardLeft();
}

void backward() {
  backwardRight();
  backwardLeft();
}

void stopAll() {
  stopMotorRight();
  stopMotorLeft();
}

// Plays different color sounds for their duration
void playBlue() {
  MP3player.play(2);
  delay(3000);
  MP3player.stop();
}

void playRed() {
  MP3player.play(3);
  delay(3000);
  MP3player.stop();
}

void playYellow() {
  MP3player.play(4);
  delay(3000);
  MP3player.stop();
}

void playBlack() {
  MP3player.play(5);
  delay(3000);
  MP3player.stop();
}


// MAIN CODE ENDS HERE
// ALL FUNCTIONS BELOW THIS LINE ARE EITHER PAST CODE ITERATIONS OR USED FOR DEBUGGING PURPOSES



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

  //   Serial.println("STATE CHANGE");
  //   changeStateVariable();
  //   delay(50);
  // }
  // forward();
  
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
 

// void changeStateVariable() {
//   if (stateVariable == 6) {
//     stateVariable = 0;
//   } else {
//     stateVariable++;
//   }
//   Serial.println(stateVariable);
//   changeState();
// }

// void changeState() {
//   switch (stateVariable) {
//     case 0:  // your hand is on the sensor
//       Serial.println("Stop");
//       // forward();
//       break;
//     case 1:  // your hand is close to the sensor
//       Serial.println("Go Forward");
//       // backward();
//       break;
//     case 2:  // your hand is a few inches from the sensor
//       Serial.println("Go backward");
//       // forward();
//       break;
//     case 3:  // your hand is nowhere near the sensor
//       Serial.println("Pivot in place clockwise");
//       // backward();
//       break;
//     case 4:  // your hand is nowhere near the sensor
//       Serial.println("Pivot in place counterclockwise");
//       // forward();
//       break;
//     case 5:  // your hand is nowhere near the sensor
//       Serial.println("Right turn with desired radius");
//       // backward();
//       break;
//     case 6:  // your hand is nowhere near the sensor
//       Serial.println("Left turn with desired radius");
//       // forward();
//       break;
//   }
// }

    //forward();
    //   int robustcolor= robustColorSensing(currIndex);
    //   Serial.println(robustcolor);
    //   //int robustcolor = pathColors[currIndex];
    //    if (robustcolor == 2) {
    //   robustcolor = 2; //black
    //   playBlack();
    // } else if (robustcolor == 1) {
    //   robustcolor = 1; //yellow
    //   playYellow();
    // } else if (robustcolor == 3) {
    //   robustcolor = 3;//red 
    //   playRed();
    // } else if (robustcolor == 4) {
    //   robustcolor = 4; //blue
    //   playBlue();
    // }
    // if (pathColors[currIndex] == 2) {
    //   robustcolor = 2; //black
    //   playBlack();
    // } else if (pathColors[currIndex] == 1) {
    //   robustcolor = 1; //yellow
    //   playYellow();
    // } else if (pathColors[currIndex] == 3) {
    //   robustcolor = 3;//red 
    //   playRed();
    // } else if (pathColors[currIndex] == 4) {
    //   robustcolor = 4; //blue
    //   playBlue();
    // }
    
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

// void changeState2() {
//   switch (stateVariable) {
//     case 0:  // fwd until wall
//       // wait for bot 1
//       forward();
//       bool foundWall = wallSensor();
//       if (foundWall) {
//         stopAll();
//         stateVariable++;
//       }
//     case 1:  // flip around
//       leftTurn();
//       stopAll();
//       stateVariable++;
//     case 2:  // fwd until blue
//       forward();
//       int color = robustColorSensing(currIndex);
//       if (color == 4) {
//         stopAll();
//         //signal to bot 1 that blue is found here
//         stateVariable++;
//       }
//     case 3:  // lane follow blue
//       laneFollow(4); 
//       bool foundWallBlue = wallSensor();
//       if (foundWallBlue) {
//         rightTurn();
//         stopAll();
//         stateVariable++;
//       }
//     case 4:  // go fwd to find yellow
//       forward();
//       colorFindYellow = robustColorSensing(currIndex);
//       if (colorFindYellow == 1) {
//         rightTurn();
//         stopAll();
//         stateVariable++;
//       }
//     case 5:  // lane follow yellow
//       // WAIT UNTIL RECIEVED SIGNAL FROM BOT 2
//       laneFollow(1); 
//       bool foundWallYellow = wallSensor();
//       if (foundWallYellow) {
//         rightTurn();
//         forward();
//         stopAll();
//         stateVariable = 0;
//         // tell bot 1 we have returned
//       }
//     case 6:  // idle
//       stopAll();
//   }
// }



