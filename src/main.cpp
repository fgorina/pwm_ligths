#include <Arduino.h>

// Hi han 2 grans grips de llums, navigation.lights que son 1/0
// i electrival.lights.1.compass que es un valor entre 0 i 1 amb PWM
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>

#define TEST 0
#define DEBUG true
#define PWMRANGE 255

#define ssid "Yamato"
#define password "ailataN1991"



const char* signalk_server = "ws://192.168.1.2:3000/signalk/v1/stream?subscribe=none";

//const char* signalk_server = "ws://192.168.1.53:3000/signalk/v1/stream?subscribe=none";

const String login = "{\"requestId\": \"6986b469-ec48-408a-b3f5-660475208dca\", \"login\": { \"username\": \"pi\", \"password\": \"um23zap\" }}";

const String subscribe = "\", \"subscribe\": [{\"path\": \"electrical.lights.1.*.state\", \"policy\": \"instant\"},{\"path\": \"navigation.lights.*.state\", \"policy\": \"instant\"}]}";

const String update1 = "{ \"context\": \"";

using namespace websockets;

const int LED_COMPASS = 14;      // D5
const int LED_NAVIGATION = 15;  // D8
const int LED_MOTORING = 13;    // D7
const int LED_ANCHOR = 12;      // D6
const int LED_DECK = 5;        // D1
const int LED_INSTRUMENTS = 4;        // D2

const char* bigBuffer;

WebsocketsClient client;

unsigned int socketState = 0;
String me = "vessels.self";
String token;

// Must convert units!!!

void setCompass(float v) {
  int i = floor(v * 255.0);
  analogWrite(LED_COMPASS, i);
}

void sendLogin() {
  client.send(login);
}

void sendSubscribe() {

  String s = update1 + me + subscribe;
  client.send(s);
  if (DEBUG) { Serial.println(s); }
}

void onMessageCallback(WebsocketsMessage message) {
  if (DEBUG) { Serial.print("Got Message: "); }
  if (DEBUG) { Serial.println(message.data()); }
  bigBuffer = message.c_str();
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, bigBuffer);
  if (socketState == 0) {

    me = String(doc["self"]);
    if (DEBUG) { Serial.print("I am "); }
    if (DEBUG) { Serial.println(me); }
    socketState = 1;
    if (DEBUG) { Serial.println("Sending login"); }
    delay(100);

    sendLogin();

  } else if (socketState == 1) {

    if (doc["state"] == "COMPLETED" && doc["statusCode"] == 200) {
      if (DEBUG) { Serial.println("Login Completed"); }
      if (DEBUG) { Serial.print("Token: "); }
      token = String(doc["login"]["token"]);
      if (DEBUG) { Serial.println(token); }
      socketState = 2;
      delay(100);
      sendSubscribe();
      delay(100);
    }
  } else if (socketState == 2) {
    String somePath = String(doc["updates"][0]["values"][0]["path"]);
    Serial.print("Path received ");
    Serial.println(somePath);
    if (somePath == "electrical.lights.1.compass.state") {
      float v = doc["updates"][0]["values"][0]["value"];
      Serial.print(somePath);
      Serial.print(": ");
      Serial.println(v);
      setCompass(v);
      Serial.print("Setting compass light to ");
      Serial.println(v);
    } else if (somePath == "navigation.lights.navigation.state") {
      int v = doc["updates"][0]["values"][0]["value"];
      Serial.print(somePath);
      Serial.print(": ");
      Serial.println(v);
      digitalWrite(LED_NAVIGATION, v);
      Serial.print("Setting navigation lights to ");
      Serial.println(v);
    } else if (somePath == "navigation.lights.motoring.state") {
      int v = doc["updates"][0]["values"][0]["value"];
      Serial.print(somePath);
      Serial.print(": ");
      Serial.println(v);
      digitalWrite(LED_MOTORING, v);
      Serial.print("Setting motoring light to ");
      Serial.println(v);
    } else if (somePath == "navigation.lights.anchor.state") {
      int v = doc["updates"][0]["values"][0]["value"];
      Serial.print(somePath);
      Serial.print(": ");
      Serial.println(v);
      digitalWrite(LED_ANCHOR, v);
      Serial.print("Setting anchor light to ");
      Serial.println(v);
    } else if (somePath == "navigation.lights.deck.state") {
      int v = doc["updates"][0]["values"][0]["value"];
      Serial.print(somePath);
      Serial.print(": ");
      Serial.println(v);
      digitalWrite(LED_DECK, v);
      Serial.print("Setting deck light to ");
      Serial.println(v);
    }else if (somePath == "navigation.lights.instruments.state") {
      int v = doc["updates"][0]["values"][0]["value"];
      Serial.print(somePath);
      Serial.print(": ");
      Serial.println(v);
      digitalWrite(LED_INSTRUMENTS, v);
      Serial.print("Setting intruments light to ");
      Serial.println(v);
    }
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {

    if (DEBUG) { Serial.println("Wss Connnection Opened"); }
  } else if (event == WebsocketsEvent::ConnectionClosed) {

    if (DEBUG) { Serial.println("Wss Connnection Closed"); }
    socketState = 0;

  } else if (event == WebsocketsEvent::GotPing) {
    if (DEBUG) { Serial.println("Wss Got a Ping!"); }
  } else if (event == WebsocketsEvent::GotPong) {
    if (DEBUG) { Serial.println("Wss Got a Pong!"); }
  }
}

void setup_wifi() {

  socketState = 0;
  delay(10);
  // We start by connecting to a WiFi network
  if (DEBUG) { Serial.println(); }
  if (DEBUG) { Serial.print("Connecting to "); }
  if (DEBUG) { Serial.println(ssid); }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    //ESP.restart();
  }

  if (DEBUG) { Serial.println(""); }
  if (DEBUG) { Serial.print("WiFi Connected to "); }
  if (DEBUG) { Serial.println(ssid); }
  if (DEBUG) { Serial.print("IP address: "); }
  if (DEBUG) { Serial.println(WiFi.localIP()); }



  // Connecting to Signal K Server

  if (DEBUG) { Serial.print("Connecting to "); }
  if (DEBUG) { Serial.println(signalk_server); }


  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);

  socketState = 0;
  //client.connect(signalk_server);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_COMPASS, OUTPUT);
  pinMode(LED_NAVIGATION, OUTPUT);
  pinMode(LED_MOTORING, OUTPUT);
  pinMode(LED_ANCHOR, OUTPUT);
  pinMode(LED_DECK, OUTPUT);
  pinMode(LED_INSTRUMENTS, OUTPUT);

  bigBuffer = (char*)malloc(1024);

  // Connect to WiFi

  setup_wifi();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() != WL_CONNECTED && TEST == 0) {
    setup_wifi();
  }

  if (socketState == 0) {
    client.connect(signalk_server);
  }
  //ArduinoOTA.handle();
  client.poll();
}
