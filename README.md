# Alexa Face Recognition with ESP32-CAM
An ESP32-CAM based face recognition solution to trigger Alexa routines.

![ESP32-CAM](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/ESP32-CAM.png)

The purpose of this repository is to start routines in Alexa service based on recognised faces from ESP32-CAM.

![Web](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Alexa%20Face%20Recognition.pgn)

It is based on this repository: https://github.com/robotzero1/esp32cam-access-control

I did several changes to the code:
- Additional comments in code.
- Use readable HTML/Javascript code in camera_index.h (makes it easier to change content).
- Changed Javascript code to make it work also with Safari web client (deleted audio interface).
- Allow face detection with and without client connected via web socket.
- Added root certifficate and code to request URLs for each recognised face.
- Use builtin LED to show if face is detected and also to provide addtional light for better detection.

## Preparations
The solution is using https://www.virtualsmarthome.xyz/ "URL Routine Trigger" service to trigger Alexa routines.

You have to register for this free service with the Amazon account and also enable the Virtualsmarthome skill in Alexa.
For each person to be recognised, create a "Trigger name" and URL.

The different URLs are then requested from ESP32 via https after a defined face has been recognised.
A virtual SmartHome "Door Bell" can be used in Alexa to trigger routines for each face/URL.

## Changes in the Code
You have to set the WLAN access details in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L35):
```
const char *ssid = "ssid";
const char *password = "password";
```

And you have to set the different URLs in function [ReqURL()](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L250):
```
 const char *URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                              "https://www.virtualsmarthome.xyz/url_routine_trigger/..."}; 
```
Just copy your individual URLs from the Virtualsmarthome web site. The JSON version is the preferred option (short response).

For security reasons the [Root CA certifficate](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/dbfc8bc4eaf89e81cfe0bc2ecbc2932a62472344/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L37) is sored in the code. The certifficate will expire in September 2021. It has to be updated then.

## Arduino IDE and Programming
The sketch works with current Arduino IDE 1.8.13 and ESP32 board version 1.0.5.

In the IDE you have to select:
- Board: ESP32 Wrover Module
- Partition scheme: Huge APP...

An additional library "ArduinoWebsockets" has to be installed via the IDE Library Manager (version 0.5.0 is working for me).

You need an external programmer to install the sketch on the ESP32-CAM module. Here is a [tutorial](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/) that shows the process.

## Web Frontend
After programming you have to start the web frontend with the IP shown in the SerialMonitor of the IDE.

If the sketch is working, you have to add the persons with names with the web frontend "ADD USER
".

The face information is stored persistently in flash memory.

The same names have to be used the in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L428) to request the URL for each person (currently "Person1" and Person2").

If you want to change the names just change the code accordingly. You can allso add more names. But then also add URLs in list in [ReqURL()](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/820072e45e19db61a0750780037e1fea23065cbc/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L250)

```
if (strcmp(name, "Person1") == 0) {
      ReqURL(0);
    } else if (strcmp (name, "Person2") == 0) {
      ReqURL(1);
    }
```

## Cofigure Alexa
After enabling the skill in Alexa you can then create new routines. As trigger you can then select a "Door Bell" with the name you have given at Virtualsmarthome.

Enjoy now personal responses of Alexa after your face has been recognised.

# Updates
- Version 0.1, 23.03.2021: Initial version.
