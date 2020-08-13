#include <HX711.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

#define calibration_factor1 -409200 //This value is obtained using the Calibration sketch
#define calibration_factor2 -442400 //This value is obtained using the Calibration sketch

#define DOUT1  D1
#define CLK1  D2

#define DOUT2 D5
#define CLK2 D6

float weight1, weight2, total_weight;

HX711 scale1(DOUT1,CLK1);
HX711 scale2(DOUT2,CLK2);

#define BUILTIN_LED D4

const char* ssid = "****";
const char* password = "****";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    delay(50);
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("OutTopic_Try/weigh/", "Weighing Scale Demo");
      // ... and resubscribe
      client.subscribe("OutTopic_Try/weigh/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  delay(50);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  scale1.set_scale(calibration_factor1); //This value is obtained by using the Calibration sketch
  scale1.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

  scale2.set_scale(calibration_factor2);
  scale2.tare();

}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  
    if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    weight1=scale1.get_units(5);
    weight2=scale2.get_units(5);
    total_weight=(weight1+weight2)*1000;
    snprintf (msg, 75, "Total weight #%4.2f", total_weight);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("OutTopic_Try/weigh/", msg);
  }
}
