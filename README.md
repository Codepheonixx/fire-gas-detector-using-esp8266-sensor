# Smart Fire and Gas Alarm using ESP8266
Gas leakage is a critical safety concern in residential, commercial, and industrial settings and poses a significant risk to the environment. It leads to potential explosions, fire hazards, health issues, and environmental damage, for example: The Bhopal Gas Tragedy (nearly 3787 deaths occurred )

## Solution
Our solution is very simple, We need to build a cost-effective alarm system to detect various hazardous gases (when in case of any leakage). Our device will contain an ESP8266 that will link different components like a Buzzer, Light, Fire detector sensor, and Gas sensor (all the components will get linked with each other). Now we will build 5 modules/modems of the same type, this will allow us to build the model on a larger scale which will create a network among the 5 modems. Our module will contain ESP-now that will help to connect all the devices via MAC Address.

## Methodology
For instance lets say an industry faces gas leakage or accidental fire, child devices present at specific locations after detecting gas or fire will trigger alarms consisting of buzzer and lights. After triggering alarm it will use ESP-now to notify the central hub without any delay. The central hub consisting of an esp8266 will notify about the hazard to workers smartphone with high alert via MQTT broker.
