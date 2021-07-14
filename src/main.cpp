#include <Arduino.h>
#include <Tone32.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define BUZZER_PIN 16
#define BUZZER_CHANNEL 0

int buzzer = 4;
int beat;


String serverName = "https://musicarduino-default-rtdb.firebaseio.com/";
/*MQTT*/
const char* ssid     = "";
const char* password = "";

const char *mqtt_server = "broker.hivemq.com";
const int   mqtt_port = 1883;
const char *mqtt_user = "usuario mqtt";
const char *mqtt_pass = "clave mqtt";
/**/
/************************************/




double getFrecuency(char note,int octave,int sharp){
  double keyn = 0;
  double numKey = 0;
  double frecu = 400; 
  double poww = 0;  
  switch (note)
  {
  case 'C':
    keyn = 4;
    
    break;
  case 'D':
    keyn = 6;
    break;
  case 'E':
    keyn = 8;
    break;
  case 'F':
    keyn = 9;
    break;
  case 'G':
    keyn = 11;
    break;
  case 'A':
    keyn = 13;
    break;
  case 'B':
    keyn = 15;
    break;
  
  default:
    break;
  }
  numKey = 12 * (octave-1) + keyn + sharp;
  poww = (numKey-49)/12; 
  frecu = round(pow(2,poww) * 440);
  return frecu;
}
int getBeat(int beat){
  return 400 * beat;
}
void playNote(const char* note,int beat){
  //
  String notr = String(note);
  int len = notr.length();
  int sharp = 0;
  int octave = String(note[1]).toInt();
  int beating;
  if(len > 2){
    //note with sharp
    sharp = 1;
    octave = String(note[2]).toInt();
  }
  double frecuenncy = getFrecuency(note[0],octave,sharp);
  Serial.println(frecuenncy);
  beating = getBeat(beat);
  delay(100);
  tone(BUZZER_PIN, (int) frecuenncy, beating, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL); 
}

void notesSong(char json[]){
  //char json[] = "[{\"note\":\"C4\",\"beat\":1},{\"note\":\"D4\",\"beat\":1},{\"note\":\"E4\",\"beat\":1},{\"note\":\"C4\",\"beat\":1},{\"note\":\"C4\",\"beat\":1},{\"note\":\"D4\",\"beat\":1},{\"note\":\"E4\",\"beat\":1},{\"note\":\"C4\",\"beat\":1}]";
  Serial.println(String(json));
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, json);
  
  for (int i = 0; i < doc.size(); i++)
  {
    beat = doc[i]["beat"];
    playNote(doc[i]["note"],beat);
  }
  
}
/*Methods wifi and MQTT*/

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
/**HTPP**/
 void httpGetMusic(String id){
    HTTPClient http;

      String serverPath = serverName + id +"/notes.json";
      Serial.println(serverPath);
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);

        char toJson[payload.length()];
        payload.toCharArray(toJson,payload.length());
        notesSong(toJson);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
 }
/********************/
void callback(char* topic, byte* payload, unsigned int length) {
  String mySong = "";
  Serial.print("Mensaje recibido bajo el tópico -> ");
  Serial.print(topic);
  Serial.print("\n");

  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    mySong = mySong + (char)payload[i];
    //Serial.println((char)payload[i]);
  }
  httpGetMusic(mySong);
  /* char toJson[mySong.length()];
  mySong.toCharArray(toJson,mySong.length()); */
  
  //notesSong(toJson);
  //Serial.println(mySong);

  /* if ((char)payload[0] == '0') {
    digitalWrite(2, LOW);
    Serial.println("\n Led Apagado");
  } else {
    digitalWrite(2, HIGH);
    Serial.println("\n Led Encendido");
  } */

  Serial.println();
}

void reconnect(){

  while (!client.connected()) {
    Serial.println("Intentando Conexión MQTT");

    String clientId = "iot_1_";
    clientId = clientId + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Conexión a MQTT exitosa!!!");
      //client.publish("salida", "primer mensaje");
      client.subscribe("testtopic/musicarduino");
    }else{
      Serial.println("Falló la conexión ");
      Serial.print(client.state());
      Serial.print(" Se intentará de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup_wifi(){
  // Conexión a la red Wifi
  Serial.println();
  Serial.println("Conectando a...");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());

}



void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  //notesSong();
  
  pinMode(2, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(client.connected()==false){
    reconnect();
  }

  client.loop();

  /* if (millis() - lastMsg > 2000){
    lastMsg = millis();

    //Este valor podría ser una temeperatura por ejemplo
    value++;
    String mes = "Valor -> " + String(value);
    mes.toCharArray(msg, 50);
    client.publish("salida", msg);
    Serial.println("Mensaje enviado -> " + String(value));
  } */
}


      
