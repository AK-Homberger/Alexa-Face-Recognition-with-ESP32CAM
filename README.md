# Alexa Face Recognition with ESP32-CAM
An ESP32-CAM based face recognition solution to trigger Alexa routines.

![ESP32-CAM](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/ESP32-CAM.png)

The purpose of this repository is to start routines in Alexa service based on recognised faces from ESP32-CAM.

![Web](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Alexa%20Face%20Recognition.png)

It is based on this repository: https://github.com/robotzero1/esp32cam-access-control

I did several changes to the code:
- Additional comments in code.
- Use readable HTML/Javascript code in camera_index.h (makes it easier to change content).
- Changed Javascript code to make it work also with Safari web client (deleted audio interface).
- Allow face detection with and without client connected via web socket.
- Added root certificate and code to request URLs for each recognised face.
- Use built-in LED to show if face is detected and also to provide additional light for better detection.

Background information regarding ESP-Face component from Espressiv can be found [here](https://github.com/espressif/esp-face/).

## Preparations
The solution is using https://www.virtualsmarthome.xyz/ "URL Routine Trigger" service to trigger Alexa routines.

You have to register for this free service with the Amazon account and also enable the Virtualsmarthome skill in Alexa.
For each person to be recognised, create a "Trigger name" and URL.

The different URLs are then requested from ESP32 via https after a defined face has been recognised.
A virtual SmartHome "Door Bell" can be used in Alexa to trigger routines for each face/URL.

## Changes in the Code
You have to set the WLAN access details in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/679f133bf7162803b51e32012d9674ca8e168aaa/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L34):
```
const char *ssid = "ssid";
const char *password = "password";
```

And you have to set the different URLs in function [ReqURL()](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/679f133bf7162803b51e32012d9674ca8e168aaa/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L250):
```
 const char *URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                              "https://www.virtualsmarthome.xyz/url_routine_trigger/..."}; 
```
Just copy your individual URLs from the Virtualsmarthome web site. The JSON version is the preferred option (short response).

For security reasons the [Root CA certificate](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/679f133bf7162803b51e32012d9674ca8e168aaa/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L38) is stored in the code. The certificate will expire in September 2021. It has to be updated then.

To perform the update (with Firefox browser) just go to the https://www.virtualsmarthome.xyz web site an click on lock symbol left to the URL. Then show details of connection, further information and show certificate. Then click on [DST Root CA X3](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Root-Certificate.png) and then on "PEM (Certificate)". The certificate text have to be copied into the sketch to update.

## Arduino IDE and Programming
The sketch works with current Arduino IDE 1.8.13 and ESP32 board version 1.0.5.

In the IDE you have to select:
- Board: ESP32 Wrover Module
- Partition scheme: Huge APP...

An additional library "ArduinoWebsockets" has to be installed via the IDE Library Manager (version 0.5.0 is working for me).

If you copy/move the sketch, make sure all four files are in the new folder:
- AlexaFaceDetectionESP32Cam.ino 
- camera_index.h
- camera_pins.h
- partitions.csv

You need an external programmer to install the sketch on the ESP32-CAM module. Here is a [tutorial](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/) that shows the process.

## Web Frontend
After programming you have to start the web frontend with the IP shown in the Serial Monitor of the IDE.
If the sketch is working, you have to add the persons with names with the web frontend "ADD USER".

The face information is stored persistently in flash memory.

The same names have to be used the in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/679f133bf7162803b51e32012d9674ca8e168aaa/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L430) to request the URL for each person (currently "Person1" and "Person2").
If you want to change the names just change the code accordingly. You can also add more names. But then also add URLs in list in [ReqURL()](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/679f133bf7162803b51e32012d9674ca8e168aaa/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L250)

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

## Housing

There are many nice 3D print housings for the ESP32-CAM available at Thingiverse. Example: https://www.thingiverse.com/thing:3652452  

# Updates
- Version 0.1, 23.03.2021: Initial version.
