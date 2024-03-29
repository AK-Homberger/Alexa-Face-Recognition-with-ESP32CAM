/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Alexa face detection with ESP32-CAM
// Based on this repository: https://github.com/robotzero1/esp32cam-access-control
// Is using https://www.virtualsmarthome.xyz/ "URL Routine Trigger" to trigger Alexa routines.
// URLs are requested from ESP32 via https after a defined face has been recognised.
// A Virtual "Door Bell" can be used in Alexa to trigger routines for each face/URL.

// Version 1.1, 04.10.2021, AK-Homberger

#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <esp_camera.h>
#include <fd_forward.h>
#include <fr_forward.h>
#include <fr_flash.h>
#include "camera_index.h"        // Main HTML page

#define CAMERA_MODEL_AI_THINKER  // Camera model for ESP32-CAM
#include "camera_pins.h"

#define FACE_ID_SAVE_NUMBER 7    // Maximum number of faces stored in flash
#define ENROLL_CONFIRM_TIMES 5   // Confirm same face 5 times

// WLAN credentials
const char *ssid = "ssid";
const char *password = "password";

// Trigger URLs: 7 URLs for maximum 7 enrolled faces (see FACE_ID_SAVE_NUMBER)
const char *URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                             "https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                             "",
                             "",
                             "",
                             "",
                             ""                             
                            };

// Root certificate from Digital Signature Trust Co.
// Required for www.virtualsmarthome.xyz
// Will expire on 30. September 2021

const char rootCACertificate[] PROGMEM = R"=====(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)=====";

WiFiClientSecure client;        // Create HTTPS client
WebServer web_server(80);       // Create Web Server on TCP port 80
using namespace websockets;
WebsocketsServer socket_server; // Create Web Socket server

bool no_socket_connection = true;             // Socket connection flag 

unsigned long last_detected_millis = 0;       // Timer last time face detected
unsigned long last_sent_millis = 0;           // Timer last time URL sent

#define LED_BUILTIN 4 
unsigned long led_on_millis = 0;              // Timer for LED on time
const int interval = 3000;                    // LED on for 3 seconds

// Face detection/recognition variables
camera_fb_t *fb;                              // Frame buffer pointer for picture from camera
box_array_t *detected_face;                   // Information for a detected face
dl_matrix3du_t *image_matrix;                 // Image matrix pointer
dl_matrix3du_t *aligned_face;                 // Aligned face pointer
dl_matrix3d_t *face_id;                       // Face ID pointer
face_id_node *face_recognized;                // Recognized face pointer

mtmn_config_t mtmn_config;                    // MTMN detection settings
face_id_name_list st_face_list;               // Name list for defined face IDs
char enroll_name[ENROLL_NAME_LEN+1];          // Name for face ID to be stored

char time_str[40];                            // Last start date/time

typedef enum        // Status definitions
{
  START_STREAM,
  START_DETECT,
  SHOW_FACES,
  START_RECOGNITION,
  START_ENROLL,
  ENROLL_COMPLETE,
  DELETE_ALL,
} en_fsm_state;
en_fsm_state g_state;


//*****************************************************************************
void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  esp_err_t result = camera_init();    // Initialise camera
  
  if (result != ESP_OK){
    Serial.printf("Camera init failed with error 0x%x", result);
    return;
  }

  // Start WiFi
  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if(i > 20) ESP.restart();           // Restart ESP32 in case of connection problems
  }
  Serial.println("");
  Serial.println("WiFi connected");

  setClock();                           // Set Time/date for TLS certificate validation
  client.setCACert(rootCACertificate);  // Set Root CA certificate

  mtmn_config = mtmn_init_config();     // Set MTMN face recognition details (default values)
  read_faces();                         // Read faces from flash
  
  image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);                 // Allocate memory for image matrix
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);  // Allocate memory for aligned face
  
  // Define web server events
  web_server.on("/", handleRoot);           // This is the main HTML page
  web_server.on("/uptime", handleUptime);   // To show uptime and heap size
  web_server.onNotFound(handleNotFound);    // Error page
  
  web_server.begin();                       // Start web server
  socket_server.listen(81);                 // Start web socket server on port 81

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}


//*****************************************************************************
// Configure camera settings
//
esp_err_t camera_init(void){
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // Frame size 1/4 VGA as required for face detection
  config.jpeg_quality = 10;
  config.fb_count = 1;
  
  return esp_camera_init(&config);
}


//*****************************************************************************
// Send main web page if no WebSocket connection is established
// Otherwise error message
//
void handleRoot() {
  if (no_socket_connection){
    web_server.send(200, "text/html", index_main); // index_main is defined in camera_index.h  
  } else {
    web_server.send(200, "text/plain", "Sorry, only one (WebSocket) connection possible!"); 
  }  
}


//*****************************************************************************
// Unknown request - Send error 404
//
void handleNotFound() {                          
  web_server.send(404, "text/plain", "File Not Found\n\n");
}


//*****************************************************************************
// Handle uptime request
// Last start date/time and free heap size (to detect memory leaks)
//
void handleUptime() {                          
  char text[80];
  snprintf(text, 80, "Last start: %s \nFree Heap: %d\n", time_str, ESP.getFreeHeap());
  web_server.send(200, "text/plain", text);  
}


//*****************************************************************************
// Not sure if WiFiClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
//
void setClock() {
  struct tm timeinfo;
  
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  getLocalTime(&timeinfo);
  strftime(time_str, sizeof(time_str), "%d.%m.%Y %T %A", &timeinfo);
  Serial.println(time_str);  
}


//*****************************************************************************
// Request URL for detected face "i".
//
void ReqURL(int i) {

  if (millis() < last_sent_millis + 5000) return;   // Only once every 5 seconds
  last_sent_millis = millis();

  HTTPClient https;

  if (https.begin(client, URL[i])) {  // Set HTTPS request for URL i
     
    int httpCode = https.GET();       // Request URL

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
  
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);      // Show web server response       
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      
      ESP.restart();  // Restart ESP in case of connection problems
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
}


//*****************************************************************************
// Read face data from flash memory
//
void read_faces() {
  face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  read_face_id_from_flash_with_name(&st_face_list);
}


//*****************************************************************************
// Store new face information in flash
//
int do_enrollment(face_id_name_list *face_list, dl_matrix3d_t *new_id) {
  int left_sample_face = enroll_face_id_to_flash_with_name(face_list, new_id, enroll_name);
  return left_sample_face;
}


//*****************************************************************************
// Send face list to client via WebSocket connection.
//
void send_face_list(WebsocketsClient &client) {
  client.send("delete_faces"); // Tell browser to delete all faces
  face_id_node *head = st_face_list.head;
  char add_face[64];
  
  for (int i = 0; i < st_face_list.count; i++) { // Loop current faces
    sprintf(add_face, "listface:%s", head->id_name);
    client.send(add_face); // Send face to browser
    head = head->next;
  }
}


//*****************************************************************************
// Delete all faces from flash
//
void delete_all_faces(WebsocketsClient &client) {
  delete_face_all_in_flash_with_name(&st_face_list);
  client.send("delete_faces");
}


//*****************************************************************************
// Handle web socket message (commands) sent from web client
//
void handle_message(WebsocketsClient &client, WebsocketsMessage msg) {
  
  if (msg.data() == "stream") {
    g_state = START_STREAM;
    client.send("STREAMING");
  }
  
  if (msg.data() == "detect") {
    g_state = START_DETECT;
    client.send("DETECTING");
  }
  
  if (msg.data().substring(0, 8) == "capture:") {

    if (st_face_list.count < FACE_ID_SAVE_NUMBER) {
      g_state = START_ENROLL;
      msg.data().substring(8).toCharArray(enroll_name, ENROLL_NAME_LEN);
      client.send("CAPTURING");
    } else {
      client.send("MAXIMUM REACHED");
    }
  }
  
  if (msg.data() == "recognise") {
    g_state = START_RECOGNITION;
    client.send("RECOGNISING");
  }
  
  if (msg.data().substring(0, 7) == "remove:") {
    char person[ENROLL_NAME_LEN+1];
    msg.data().substring(7).toCharArray(person, ENROLL_NAME_LEN);
    delete_face_id_in_flash_with_name(&st_face_list, person);
    send_face_list(client); // Reset faces in the browser
  }
  
  if (msg.data() == "delete_all") {
    delete_all_faces(client);
  }
}


//*****************************************************************************
// Face with "name" detected
// Request URL depending on name
//
void face_detected(char *name) {
  int i;
  
  // Do a short blink with LED to show that a face was recognised
  
  digitalWrite(LED_BUILTIN, LOW); // LED off
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH); // LED on
  
  Serial.println("Face detected");
  
  face_id_node *head = st_face_list.head;   // Set face list start

  for (i=0; i < st_face_list.count; i++)  { // Check all faces
    if (strcmp(name, head->id_name) == 0) ReqURL(i); // Name found, request URL number i
    head = head->next;  // Pointer to next face
  }
    
  digitalWrite(LED_BUILTIN, LOW); // LED off
}


//*****************************************************************************
void loop() {
  
  // Do face recognition while no client is connected via web socket connection
  
  while (!socket_server.poll()) {   // No web socket connection
    no_socket_connection = true;
    web_server.handleClient();
    
    if (millis() > led_on_millis + interval) { // LED off after 3 secs
      digitalWrite(LED_BUILTIN, LOW); 
    }
        
    fb = esp_camera_fb_get();       // Get frame buffer pointer from camera (JPEG picture)
    
    fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item); // JPEG to bitmap conversion
    
    detected_face = face_detect(image_matrix, &mtmn_config);      // Detect face

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
      dl_lib_free(detected_face->score);  // Free allocated memory
      dl_lib_free(detected_face->box); 
      dl_lib_free(detected_face->landmark);
      dl_lib_free(detected_face);
    }
  esp_camera_fb_return(fb);  // Release frame buffer  
  }  

  // Do face recognition while a client is connected via web socket connection
  
  auto client = socket_server.accept();  // Accept web socket connection
  client.onMessage(handle_message);
    
  send_face_list(client);    // Send face list to client
  client.send("STREAMING");  // Set mode for client
   
  while (client.available()) {  // Loop as long as a client is connected
    no_socket_connection = false;
    web_server.handleClient();
    client.poll();

    if (millis() > led_on_millis + interval) { // LED off after 3 secs
      digitalWrite(LED_BUILTIN, LOW); 
    }

    fb = esp_camera_fb_get();

    if (g_state == START_DETECT || g_state == START_ENROLL || g_state == START_RECOGNITION) {
      
      fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
      detected_face = face_detect(image_matrix, &mtmn_config);

      if (detected_face) {

        digitalWrite(LED_BUILTIN, HIGH); // LED on
        led_on_millis = millis();
                
        if (align_face(detected_face, image_matrix, aligned_face) == ESP_OK) {
          
          face_id = get_face_id(aligned_face);
          last_detected_millis = millis();
          
          if (g_state == START_DETECT) {
            client.send("FACE DETECTED");
          }

          if (g_state == START_ENROLL) {
            int left_sample_face = do_enrollment(&st_face_list, face_id);
            char enrolling_message[64];
            sprintf(enrolling_message, "SAMPLE NUMBER %d FOR %s", ENROLL_CONFIRM_TIMES - left_sample_face, enroll_name);
            client.send(enrolling_message);
            
            if (left_sample_face == 0) {
              g_state = START_STREAM;
              char captured_message[64];
              sprintf(captured_message, "FACE CAPTURED FOR %s", st_face_list.tail->id_name);
              client.send(captured_message);
              send_face_list(client);
            }
          }

          if (g_state == START_RECOGNITION  && (st_face_list.count > 0)) {
            face_recognized = recognize_face_with_name(&st_face_list, face_id);
            
            if (face_recognized) {
              char recognised_message[64];
              sprintf(recognised_message, "Detected %s", face_recognized->id_name);
              face_detected(face_recognized->id_name);
              client.send(recognised_message);
            }
            else {
              client.send("FACE NOT RECOGNISED");
            }
          }
          dl_matrix3d_free(face_id);  // Free allocated memory
        }
        dl_lib_free(detected_face->score);  // Free allocated memory
        dl_lib_free(detected_face->box);
        dl_lib_free(detected_face->landmark);
        dl_lib_free(detected_face);
      }
      else {
        if (g_state != START_DETECT) {
          client.send("NO FACE DETECTED");
        }
      }

      if (g_state == START_DETECT && millis() > last_detected_millis + 500) { // Detecting but no face detected
        client.send("DETECTING");
      }
    }

    client.sendBinary((const char *)fb->buf, fb->len); // Send frame buffer (JPEG picture) to client

    esp_camera_fb_return(fb);       // Release frame buffer    
  }  
}
