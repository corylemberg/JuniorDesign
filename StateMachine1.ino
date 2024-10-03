#include <ArduinoHttpClient.h>
#include <WiFi.h>

int buttonState = 0;
int previousButtonState = 0;
int stateVariable = 0;

char ssid[] = "tufts_eecs";
char pass[] = "foundedin1883";

char serverAddress[] = "34.28.153.91";  // server address
int port = 80;

WiFiClient wifi;
WebSocketClient client = WebSocketClient(wifi, serverAddress, port);
String clientID = "DCF2BCAB6F0B"; //Insert your Server ID Here!;

int status = WL_IDLE_STATUS;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
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
  pinMode(4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  previousButtonState = digitalRead(2);
}


void loop() {
  // int sensorValue = analogRead(A0);
  // float voltage = sensorValue * (5.0 / 1023.0);
  Serial.print("Wifi Status: ");
  Serial.println(WiFi.status());
  buttonState = digitalRead(2);
  if ((buttonState == HIGH) && (previousButtonState == LOW)) {
    Serial.println("STATE CHANGE");
    changeStateVariable();
    delay(50);
  }
  previousButtonState = buttonState;
  
  int messageSize = client.parseMessage();
    if (messageSize > 0) {
      Serial.println("Received a message:");
      String message = client.readString();
      Serial.println(message);
      if (message == "WebClient_4A9EDB0160D5.motors.255.-255") {
        forward();
      }
      else if (message == "WebClient_4A9EDB0160D5.motors.-255.255") {
        backward();
      }
      else if (message == "WebClient_4A9EDB0160D5.motors.0.0") {
        stopMotor();
      }
      else {
        stopMotor();
      }
    }
  //   // wait 10ms
  delay(10);

  // Serial.println("disconnected");         
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

void forward() {
  Serial.println("Forward");
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  delay(50);
}

void backward() {
  Serial.println("Backward");
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);
  delay(50);
}

void stopMotor() {
  Serial.println("Stop");
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  delay(50);
}

