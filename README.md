# Alexa Face Recognition with ESP32-CAM
An ESP32-CAM based face recognition solution to trigger Alexa routines.

Purpose is to start routines in Alexa service based on recognised faces from ESP32-CAM.

It is based on this repository: https://github.com/robotzero1/esp32cam-access-control

Is using https://www.virtualsmarthome.xyz/ "URL Routine Trigger" solution to trigger Alexa routines.

You have to register for this service with the Amazon account and also enable the Virtualsmarthome skill in Alexa.
For each person to be recognised create a "Trigger name" and URL.

The different URLs are then requested from ESP32 via https after a defined face has been recognised.
A Virtual "Door Bell" can be used in Alexa to trigger routines for each face/URL.

You have to set the WLAN access details in the code:
```
const char* ssid = "ssid";
const char* password = "password";
```

And you have to set the different URLs in function ReqURL():
```
 const char* URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                               "https://www.virtualsmarthome.xyz/url_routine_trigger/..."}; 
```
For security reasons the Root CA is sored in the code. The certifficate will expire in September 2021. It has to be updated then.

Sketch works with current Arduiono IDE 1.9.13 and ESP32 board version 1.0.5.

In IDE you have to select:
- Board: ESP32 Wrover Module
- Partition scheme: Huge APP...

![Web](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Alexa%20Face%20Recognition.pgn)

You have to add the persons with names with the web frontend.

The same names have to be used the in the code to request the URL for each person.

```
if (strcmp(name, "Person1") == 0) {
      ReqURL(0);
    } else if (strcmp (name, "Person2") == 0) {
      ReqURL(1);
    }
```

After enabling the skill in Alexa you cen the create new routines. As trigger you can the select a "door bell" with the name you have given at Virtualsmarthome.


