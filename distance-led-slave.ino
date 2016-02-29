// This #include statement was automatically added by the Particle IDE.
#include "MQTT/MQTT.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkJson/SparkJson.h"



//define your mqtt credentials
#define DEVICE_ID "121fda0b-6a8d-43e4-839b-4effd647164b" 
#define MQTT_USER "121fda0b-6a8d-43e4-839b-4effd647164b" 
#define MQTT_PASSWORD "5ZUsjE7zXyg8"
#define MQTT_CLIENTID "TEh/aC2qNQ+SDm07/1kcWSw" //can be anything else
#define MQTT_TOPIC "/v1/121fda0b-6a8d-43e4-839b-4effd647164b/"
#define MQTT_SERVER "mqtt.relayr.io"


//A remote sensor we're subscribing to
#define SUBSCRIBED_DEVICE_ID "d06b56df-b471-456c-95d7-5e3c2e4202c5"
#define SUBSCRIBED_MQTT_USER "d06b56df-b471-456c-95d7-5e3c2e4202c5" 
#define SUBSCRIBED_MQTT_PASSWORD "WJM1xGYbPK0-"
#define SUBSCRIBED_MQTT_CLIENTID "T0GtW37RxRWyV1148LkICxQ" //can be anything else
#define SUBSCRIBED_MQTT_TOPIC "/v1/d06b56df-b471-456c-95d7-5e3c2e4202c5/"


int distancia;

const int led = D7;
const int meaningLen = 50;
int ledState = LOW;
char message_buff[100];
char meaning_buff[50];
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;

//led indicators
int indicador0 = D0, indicador1 = D1, indicador2 = D2, indicador3 = D3;


                                                 // Set here the time in milliseconds between publications
int publishingPeriod = 50;                     // ATTENTION !!!
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
 
 distancia = json["value"];
 
 
 
  //distancia = json["value"];
  Serial.println("The distace is: ");
  Serial.println(distancia);
  
}


void mqtt_connect() {
  Serial.println("Connecting to mqtt server");
  if (client.connect(SUBSCRIBED_MQTT_CLIENTID, SUBSCRIBED_MQTT_USER, SUBSCRIBED_MQTT_PASSWORD)) {
    Serial.println("Connection success, subscribing to topic");
    //subscribe to a topic, noiseLevel in this case...
    client.subscribe("/v1/"SUBSCRIBED_DEVICE_ID"/#");
  }
  else {
    Serial.println("Connection failed, check your credentials or wifi");
  }
}

void setup() {
  RGB.control(true);
  Serial.begin(9600);
  Serial.println("Hello There, I'm your Photon!");
  //setup our LED pin and connect to mqtt broker
  pinMode(led, OUTPUT);
  pinMode(indicador0, OUTPUT);
  pinMode(indicador1, OUTPUT);
  pinMode(indicador2, OUTPUT);
  pinMode(indicador3, OUTPUT);
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
   
       publish("distance",distancia);
      
      
    }
    
    if (distancia < 10)
    {
        digitalWrite(indicador3, HIGH);
        digitalWrite(indicador2, LOW);
        digitalWrite(indicador1, LOW);
        digitalWrite(indicador0, LOW);
    }    
    else if (distancia >= 10 && distancia < 20)
    {
        digitalWrite(indicador3, HIGH);
        digitalWrite(indicador2, HIGH);
        digitalWrite(indicador1, LOW);
        digitalWrite(indicador0, LOW);
    }
    else if (distancia >= 20 && distancia < 30)
    {
        digitalWrite(indicador3, HIGH);
        digitalWrite(indicador2, HIGH);
        digitalWrite(indicador1, HIGH);
        digitalWrite(indicador0, LOW);
    }
    else if (distancia >= 30)
    {
        digitalWrite(indicador3, HIGH);
        digitalWrite(indicador2, HIGH);
        digitalWrite(indicador1, HIGH);
        digitalWrite(indicador0, HIGH);
    }

    else
    {
        digitalWrite(indicador3, LOW);
        digitalWrite(indicador2, LOW);
        digitalWrite(indicador1, LOW);
        digitalWrite(indicador0, LOW);
    }
    
    blink(publishingPeriod / 2);
  }
  else {
    //if connection lost, reconnect
    Serial.println("retrying...");
    mqtt_connect();
  }
  
  

//   else if (distancia > 15 && distancia < 20)
//     {
//     digitalWrite(indicador3, HIGH);
//     digitalWrite(indicador2, HIGH);
//     }
//   
  
//   else
//     {
//     digitalWrite(indicador3, LOW);
//     digitalWrite(indicador2, LOW);

//     }    
  /*
  else {
  
      for(pos = 180; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
     {                                
       myservo.write(pos);              // tell servo to go to position in variable 'pos' 
       delay(15);                       // waits 15ms for the servo to reach the position 
     }  
  }
  */
  
////////////////////////////////////////////////////////////////  
  
}

void publish(char* meaning, int val) {
  //create our json payload as {"meaning":"moisture","value":sensorvalue}
  StaticJsonBuffer<100> pubJsonBuffer;
  JsonObject& pubJson = pubJsonBuffer.createObject();
  //define a meaning for what we're sending
  snprintf(meaning_buff, meaningLen,"%s", meaning);
  pubJson["meaning"] = meaning_buff;   //"humidity";
  //set our value key to the sensor's reading
  pubJson["value"] = val;   //analogRead(sensorPin);
  //copy our json object as a char array and publish it via mqtt
  char message_buff[100];
  pubJson.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));
}

// void subscribe() {
//   //create our json payload as {"meaning":"moisture","value":sensorvalue}
//   StaticJsonBuffer<100> pubJsonBuffer;
//   JsonObject& pubJson = pubJsonBuffer.createObject();
//   //define a meaning for what we're sending
//   pubJson["meaning"] = "distance";
//   //set our value key to the sensor's reading
//   pubJson["value"] = sound;
//   //copy our json object as a char array and publish it via mqtt
//   char message_buff[100];
//   pubJson.printTo(message_buff, sizeof(message_buff));
//   client.subscribe("/v1/"SUBSCRIBED_DEVICE_ID"/data", message_buff);
//   Serial.println("Publishing " + String(message_buff));
// }

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



