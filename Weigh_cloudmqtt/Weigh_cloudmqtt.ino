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

float weight1, weight2, total_weight,prev_weight;

HX711 scale1(DOUT1,CLK1);
HX711 scale2(DOUT2,CLK2);

long lastMsg = 0;
char msg[50];
int value = 0;

//For the channel input string and present state
String channel_in, present_st;
boolean flag;

const char* ssid = "mesch_network";
const char* password =  "meschnetwork...";
const char* mqttServer = "m21.cloudmqtt.com";  //mqtt://m21.cloudmqtt.com:13462
const int mqttPort = 13462;
const char* mqttUser = "ilamiyvz";
const char* mqttPassword = "hC1RJUurBdMP";
 
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() 
{
  delay(5);
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
void callback(char* topic, byte* payload, unsigned int length) {
 
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
  }
  else if (channel_in.equals("start")) 
  {
    // Start sending message to the MQTT channel;
    flag = true;
    //Serial.println(present_st);
    present_st = "start";
    //Serial.println(present_st);
    snprintf (msg, 75, "%4.2f", prev_weight);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("OutTopic_Try/weigh/", msg);
    while(flag)
    {
      delay(5); // For disabling WDT reset.
      weight1=scale1.get_units(5);
      weight2=scale2.get_units(5);
      total_weight=(weight1+weight2)*1000;
    
      //if(total_weight < 0) { total_weight = 0; }
      
     if((fabs(total_weight-prev_weight)>1) && (((fabs((total_weight-prev_weight)/prev_weight))) > 0.02))
      {
        snprintf (msg, 75, "%4.2f", total_weight);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("OutTopic_Try/weigh/", msg);
        prev_weight = total_weight;
      }
      client.loop();
    }
  }
  else if(channel_in.equals("tare"))
  {
    delay(5);
    Serial.println(present_st);
    set_scale(); 
  } 
  else if(channel_in.equals("stop"))
  {
    // do not send any message to the MQTT channel;
    present_st = "";
    Serial.println(present_st);
    flag = false;
    snprintf (msg, 75, "%4.2f", 0.0);
    client.publish("OutTopic_Try/weigh/", msg);
  }
 
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) 
    {
      Serial.println("connected");
      client.subscribe("InTopic_Try/weigh/");  
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}


void setup() 
{
  Serial.begin(9600);
  delay(50);
  
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  set_scale(); // To initialise the weighing scale
  
  //present_st and flag are used in callback function to get required output from start, reset and stop states. 
  present_st = "";
  flag = true;
  prev_weight = 0; // prev_weight is used to store the last weight. Which is used to compare with current weight to decide on whether to send the data to MQTT or not
 
}

void loop() 
{
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
