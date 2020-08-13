# Weighing-Scale-IoT

  -The project uses ESP8266 and HX711 ADC to weigh the object and send the result to the respective MQTT channel.                           
  -This is part of SmartKiitchen which uses the weighing scale to measure the weight of the ingredients used for cooking and send it to the MQTT channel in real time. The result is then subscribed from the channel and is displayed.

# Libraries used
   1. HX711 library to interface the Semiconductor HX711 Analog-to-Digital Converter (ADC) for Weighing Scale(https://github.com/bogde/HX711.git)
   2. ESP8266WiFi for wifi connection setup of ESP8266 wifi module. 
   3. Pubsubclient library to connect a client for publish/subscribe messaging with a server that supports MQTT.(https://github.com/knolleary/pubsubclient.git)

#  How to Calibrate the Scale
   1.Call set_scale() with no parameter.                                                                                                   
   2.Call tare() with no parameter.                                                                                                       
   3.Place a known weight on the scale and call get_units(10).                                                                             
   4.Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale.                   
   5.Adjust the parameter in step 4 until you get an accurate reading.                                                                     
 
# MQTT Channel setup
   1. client.setServer(mqtt_server, port number); // to set the MQTT server and port number.
   2. client.publish("OutTopic_Try/weigh/", msg);  // OutTopic_Try/weigh/ --> is the channel to which the scale sends the weight.
   3. client.setCallback(callback);  // callback is a function defined to receive any message from the subscribed channel.

