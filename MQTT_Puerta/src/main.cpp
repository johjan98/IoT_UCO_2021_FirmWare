#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <sstream>

const char* ssid = "****";
const char* password =  "*****";
const char* mqttServer = "****";
const int mqttPort = 18820;
const char* mqttUser = "*****";
const char* mqttPassword = "*******";
const char* mqttdoor = "triggeropendoor";
const char* mqttstatus = "doorstatus";    //Topic donde se publica el estado del actuador

const int act=5;

WiFiClient espClient;
PubSubClient client(espClient);

/* Configuración para conectar a la red WiFi
 Imprime la dirección IP cuando se logra la conexión. */
void setup_Wifi (){
  delay(10);

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


/*  Función para accionar un acturador. Si "action" es igual a 1 el actuador se enciende,
    se es igual a 0 el actuador se apaga y se publica el estado en el topic "doorstatus"*/
void Actuador(int action){
  Serial.println("On/Off actuator");
  if (action == 1)
  {
    digitalWrite(act,HIGH);
    client.publish(mqttstatus,"On");
  }
  else if (action == 0){
    digitalWrite(act,LOW);
    client.publish(mqttstatus,"Off");
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

  action = (int)message[0] - 48;
  
  Actuador(action);

  Serial.println();
  Serial.println("-----------------------");
  Serial.println(action);
}


void setup(){

  pinMode(act,OUTPUT);    // Pin del actuador como salida

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

}

void loop() {
  //MQTT client loop
  client.loop();
}