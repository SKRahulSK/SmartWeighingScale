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

const char* ssid = "TP-LINK_FFAA";
const char* password = "9535947698";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//For the channel input string and present state
String channel_in;
String present_st;
boolean flag;

void setup_wifi() 
{
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

void set_scale() // This function is used to initialise/reset the weighing scale 
{
  scale1.set_scale(calibration_factor1); //This value is obtained by using the Calibration sketch
  scale1.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

  scale2.set_scale(calibration_factor2);
  scale2.tare();
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  channel_in = "";
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    channel_in+=String((char(payload[i])));
  }
  Serial.println(channel_in);

  // Switch on the LED if an 1 was received as first character
  if(channel_in.equals(present_st))
  {
    Serial.println(present_st);
    exit;
  }
  else if (channel_in.equals("start")) 
  {
    // Start sending message to the MQTT channel;
    flag = true;
    Serial.println(present_st);
    present_st = "start";
    Serial.println(present_st);
    while(flag)
    {
      weight1=scale1.get_units(7);
      weight2=scale2.get_units(7);
      total_weight=(weight1+weight2)*1000;
    
      if(total_weight < 0) { total_weight = 0; }
    
      snprintf (msg, 75, "Weight = %4.2f gms", total_weight);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("OutTopic_Try/weigh/", msg);
      delay(2000);
      client.loop();
    }
  } 
  else if(channel_in.equals("reset"))
  {
    Serial.println(present_st);
    set_scale();
  }
  else if(channel_in.equals("stop"))
  {
    // do not send any message to the MQTT channel;
    present_st = "";
    Serial.println(present_st);
    flag = false;
  }
}

void reconnect() 
{
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
      client.subscribe("InTopic_Try/weigh/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() 
{
  Serial.begin(9600);
  delay(50);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  set_scale(); // To initialise the weighing scale
  
  //present_st and flag are used in callback function to get required output from start, reset and stop states. 
  present_st = "";
  flag = true;
}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
