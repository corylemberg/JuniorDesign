#include <ArduinoHttpClient.h>
#include <WiFi.h>
#include <String.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <FastLED.h>

// BATTERY LED DEFINITIONS
#define LED_PIN   12
#define NUM_LEDS  12
CRGB leds[NUM_LEDS];
float input;

// LED PINS
int RedLEDpinTest1 = 7;
int BlueLEDpinTest1 = 8;

// DFPLAYER DEFINITIONS
static const uint8_t D9 = 9;
static const uint8_t D11 = 11;
SoftwareSerial mySerial(D9, D11);
DFRobotDFPlayerMini MP3player;

// WEBSOCKET DEFINITIONS
WiFiClient wifi;
char ssid[] = "tufts_eecs";
char pass[] = "foundedin1883";
char serverAddress[] = "34.28.153.91";  // server address
int port = 80;
String clientID = "DCF2BCAB6F0B"; //Insert your Server ID Here!;
WebSocketClient client = WebSocketClient(wifi, serverAddress, port);

// MESSAGE AND COLOR ARRAYS
String ColorNamesString[4] = {"Yellow","Black","Red","Blue"};
String partnerIDn = "MAC_N_CHEESE_4465.";
String partnerID = "MAC_N_CHEESE";
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

// MESSAGE ACKNOWLEDGEMENT BOOLS
bool playBlueLaneFound = false;
bool BlueLaneAcknowledged = false;
bool Bot1finished = false;
bool Bot2finished = false;
bool redLaneNotFound = true;

// GENERAL VARIABLES
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
int delay90 = 1400;
int delay180 = 2*delay90;
int sameCount = 0;
int robustcolor = 0;
int buttonState = 0;
int previousButtonState = 0;
int stateVariablePrevious = -1;
int stateVariable = -1;



// the setup function runs once when you press reset or power the board
void setup() {

  // Initialize serials
  Serial.begin(9600);
  mySerial.begin(9600);
  input = 0;

  // Initialize LEDS and DFplayer
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  MP3player.begin(mySerial);
  MP3player.volume(30);
  delay(500);

  // Network connection
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

  // Pin declarations
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  previousButtonState = digitalRead(2);
  String Colors[] = {"Red","Blue", "Ambient"};
  
  // End setup
  Serial.print("printing Start Message: ");
  Serial.println(startMessage);

          Serial.println(startMessage.substring(0,12));
        Serial.println(startMessage.substring(18));
}

// FUNCTIONAL LOOP
void loop() {
  // Always process battery on every loop
  processBattery();
  
  // BEGIN STATE PROCEDURE
  if (stateVariablePrevious != stateVariable) {
    Serial.print("stateVariable: ");
    Serial.println(stateVariable);
  }
  
  if (parseMessage(bot1foundRed) && redLaneNotFound) {
    Serial.println("Bot 1 found Red Message received");
    stateVariable = 0;
    redLaneNotFound = false;
  }
  
  if (playBlueLaneFound){
    sendMessage(bot2BlueLaneFound);
    delay(10);
  } else if (Bot2finished) {
    sendMessage(Bot2finish);
    delay(10);
  }

  // Wait until bot 2 says go
  if (stateVariable == 0) {
      // FWD until wall
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
      delay(delay180);
      stopAll();
      stateVariable = 2;
  } 
  else if (stateVariable == 2){
    // fwd until red
      Serial.println("Looking for blue");
      forward();
      int bluecolor = robustColorSensing(currIndex);

      if (bluecolor == 4) {
        stopAll();
        forward();
        delay(200);
        Serial.println("TURNING...");
        rightTurn();
        delay(delay90-delay90/5);
        stopAll();
        delay(100);
        stateVariable = 3;
        // RETURNING
      }
    } 

    // LANE FOLLOW RED
    else if (stateVariable == 3){
      Serial.println("LANE FOLLOWING Blue");
      playBlueLaneFound = true;
      laneFollow(4); 
      if (parseMessage(bot1acknowledgeMessage))
      {
        BlueLaneAcknowledged = true;

      } 
      if (wallSensor()) {
        
          rightTurn();
          delay(delay90+delay90/8);
          stopAll();
          delay(100);
          stateVariable = 4;
      }
    }
    // FORWARD TO YELLOW
    else if (stateVariable == 4){  
      forward();   
      colorSensing(Sensing, currIndex);
      int colorFindYellow = pathColors[currIndex];
      if (colorFindYellow == 1) {
        forward();
        delay(500);
        rightTurn();
        delay(delay90-delay90/4);
        stopAll();
        //Wait until bot 1 has returned home.
        stateVariable = 5;
      }
    }

    // LANE FOLLOW YELOW
    else if (stateVariable == 5){  
      // WAIT UNTIL RECIEVED SIGNAL FROM BOT 1
      if (parseMessage(bot1returned))
      {
        Bot1finished = true;
      } 
      if (Bot1finished) {
      robustLaneFollow(1); 
        if (wallSensor()) {
          rightTurn();
          delay(delay90);
          stopAll();
          delay(100);
          stateVariable = 6;
        }
      }
    }

    // RETURN HOME
    else if (stateVariable == 6){
    forward();
    if (wallSensor()) {
      stopAll();
      //returned home
      Bot2finished = true;
      stateVariable = 7;
      }
    }

    // IDLE
    else if (stateVariable == 7){ 
      // WAIT to ACKNOWLEDGE BOT1's acknowledgement of bot 2's RETURN
      stopAll();
      if (parseMessage(Bot1finishAcknowledge)) {
        //playRed();
      }
    } 

  // Save previous state
  stateVariablePrevious = stateVariable;

  // index 
  currIndex++;
  if (currIndex > 3) {
    currIndex = 0;
  }
}

// SEND A STRING OVER WEBSOCKET
void sendMessage(String Message) {
  client.beginMessage(TYPE_TEXT);
  client.print(Message);
  client.endMessage();
}

// PARSE A STRING OVER WEBSOCKET, return valid message
bool parseMessage(String messageToLookFor) {
  
      int messageSize = client.parseMessage();
      if (messageSize > 0) {
        String message = client.readString();

        // SECURITY PROCEDURE: only parse if message matches our partner bot's ID
        bool fourNums = message.substring(0,12) == partnerID && message.substring(18) == messageToLookFor;
        bool threeNums = message.substring(0,12) == partnerID && message.substring(17) == messageToLookFor;
        bool twoNums = message.substring(0,12) == partnerID && message.substring(16) == messageToLookFor;
        bool oneNums = message.substring(0,12) == partnerID && message.substring(15) == messageToLookFor;
        bool nonums = message.substring(0,12) == partnerID && message.substring(13) == messageToLookFor;
        if (fourNums || threeNums || twoNums || oneNums || nonums)  {
          return true;
        } 
        else 
        {
          return false;
        }
  } else {
    return false;
  }
}

// COLLISION DETECTION, return if detected
bool wallSensor() {
  float wallSense = analogRead(A5) * (5.0/1023);
  Serial.print("wall sensor: ");
  Serial.println(wallSense);
  bool wall = false;
  if (wallSense >= 0.9) {
    wall = true;
    Serial.println("Detected wall (in wall sensor function)");
  }

  return wall;
}

//  COLOR SENSING
void colorSensing(float* Sensing, int currIndex) {
  // Sensing stores [red,blue,yellow]
  int plex[2] = {1,0};
  
  // LED sensing loop
  for (int i = 0; i < 2; i++) {
    
    digitalWrite(RedLEDpinTest1, plex[i]);
    digitalWrite(BlueLEDpinTest1, plex[(i+1)%2]);
      
    delay(50);
    float sensorValue = analogRead(A0) * (5.0/1023);
    
    Sensing[i] = sensorValue; ///The ambientValue;
    delay(10);    
    digitalWrite(BlueLEDpinTest1, 0);
    digitalWrite(RedLEDpinTest1, 0);   
  }
  
  float diff = Sensing[0] - Sensing[1];
  int color = 0;

  if (fabs(diff) < 1) {
    if (Sensing[0] < 2 && Sensing[1] < 2){
      color = 2;  //black
    } else {
    color = 1;    //yellow
    }
  } else if (diff > 0 ) {
    color = 3;    //red
  } else if (diff < 0) {
    color = 4;    //blue 
  }

  pathColors[currIndex] = color;
}

// MORE ROBUST COLOR SENSING: checks if colors stay constant, for lane follow
int robustColorSensing(int currIndex) {
  robustcolor = 0;
  sameCount = 0;
  colorSensing(Sensing, currIndex);

  for (int j = 0; j < pathColorsSize - 1; j++) {
    if (pathColors[j + 1] == pathColors[j]) {
      sameCount++;
    }
  }
  
  if (sameCount >= pathColorsSize - 1) {
       if (pathColors[0] == 2) {
      robustcolor = 2;   //black

    } else if (pathColors[0] == 1) {
      robustcolor = 1;   //yellow

    } else if (pathColors[0] == 3) {
      robustcolor = 3;  //red 

    } else if (pathColors[0] == 4) {
      robustcolor = 4;  //blue

    }

  }
  return robustcolor;
}


// LANE FOLLOW PROCEDURE W/ SWEEPING
int sweeperNumber = 8;
void laneFollow(int colorOfLane) {
  colorSensing(Sensing, currIndex);
  int color = pathColors[currIndex];

  // check color sensed and color of lane
  // if mismatch, sweep until lane is found
  if (color == colorOfLane) {
    sweeper = sweeperNumber;
    forward();
  } else {
    Serial.print("Sweeper Number: ");
    Serial.println(sweeper);
      if(color != prevColor) {
        falls++;
      }
    Serial.print("Falls Number: ");
    Serial.println(falls);
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

// LANE FOLLOWING PROCEDURE W/ ROBUST SENSING FUNCTION
void robustLaneFollow(int colorOfLane) {
  int color = robustColorSensing(currIndex);

  if (color == colorOfLane) {
    sweeper = sweeperNumber;
    forward();
  } else {
    Serial.print("Sweeper Number: ");
    Serial.println(sweeper);
      if(color != prevColor) {
        falls++;
      }
    Serial.print("Falls Number: ");
    Serial.println(falls);
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

// MOTOR CONTROL FUNCTIONS
int motorSpeed = 40;
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
  digitalWrite(6, LOW);
}

void stopMotorLeft() {
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}

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

// MP3 PLAYING FUNCTIONS: used in debugging
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

// STATE CHANGE FUNCTION
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

// RECEIVES BATTERY, UPDATES LEDs
void processBattery() {
  float input = analogRead(A2) * 5.0/1023;
  int charge = (input - 3.75) / 0.75 * 12.0;
  if(charge <= 0) {
    charge = 0;
  }
  else if(charge >= 12) {
    charge = 12;
  }
  for (int i = 0; i <= NUM_LEDS-1; i++) {
      if (i <= charge) {
        leds[i] = CRGB (80 - (charge * 6), charge * 6, 0);
      }
      else leds[i] = CRGB (0, 0, 0);
    FastLED.show();
  }
}






