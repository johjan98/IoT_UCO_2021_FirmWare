#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <sstream>

const char* ssid = "*******";
const char* password =  "******";
const char* mqttServer = "*******";
const int mqttPort = 1883;
const char* mqttUser = "******";
const char* mqttPassword = "*******";
const char* mqttdoor = "triggeropendoor";
const char* mqttstatus = "doorstatus";    //Topic donde se publica el estado del actuador

const int act=5;
const int sens=14;
const int buz=4;
boolean sensorstat;
boolean servstat;

WiFiClient espClient;
PubSubClient client(espClient);

/* Configuración para conectar a la red WiFi
 Imprime la dirección IP cuando se logra la conexión. */
void setup_Wifi (){
  delay(10);
  analogWriteFreq(100);

  Serial.println();
  Serial.print(F("Connecting to ")) ;
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));    // Mientras no haya conexión se imprime '.' cada 500ms.
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int obtJson(String message){
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc,message);
  if (error){return 2;}
  String pattern = doc["pattern"];
  String data = doc["data"];
  return (int) data[0]-48;
}

/*  Función para accionar un acturador. Si "action" es igual a 1 el actuador se enciende,
    se es igual a 0 el actuador se apaga y se publica el estado en el topic "doorstatus"*/
void Actuador(int action){
  Serial.println("On/Off actuador");
  if (action == 0){
    analogWrite(act, 76);  // Con 76 se cierra la puerta
    servstat=false;
    //client.publish(mqttstatus,"Off");
    Serial.println("Cerrando");

  }else if(action == 1){
    analogWrite(act,180);  // Con 180 se abre la puerta
    servstat=true;
    //client.publish(mqttstatus,"On");
    Serial.println("Abriendo");

  }
  // Se leen las condiciones del servo y el sensor. Si el sensor está en abierto y el servo está cerrado
  // se enciende la alarma, debido a que la puerta no se encuentra cerrada correctamente.
  // La alarma se desactiva cuando tanto el sensor como el servo indican que la puerta está cerrada.
  if (sensorstat && !servstat){
    digitalWrite(buz,HIGH);

  } else if (!sensorstat && !servstat){
     
    digitalWrite(buz,LOW);

  }
}

/* Función donde se procesa el mensaje que llega a un topic MQTT. El mensaje queda almacenado en forma de byte*
   en la variable 'payload' de la función.*/
void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  int action;

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);    
    message += (char)payload[i];    // Se convierte el mensaje en String
  }

  action = obtJson(message);
  
  Actuador(action);

  Serial.println();
  Serial.println("-----------------------");
}

void ICACHE_RAM_ATTR sensormessage(){
  int stat = digitalRead(sens);
  if (stat == HIGH)
  {
    Serial.println("Cerrado.");
    client.publish(mqttstatus,"{\"pattern\": \"doorstatus\", \"data\": \"Cerrado\"}");
    sensorstat=false;
  }
  else if (stat==LOW)
  {
    Serial.println("Abierto.");
    client.publish(mqttstatus,"{\"pattern\": \"doorstatus\", \"data\": \"Abierto\"}");
    sensorstat=true;
  }  
}


void setup(){

  pinMode(act,OUTPUT);    // Pin del actuador como salida
  analogWrite(act, 180);
  servstat=true;

  pinMode(buz,OUTPUT);    // Pin de la alarma como salida

  pinMode(sens, INPUT_PULLUP);  // Activar interrupción Pin sensor

  Serial.begin(115200);   // Iniciar comunicación serial

  setup_Wifi();           // Iniciar conexión a red WiFi

  client.setServer(mqttServer, mqttPort); // Iniciar conexión al Broker MQTT
 
  // Validación MQTT
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {  // "ESP8266Cliente": ID con el que se reconoce en el servidor MQTT
      Serial.println("connected");
    } 
    else { 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
  }

  client.subscribe(mqttdoor);    // Subcripción al topic donde se publica la acción para el actuador.  

  client.setCallback(callback); // Iniciar escucha de los topics suscritos.
  
  attachInterrupt(sens,sensormessage,CHANGE);
}

void loop() {
  //MQTT client loop
  client.loop();
}