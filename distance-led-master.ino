// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkJson/SparkJson.h"

// This #include statement was automatically added by the Particle IDE.
#include "MQTT/MQTT.h"



//define your mqtt credentials
#define DEVICE_ID "d06b56df-b471-456c-95d7-5e3c2e4202c5" 
#define MQTT_USER "d06b56df-b471-456c-95d7-5e3c2e4202c5" 
#define MQTT_PASSWORD "WJM1xGYbPK0-"
#define MQTT_CLIENTID "T0GtW37RxRWyV1148LkICxQ" //can be anything else
#define MQTT_TOPIC "/v1/d06b56df-b471-456c-95d7-5e3c2e4202c5/"
#define MQTT_SERVER "mqtt.relayr.io"


#define DHTPIN 0
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//Defining Ultrasonic ranger sensor variables

const int pingPin = D4;

const int led = D7;
int ledState = LOW;
char message_buff[100];
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;
 
//for publish mutiple times
char meaning_buff[50];
const int meaningLen = 50;


                                                // Set here the time in milliseconds between publications
int publishingPeriod = 200;					// ATTENTION !!!
                                                // DO NOT try to set values under 200 ms of the server
                                                // will kick you out


void callback(char* topic, byte* payload, unsigned int length);

//create our instance of MQTT object
MQTT client(MQTT_SERVER, 1883, callback);

//implement our callback method thats called on receiving data from a subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  //store the received payload and convert it to string
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  //print the topic and the payload received
  Serial.println("topic: " + String(topic));
  Serial.println("payload: " + String(p));
  //call our method to parse and use the payload received
  handlePayload(p);
}


void handlePayload(char* payload) {
  StaticJsonBuffer<200> jsonBuffer;
  //convert payload to json
  JsonObject& json = jsonBuffer.parseObject(payload);
  if (!json.success()) {
    Serial.println("json parsing failed");
    return;
  }
  //get value of the key "command"
  const char* command = json["command"];
  Serial.println("parsed command: " + String(command));
  if (String(command).equals("color"))
  {
    const char* color = json["value"];
    Serial.println("parsed color: " + String(color));
    String s(color);
    if (s.equals("red"))
      RGB.color(255, 0, 0);
    else if (s.equals("blue"))
      RGB.color(0, 0, 255);
    else if (s.equals("green"))
      RGB.color(0, 255, 0);
  }
}


void mqtt_connect() {
  Serial.println("Connecting to mqtt server");
  if (client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("Connection success, subscribing to topic");
    //subscribe to a topic
    client.subscribe("/v1/"DEVICE_ID"/cmd");
  }
  else {
    Serial.println("Connection failed, check your credentials or wifi");
  }
}

void setup() {
  RGB.control(true);
  Serial.begin(9600);
  Serial.println("Hello There, I'm your photon!");
  //setup our LED pin and connect to mqtt broker
  pinMode(led, OUTPUT);
  //set 200ms as minimum publishing period
  publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;
  mqtt_connect();
 
  
}

void loop() {
  if (client.isConnected()) {
    client.loop();
    //publish within publishing period
    if (millis() - lastPublishTime > publishingPeriod) {
      lastPublishTime = millis();
      
    //   publish("temperature",abs(dht.getTempCelcius()));
    //   publish("humidity",abs(dht.getHumidity()));
      
      PublishDist();
      
    }
    blink(publishingPeriod / 2);
  }
  else {
    //if connection lost, reconnect
    Serial.println("retrying..");
    mqtt_connect();
  }

  
}

void publish (char* meaning, float val) {
    //create our json payload as {"meaning":"moisture","value":sensorvalue}
    StaticJsonBuffer<100> pubJsonBuffer;
    JsonObject& pubJson = pubJsonBuffer.createObject();
    //define a meaning for what we're sending
    
    snprintf(meaning_buff, meaningLen,"%s", meaning);
    pubJson["meaning"] = meaning_buff;  //"humidity";
    //set our value key to the sensor's reading
    pubJson["value"] = val; //analogRead(sensorPin);
    //copy our json object as a char array and publish it via mqtt
    char message_buff[100];
    pubJson.printTo(message_buff, sizeof(message_buff));
    client.publish("/v1/"DEVICE_ID"/data", message_buff);
    Serial.println("Publishing " + String(message_buff));
}

void PublishDist() {
  //create our json payload as {"meaning":"moisture","value":sensorvalue}
  StaticJsonBuffer<100> pubJsonBuffer;
  JsonObject& pubJson = pubJsonBuffer.createObject();
  
  long c, duration, cm, ctte;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  
  pinMode(pingPin, INPUT);
  
  duration = ((pulseIn(pingPin, HIGH)))/2;
  
  //Sound velocity
  c = 331.4 + (0.606 * dht.getTempCelcius()) + (0.0124 * dht.getHumidity());
  
   //define a meaning for what we're sending
  pubJson["meaning"] = "distance";
  //set our value key to the sensor's reading
  pubJson["value"] = (c * duration)/10000;
  //pubJson["value"] = duration;
  
  //copy our json object as a char array and publish it via mqtt
  char message_buff[100];
  pubJson.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));
}


void blink(int interval) {
  if (millis() - lastBlinkTime > interval) {
    // save the last time you blinked the LED
    lastBlinkTime = millis();
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    // set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
}

