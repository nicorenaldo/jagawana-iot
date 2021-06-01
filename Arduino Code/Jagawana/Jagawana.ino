#include "Arduino.h"
#include <FS.h>
#include "Wav.h"
#include "I2S.h"
#include <SD.h>
#include "WiFi.h"
#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 2048

const char* ssid = "M2000";
const char* password = "11223344";
const char* mqtt_server = "192.168.43.71";

WiFiClient espClient;
PubSubClient client(espClient);

#define I2S_MODE I2S_MODE_ADC_BUILT_IN

const int record_time = 10;  // second
const char filename[] = "/test.wav";

const int headerSize = 44;
const int waveDataSize = record_time * 44100 * 16 * 1 / 8;
//const int waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData/4;
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];

int state=0;
unsigned long lastMillis = 0;

constexpr size_t bufferSize = 512;
byte bufferr[bufferSize];

File file;

void setup() {
  Serial.begin(115200);

  // SD Card Init
  if (!SD.begin()) Serial.println("SD begin failed");
  while(!SD.begin()){
    Serial.print(".");
    delay(500);
  }
  delay(10);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
  client.publish("res/status","on");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()){
    client.connect("ESP32");
  }
  if (millis() - lastMillis > 3000){
    digitalWrite(2, !state);
    lastMillis=millis(); 
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if(messageTemp=="record"){
    record();
  }
  else if(messageTemp=="send"){
    sendfile();
  }
}

void record(){
  // Creating Wave Header
  Serial.println("Record Begin");
  client.publish("res/status","record-begin");
  CreateWavHeader(header, waveDataSize);
  SD.remove(filename);
  file = SD.open(filename, FILE_WRITE);  

  if (!file) return;
  file.write(header, headerSize);

  I2S_Init(I2S_MODE, I2S_BITS_PER_SAMPLE_32BIT);
  for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
    I2S_Read(communicationData, numCommunicationData);
    for (int i = 0; i < numCommunicationData/8; ++i) {
      partWavData[2*i] = communicationData[8*i + 2];
      partWavData[2*i + 1] = communicationData[8*i + 3];
    }
    file.write((const byte*)partWavData, numPartWavData);
  }
  file.close();
  client.publish("res/status","record-done");
  Serial.println("Record Done");
}

void sendfile(){
  client.publish("res/status","send-begin");
  Serial.println("Send Begin");
  file = SD.open(filename);
  int i = 0;
  if (file) {
    unsigned long start = millis();
    while (file.available()) {
      i++;
      file.read(bufferr, bufferSize);
      client.publish("res/data", bufferr, bufferSize);
      delay(10);
    }
    file.close();
    client.publish("res/status",String(i).c_str());
    client.publish("res/status",String(millis()-start).c_str());
    client.publish("res/status","send-done");
    Serial.println("Send Done");
  }
  else {
    client.publish("res/status","send-error");
    Serial.println("Dump file error.........");
    file.close();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32")) {
      Serial.println("connected");  
      client.subscribe("req");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
