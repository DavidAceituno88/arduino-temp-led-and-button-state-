#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>
#include <OneWire.h>


// Update these with values suitable for your network.
const char* ssid = "INFINITUM44A4_2.4"; //your WiFi Name
const char* password = "ttEtyURT3y"; //Your Wifi Password
const char* mqtt_server = "tailor.cloudmqtt.com";
const int   mqtt_port = 15179;
const char* mqtt_user ="cdbxmtzn";
const char* mqtt_pass ="1o_Xdr2qM41E";


WiFiClient espClient;
PubSubClient client(espClient);
#define ONE_WIRE_BUS 14                        //D5 = GPIO14  pin  of nodemcu example D4 = GPIO2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);            // Pass the oneWire reference to Dallas Temperature.
int button = 16;                                // el D0 es el GPIO16
int LectBoton = 0;                               // Button initial state
int LectLed = 0;                                // Led initial state
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void callback(char* topic, byte* payload, unsigned int length) {  
  String CadEntra;
  String CadShort;
  CadEntra = "";
  CadShort = "";
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
               CadEntra = CadEntra + (char)payload[i];                      // It stores the initial values from the input string 
                if (i < 10) {  CadShort = CadEntra;  }
  }
  
  Serial.println();
      // Turn Led On if it reads 'M01/S01/L1'  from the website
      if (CadShort == "M01/S02/L1") {    
        digitalWrite(BUILTIN_LED, LOW);    //Turn the LED on (Note that LOW is the voltage level
        Serial.println("\n Led On"); // but actually the LED is on; this is because
                                          // it is active low on the ESP-01)
     } else if (CadShort == "M01/S02/L0") {
         digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
         Serial.println("\n Led Off");
     }

} // Callback


void Setup_wifi(){
  // connecting to the WiFi
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("Connecting to ->:");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
       while (WiFi.status() != WL_CONNECTED){
       Serial.print(".");
       delay(250);
       }
 Serial.println("");
 Serial.println("Succesfully Connected!");
 Serial.print("IP is -> :");
 Serial.println(WiFi.localIP());
} // Setup_wifi


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection ...");
    // Create a random client ID ** it creates a dynamic ID to avoid duplicates 
    // cause sometimes we lose connection 
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
   
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.print(" MQTT Connection Sccessful !!!");
      Serial.println ("Client ID :" + clientId);
      // Once connected, publish an announcement...
      client.publish("output", "Mensaje 1");  
      // ... Here we suscribe to the topic
      client.subscribe("input");  
    } else {
      Serial.print("connection Failed, Error =");
      Serial.print(client.state());
      Serial.println("Trying angain in 5 seconds ..");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
   
  pinMode(BUILTIN_LED, OUTPUT);     //Initialize the BUILTIN_LED pin as an output
  pinMode(button, INPUT);           // declare push button as input  es el D0 o el GPIO16
  Serial.begin(115200);             // output port
  Setup_wifi();                     // initialize WIFI
  client.setServer(mqtt_server, mqtt_port);   // Selects MQTT server
  client.setCallback(callback);               
  Serial.println("Enters void setup !");
  Serial.println("DHT AM2301 Remember that goes with a positive pin resistance data in ESP8266!");
  Serial.println("DHT for Ehernet Shield goes straight to digital pin");
  Serial.println("Exits Void setup");
  digitalWrite(BUILTIN_LED, HIGH);  //    turns the LED off by making the voltage HIGH
}


void loop() { 
  
  // reeds the LED state so it can later add it to the message sent to the broker
  LectBoton = digitalRead(2);
  
  LectBoton = digitalRead(button);                   //Reeds the state of D0
  String mes = "M01/S02/";     // Here we initialize the message sent to te broker by telling him THIS IS SENSOR 02 FROM MARKET 01
         mes = mes + "B"+ LectBoton +"/T";    // *B* is for Button wich will be the char from where we can trim the string later on and know that is the state of the button
  
  if (client.connected()==false){
      reconnect();
    
  }

  client.loop();          // This keeps alive the communication with the MQTT server
                          // this loop keeps checking the server connection
                           

  if (millis()- lastMsg > 2500){
 
                                          
     sensors.requestTemperatures();                    // Send the command to get temperatures                              
                                                       //Serial.println(sensors.getTempCByIndex(0));    
     mes = mes +  String(sensors.getTempCByIndex(0));  //The temp value is always at the end of the string for easy manipulation later on  
      
                        
     lastMsg = millis();                                 
           value++;
                                                 
     mes.toCharArray(msg,50);                    // msg is a buffer where we pass the infromation as a string witha lenght less than 50
     client.publish("output",msg);               // Publishes the buffer's value on the topic 'output' Serial.println("Mensaje enviado puro --> :" + String(sensors.getTempCByIndex(0))+ " °C");  // envia solo temperatura
     Serial.println("Mensaje enviado A CloudMQTT --> :" + mes + "/L" + String(LectLed));           // sends the value of mes 
     
  }

}
