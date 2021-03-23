# Alexa Face Recognition with ESP32-CAM
An ESP32-CAM based face recognition solution to trigger Alexa routines.

Purpose is to start routines in Alexa service based on recognised faces from ESP32-CAM.

![Web](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Alexa%20Face%20Recognition.pgn)

It is based on this repository: https://github.com/robotzero1/esp32cam-access-control

I did several changes to the code:
- Use readable HTML/JS code in camera_index.h (makes it easier to change content).
- Changed javascript code to make it work also with Safari web client (deleted audio interface)
- Allow face detection with and without client connected via web socket.
- Added root certifficate and code to request URLs for each recognised face.
- Use builtin LED to show if face is detected and also to provide addtional light for better detection.

Is using https://www.virtualsmarthome.xyz/ "URL Routine Trigger" solution to trigger Alexa routines.

You have to register for this service with the Amazon account and also enable the Virtualsmarthome skill in Alexa.
For each person to be recognised, create a "Trigger name" and URL.

The different URLs are then requested from ESP32 via https after a defined face has been recognised.
A virtual "Door Bell" can be used in Alexa to trigger routines for each face/URL.

You have to set the WLAN access details in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L35):
```
const char* ssid = "ssid";
const char* password = "password";
```

And you have to set the different URLs in function [ReqURL()](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L250):
```
 const char* URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                               "https://www.virtualsmarthome.xyz/url_routine_trigger/..."}; 
```
Just copy your individual URLs from the Virtualsmarthome web site. The JSON version is the preferred option (short response).

For security reasons the Root CA is sored in the code. The certifficate will expire in September 2021. It has to be updated then.

The sketch works with current Arduiono IDE 1.9.13 and ESP32 board version 1.0.5.

In the IDE you have to select:
- Board: ESP32 Wrover Module
- Partition scheme: Huge APP...

An additional library "ArduinoWebsockets" has to be installed via the IDE Library Manager (version 0.5.0 is working for me).

You need an external programmer to install the sketch on the ESP32-CAM module. Here is a [tutorial](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/) that shows the process.

After programming you have to start the web frontend with the IP shown in the SerialMonitor of the iDE.

If the sketch is working, you have to add the persons with names with the web frontend "ADD USER
".

The face information is stored persistently in flash memory.

The same names have to be used the in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L428) to request the URL for each person (currently "Person1" and Person2").

```
if (strcmp(name, "Person1") == 0) {
      ReqURL(0);
    } else if (strcmp (name, "Person2") == 0) {
      ReqURL(1);
    }
```

After enabling the skill in Alexa you can then create new routines. As trigger you can the select a "Door Bell" with the name you have given at Virtualsmarthome.

Enjoy now personal responses of Alexa after your face has been recognised.

# Updates
- Version 0.1, 23.03.2021: Initial version.
