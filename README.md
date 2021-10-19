# Alexa Face Recognition with ESP32-CAM
An ESP32-CAM based face recognition solution to trigger Alexa routines.

![ESP32-CAM](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/ESP32-CAM-Alexa.png)

The purpose of this repository is to start routines in Alexa based on recognised faces from ESP32-CAM.

![Web](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Alexa%20Face%20Recognition.png)

It is based on this repository: https://github.com/robotzero1/esp32cam-access-control

I did several changes to the code:
- Additional comments and simplifications in code.
- Use of readable HTML/Javascript code in camera_index.h (makes it easier to change content).
- Changed Javascript code to make it work also with Safari web client (deleted audio interface).
- Allow face detection with and without client connected via web socket.
- Added root certificate and code to request URLs for each recognised face.
- Use built-in LED to show if face is detected and also to provide additional light for better detection.
- Closed memory leak by freeing up used buffers.

## Preparations
The solution is using https://www.virtualsmarthome.xyz/ "URL Routine Trigger" service to trigger Alexa routines.

You have to register for this free service with your Amazon account and also enable the Virtualsmarthome skill in Alexa. For each person to be recognised, create a "Trigger name" and URL.

The different URLs are then requested from the ESP32 via https after a defined face has been recognised.
A virtual SmartHome "Door Bell" with the name of the defined "Trigger name"can be used in Alexa to trigger routines for each face/URL.

## Changes in the Code
You have to set the WLAN access details in the [code](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/de6f3e61684926d26a0e4989af5788e650d2d3ac/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L40):
```
const char *ssid = "ssid";
const char *password = "password";
```

And you have to set the different URLs in the array [URL](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/de6f3e61684926d26a0e4989af5788e650d2d3ac/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L44):
```
// Trigger URLs: 7 URLs for maximum 7 enrolled faces (see FACE_ID_SAVE_NUMBER)
const char *URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                             "https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                             "",
                             "",
                             "",
                             "",
                             ""                             
                            };

```
Just copy your individual URLs from the Virtualsmarthome web site. The JSON version is the preferred option (short response).

The order of the URLs is matching the order of stored (enrolled) faces. 

**Tip:** You can store more then one face ID per person. That is further improving recognition precision.

## Root CA Certificate
For security reasons the [Root CA certificate](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/de6f3e61684926d26a0e4989af5788e650d2d3ac/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L54) is stored in the code. 
The certificate is used to authenticate the identity of the web server. **The certificate will expire in September 2021**. It has to be updated then.

To perform the update (with Firefox browser) just go to the https://www.virtualsmarthome.xyz web site and click on the lock symbol left to the URL. Then show details of connection, further information and show certificate. Then click on [DST Root CA X3](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Root-Certificate.png) and then on "PEM (Certificate)". The certificate text have to be copied into the sketch to update.

For the current version 1.1 code, I did the update to the new "ISRG Root X1" root certificate already.

## Arduino IDE and Programming
The sketch works with current Arduino IDE 1.8.13 and ESP32 board version 1.0.5.

In the IDE you have to select:
- Board: ESP32 Wrover Module
- Partition scheme: Huge APP...

An additional library **ArduinoWebsockets** has to be installed via the IDE Library Manager (version 0.5.0 is working for me).

If you copy/move the sketch, make sure all four files are in the new folder:
- AlexaFaceDetectionESP32Cam.ino 
- camera_index.h
- camera_pins.h
- partitions.csv

Especially **partitions.csv**. This file is not copied from Arduino IDE when using "Safe as..." function.

[**_camera_index.h_**](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/AlexaFaceDetectionESP32Cam/camera_index.h) contains the HTML/Javascript web page and [**_camera_pins.h_**](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/AlexaFaceDetectionESP32Cam/camera_pins.h) the GPIO definitions for the used camera model. In our case the model is **CAMERA_MODEL_AI_THINKER** for the ESP32-CAM.

**_partitions.csv_** defines the partition information for flash memory:

| Name    | Type    | SubType | Offset   | Size     | Flags |
|---------|---------|---------|----------|----------|-------|
| nvs     | data    | nvs     | 0x9000   | 0x5000   |       |
| otadata | data    | ota     | 0xe000   | 0x2000   |       |
| app0    | app     | ota_0   | 0x10000  | 0x300000 |       |
| fr      | 32      | 32      | 0x310000 | 0xF0000  |       |

The **fr** partition is relevant for storage of the Face IDs. With the current Arduino IDE it is sufficient to store the partition information simply in the sketch folder.

You need an external (FTDI) programmer to install the sketch on the ESP32-CAM module. 

![FTDI](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/ESP32-CAM-FTDI-v2.png)

For programming you have to connect IO0 to GND, RX/TX (crossed) and 5V/GND. Then press the "boot" button on the ESP32-CAM. After that, you can start the upload process within the Arduino IDE (with FTDI USB-Serial port selected).

**Caution:** The 5V connection shown in an earlier version of this project was wrong (CTS instead of VCC connection).

For normal start open IO0 from GND and press "boot" again.

## Configuration Web Page
After programming you have to start the configuration web page with the **IP** shown in the **Serial Monitor** of the IDE.
If the sketch is working, you have to add the persons with names with the web frontend function **ADD USER**.

![Web](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/main/Alexa%20Face%20Recognition.png)

The face information is stored persistently in flash memory.

Up to 7 different faces can be stored. You can store more than one face for the same person to improve the recognition rate even more.

The **order of added users is relevant** for the URL to be sent. The name is not relevant.

The web page is only necessary for managing the faces (add/delete). The recognition process is also active while no web client is connected. The built-in LED is activated for three seconds as soon as a (general) face is detected. A short blink with the LED shows a successfull face recognition process.

## Configuring Alexa
After enabling the Virtualsmarthome skill in Alexa you can then create new routines. As trigger you can then select a virtual "Door Bell" with the name you have given at Virtualsmarthome.

Enjoy now personal responses of Alexa after your face has been recognised.
You can start for example playing your favorite music after face recognition.

## Background Information for Face Detection/Recognition
Background information regarding the ESP-Face component from Espressif can be found [here](https://github.com/espressif/esp-face/).

In general, the face recognition process works like this:
![Flow](https://github.com/espressif/esp-face/blob/master/img/face-recognition-system.png)

Let's see how the flow chart is implemented in the sketch.

For face detection/recognition we need three **include files**:
```
#include <fd_forward.h>    // Header file for face detection
#include <fr_forward.h>    // Header file for face recognition
#include <fr_flash.h>      // Header file for flash storage handling
```

To implement the detection/recognition process, we have to define some **global variables/objects**:
```
// Face detection/recognition variables
camera_fb_t *fb;                              // Frame buffer pointer for picture from camera
box_array_t *detected_face;                   // Information for a detected face
dl_matrix3du_t *image_matrix;                 // Image matrix pointer
dl_matrix3du_t *aligned_face;                 // Aligned face pointer
dl_matrix3d_t *face_id;                       // Face ID pointer
face_id_node *face_recognized;                // Recognized face pointer
mtmn_config_t mtmn_config;                    // MTMN detection settings
```
- The frame buffer pointer **_*fb_** will later contain the pointer to the picture from the ESP camera. The format of the picture is in JPEG (compressed) format.
- The pointer **_*detected_face_** will point to a struct containig information for a detected face.
- **_*image_matrix_** pointer will point to a struct containing the bitmap of the picture. We need the bitmap to detect and recognise faces.
- The **_*aligned_face_** pointer will point to a struct containg "aligned" information for the face detected. The aligned information is necessary for face recognition process.
- The **_*face_id_** contains the the result of the face recognition process. We will later compare the face_id with the stored face_id's to detect a specific person.
- The struct **_mtmn_config_** will contain the configuration parameters for the face detection process.

The function [**_camera_init()_**](https://github.com/AK-Homberger/Alexa-Face-Recognition-with-ESP32CAM/blob/e768f120aeb69f736a2f02948c25cf25811fcc24/AlexaFaceDetectionESP32Cam/AlexaFaceDetectionESP32Cam.ino#L172) in the code is used to configure the camera. Important is the correct "frame size" It has to be 1/4 VGA (320*240 pixel). It is configured with the line **_config.frame_size = FRAMESIZE_QVGA_**.

As next step we have to do some preparation work in **setup()**:

```
  mtmn_config = mtmn_init_config();     // Set MTMN face detection details (default values)
  read_faces();                         // Read faces from flash
  
  image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);                 // Allocate memory for image matrix
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);  // Allocate memory for aligned face
```
- With **_mtmn_config = mtmn_init_config()_** we set the parameters for face detection to the default values defined in [**_fd_forward.h_**](https://github.com/espressif/esp-face/blob/420fc7e219ba98e40a5493b9d4be270db2f2d724/face_detection/include/fd_forward.h#L71).
- Then we read the faces (names and face_id's) from flash memory. This is necessary to compare the face_id's later.
- As last preparation step we have to allocate memory for the struct containing the **_image_matrix_**, which is the bitmap for face detection. The size of the bitmap is 1/4 VGA (320*240 pixel).
- And also for the struct **_alligned_face_**. An aligned face has the format 56*56 pixel (as defined in [**_fr_forward.h_**](https://github.com/espressif/esp-face/blob/420fc7e219ba98e40a5493b9d4be270db2f2d724/face_recognition/include/fr_forward.h#L12)).

That's all preparation needed.

In **loop()** we will do the detection and the the recognition work. 

### Face Detection
Let's start with face detection:

```
    fb = esp_camera_fb_get();       // Get frame buffer pointer from camera (JPEG picture)
    
    fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item); // JPEG to bitmap conversion
    
    detected_face = face_detect(image_matrix, &mtmn_config);      // Detect face
```

These three lines is all to detect a potential face:
1. With **_fb = esp_camera_fb_get()_** we will get the picture from the camera. The format is 320*240 pixel, JPEG compessed. For the face detection process we need the picture as plain bitmap. 
2. The conversion is done with **_fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)_**. The **_image_matrix_** contains then the bitmap picture. 
3. And now we try to detect a face with **_detected_face = face_detect(image_matrix, &mtmn_config)_**. The routine gets the bitmap and the configuration parameters as input. 

The **_detected_face_** struct contains the result of the detection process. If a face has been detected the value of **_detected_face_** is **true**.

### Face Recognition
Now we can do the face recognition part. That are only four more steps:

```
    if (detected_face) {  // A general face has been recognised (no name so far)

      // Switch LED on to give more light for recognition
      digitalWrite(LED_BUILTIN, HIGH); // LED on
      led_on_millis = millis();        // Set on time

      if (align_face(detected_face, image_matrix, aligned_face) == ESP_OK) {  // Align face
        
        face_id = get_face_id(aligned_face);  // Get face id for detected and aligned face
        
        if (st_face_list.count > 0) {  // Only try if we have faces registered at all
          
          face_recognized = recognize_face_with_name(&st_face_list, face_id);  // Try to recognise face by comparing face id's
          
          if (face_recognized) { // Face has been sucessfully identified
            face_detected(face_recognized->id_name);  // Request URL for recognised name            
          }
        }
        dl_matrix3d_free(face_id);        // Free allocated memory
      } 
```
1. Face alignment: **_align_face(detected_face, image_matrix, aligned_face)_**. The routine gets the detected_face and the bitmap information as input. The result will be stored in the aligned_face struct.
2. Now we get the Face ID with: **_face_id = get_face_id(aligned_face)_**. The Face ID contains then the characteristic information for an aligned face.
3. We compare then the Face ID with stored IDs: **_face_recognized = recognize_face_with_name(&st_face_list, face_id)_**. If a face has been recognised, the value of **_face_recognized_** is **true**.
4. As last step we will get the name for the recognised face: **_face_recognized->id_name_**

That's all to recognise a stored face.

### Memory Leaks
It is important to release the allocated memory blocks used for the detection/recognition process with **_dl_matrix3d_free()_**, **_dl_lib_free()_** and  **_esp_camera_fb_return()_**. Without freeing up the allocated memory, the available internal memory of the ESP32 (called **heap**) would get smaller and smaller over time. In the end the ESP32 would crash and restart. Memory leaks are often the reason for unstable code. With "IP-Address/uptime" you can check the last start time of the ESP32 and also the remaining free heap size.

```
Last start: 09.04.2021 13:37:31 Friday 
Free Heap: 185108
```

Here is the routine that handles the web request:
```
//*****************************************************************************
// Handle uptime request
// Last start date/time and free heap size (to detect memory leaks)
//
void handleUptime() {                          
  char text[80];
  snprintf(text, 80, "Last start: %s \nFree Heap: %d\n", time_str, ESP.getFreeHeap());
  web_server.send(200, "text/plain", text);  
}
```

## Housing
There are many nice 3D print housings for the ESP32-CAM available at Thingiverse. Example: https://www.thingiverse.com/thing:3652452  

# Updates
- Version 1.1, 04.10.2021: Updated root certificate to "ISRG Root X1".
- Version 1.0, 10.04.2021: Added last start date/time in /uptime.
- Version 0.8, 09.04.2021: Added uptime/free heap web page (/uptime).
- Version 0.7, 07.04.2021: Added error message for second HTTP connection. Only one WebSocket connection possible.
- Version 0.6, 07.04.2021: Corrected enroll name handling.
- Version 0.5, 06.04.2021: Code simplification.
- Version 0.2, 25.03.2021: Closed memory leak by freeing up buffers.
- Version 0.1, 23.03.2021: Initial version.
