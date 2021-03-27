/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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

// Version 0.3, 27.03.2021, AK-Homberger

#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_http_server.h>
#include <esp_timer.h>
#include <esp_camera.h>
#include <fd_forward.h>
#include <fr_forward.h>
#include <fr_flash.h>
#include "camera_index.h"

// WLAN credentials
const char *ssid = "ssid";
const char *password = "password";

// Trigger URLs
const char *URL[] PROGMEM = {"https://www.virtualsmarthome.xyz/url_routine_trigger/...",
                             "https://www.virtualsmarthome.xyz/url_routine_trigger/..."
                            };

// Root certificate from Digital Signature Trust Co.
// Required for www.virtualsmarthome.xyz
// Will expire on 30. September 2021

const char rootCACertificate[] PROGMEM = R"=====(
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
-----END CERTIFICATE-----
)=====" ;

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

WiFiClientSecure client;        // Create HTTPS client
using namespace websockets;
WebsocketsServer socket_server; // Create WebSocket server

camera_fb_t *fb = NULL;

long current_millis;
long last_detected_millis = 0;
long last_sent_millis = 0;

#define LED_BUILTIN 4 
unsigned long led_on_millis = 0;
long interval = 3000;           // LED on for 3 seconds
bool face_recognised = false;

void app_facenet_main();
void app_httpserver_init();

typedef struct
{
  uint8_t *image;
  box_array_t *net_boxes;
  dl_matrix3d_t *face_id;
} http_img_process_result;

static inline mtmn_config_t app_mtmn_config()
{
  mtmn_config_t mtmn_config = {0};
  mtmn_config.type = FAST;
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;
  return mtmn_config;
}
mtmn_config_t mtmn_config = app_mtmn_config();

face_id_name_list st_face_list;
static dl_matrix3du_t *aligned_face = NULL;

httpd_handle_t camera_httpd = NULL;

typedef enum
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

typedef struct
{
  char enroll_name[ENROLL_NAME_LEN];
} httpd_resp_value;

httpd_resp_value st_name;


//*****************************************************************************
// Not sure if WiFiClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }
}


//*****************************************************************************
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

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
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if(i > 20) ESP.restart();
  }
  Serial.println("");
  Serial.println("WiFi connected");

  setClock();                           // Set Time/date for TLS certificate validation
  client.setCACert(rootCACertificate);  // Set Root CA certificate

  app_httpserver_init();                // Initialise HTTP server
  app_facenet_main();                   // Prepare fece recognition
  socket_server.listen(81);             // Start WebSocket server on port 81

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}


//*****************************************************************************
// Request URL for detected face "i".
// Set your own URLs
//
void ReqURL(int i) {

  if (millis() < last_sent_millis + 5000) return;   // Only once every 5 seconds
  last_sent_millis = millis();

  HTTPClient https;

  if (https.begin(client, URL[i])) {  // HTTPS
     // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
  
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        Serial.print("Heap size: ");
        Serial.println(xPortGetFreeHeapSize());  // Show free memory
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
// Main HTTP page as defined in "camera_index.h"
//
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  //httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  return httpd_resp_send(req, index_main, sizeof(index_main));
}


//*****************************************************************************
// Uri definitions for web server. Currently just main page "/".
//
httpd_uri_t index_uri = {
  .uri       = "/",
  .method    = HTTP_GET,
  .handler   = index_handler,
  .user_ctx  = NULL
};


//*****************************************************************************
// Start http server and register URI handler
//
void app_httpserver_init () {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    Serial.println("httpd_start");
    httpd_register_uri_handler(camera_httpd, &index_uri);
  }
}


//*****************************************************************************
// Read face data from flash memory
//
void app_facenet_main() {
  face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
  read_face_id_from_flash_with_name(&st_face_list);
}


//*****************************************************************************
// store new face information in flash
//
static inline int do_enrollment(face_id_name_list *face_list, dl_matrix3d_t *new_id) {
  ESP_LOGD(TAG, "START ENROLLING");
  int left_sample_face = enroll_face_id_to_flash_with_name(face_list, new_id, st_name.enroll_name);
  ESP_LOGD(TAG, "Face ID %s Enrollment: Sample %d",
           st_name.enroll_name,
           ENROLL_CONFIRM_TIMES - left_sample_face);
  return left_sample_face;
}


//*****************************************************************************
// Send face list to client via websocket connection.
//
static esp_err_t send_face_list(WebsocketsClient &client) {
  client.send("delete_faces"); // tell browser to delete all faces
  face_id_node *head = st_face_list.head;
  char add_face[64];
  
  for (int i = 0; i < st_face_list.count; i++) { // loop current faces
    sprintf(add_face, "listface:%s", head->id_name);
    client.send(add_face); //send face to browser
    head = head->next;
  }
}


//*****************************************************************************
// Delete all faces from flash
//
static esp_err_t delete_all_faces(WebsocketsClient & client) {
  delete_face_all_in_flash_with_name(&st_face_list);
  client.send("delete_faces");
}


//*****************************************************************************
// Handle web socket message sent from web client
//
void handle_message(WebsocketsClient & client, WebsocketsMessage msg) {
  if (msg.data() == "stream") {
    g_state = START_STREAM;
    client.send("STREAMING");
  }
  if (msg.data() == "detect") {
    g_state = START_DETECT;
    client.send("DETECTING");
  }
  if (msg.data().substring(0, 8) == "capture:") {
    g_state = START_ENROLL;
    char person[FACE_ID_SAVE_NUMBER * ENROLL_NAME_LEN] = {0,};
    msg.data().substring(8).toCharArray(person, sizeof(person));
    memcpy(st_name.enroll_name, person, strlen(person) + 1);
    client.send("CAPTURING");
  }
  if (msg.data() == "recognise") {
    g_state = START_RECOGNITION;
    client.send("RECOGNISING");
  }
  if (msg.data().substring(0, 7) == "remove:") {
    char person[ENROLL_NAME_LEN * FACE_ID_SAVE_NUMBER];
    msg.data().substring(7).toCharArray(person, sizeof(person));
    delete_face_id_in_flash_with_name(&st_face_list, person);
    send_face_list(client); // reset faces in the browser
  }
  if (msg.data() == "delete_all") {
    delete_all_faces(client);
  }
}


//*****************************************************************************
// Face with "name" detected.
// Request URL depending on name
//
void face_detected(char *name) {
  
  // Do a short blink with LED to show that a face was recognised
  
  digitalWrite(LED_BUILTIN, LOW); // LED off
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH); // LED on
  
  Serial.println("Face detected");
    
  if (strcmp(name, "Person1") == 0) {
    ReqURL(0);
  } else if (strcmp (name, "Person2") == 0) {
    ReqURL(1);
  }
  digitalWrite(LED_BUILTIN, LOW); // LED off
}


//*****************************************************************************
void loop() {

  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);
  http_img_process_result out_res = {0};
  out_res.image = image_matrix->item;

  // Do face recognition while no client is connected via web socket connection
  
  while (!socket_server.poll()) {   // No web socket connection
        
    // Prepare camera
    fb = esp_camera_fb_get();       // Get frame buffer pointer from camera
    
    out_res.net_boxes = NULL;
    out_res.face_id = NULL;
    fmt2rgb888(fb->buf, fb->len, fb->format, out_res.image);
    out_res.net_boxes = face_detect(image_matrix, &mtmn_config);

    if (millis() - interval > led_on_millis) { // current time - face recognised time > 3 secs
      digitalWrite(LED_BUILTIN, LOW); // LED off after 3 secs
    }

    if (out_res.net_boxes) {  // A general face has been recognised (no name so far)
      if (align_face(out_res.net_boxes, image_matrix, aligned_face) == ESP_OK) {
        
        // Switch LED on to give mor light for recognition
        digitalWrite(LED_BUILTIN, HIGH); // LED on
        led_on_millis = millis();        // Set on time
        
        out_res.face_id = get_face_id(aligned_face);  // Try to get face id for face
        
        if (st_face_list.count > 0) {  // Only try if we have faces registered at all
          face_id_node *f = recognize_face_with_name(&st_face_list, out_res.face_id);
          
          if (f) { // Face has been sucessfully identified
            face_detected(f->id_name);            
          }
        }
        dl_matrix3d_free(out_res.face_id);   // Free allocated memory
      }
      dl_lib_free(out_res.net_boxes->score); // Free allocated memory
      dl_lib_free(out_res.net_boxes->box); 
      if (out_res.net_boxes->landmark != NULL) dl_lib_free(out_res.net_boxes->landmark);
      dl_lib_free(out_res.net_boxes);
    }
  esp_camera_fb_return(fb);  // Release frame buffer
  fb = NULL;
  }  
  
  auto client = socket_server.accept();  // Accept web socket connection
  client.onMessage(handle_message);
    
  send_face_list(client);    // Send face list to client
  client.send("STREAMING");  // Set mode for client
   
  while (client.available()) {
    client.poll();

    if (millis() - interval > led_on_millis) { // Current time - face recognised time > 3 secs
      digitalWrite(LED_BUILTIN, LOW); //LED off
    }

    fb = esp_camera_fb_get();

    if (g_state == START_DETECT || g_state == START_ENROLL || g_state == START_RECOGNITION) {
      out_res.net_boxes = NULL;
      out_res.face_id = NULL;

      fmt2rgb888(fb->buf, fb->len, fb->format, out_res.image);

      out_res.net_boxes = face_detect(image_matrix, &mtmn_config);

      if (out_res.net_boxes) {
        if (align_face(out_res.net_boxes, image_matrix, aligned_face) == ESP_OK) {
          digitalWrite(LED_BUILTIN, HIGH); // LED on
          led_on_millis = millis(); // 
          
          out_res.face_id = get_face_id(aligned_face);
          last_detected_millis = millis();
          
          if (g_state == START_DETECT) {
            client.send("FACE DETECTED");
          }

          if (g_state == START_ENROLL) {
            int left_sample_face = do_enrollment(&st_face_list, out_res.face_id);
            char enrolling_message[64];
            sprintf(enrolling_message, "SAMPLE NUMBER %d FOR %s", ENROLL_CONFIRM_TIMES - left_sample_face, st_name.enroll_name);
            client.send(enrolling_message);
            
            if (left_sample_face == 0) {
              ESP_LOGI(TAG, "Enrolled Face ID: %s", st_face_list.tail->id_name);
              g_state = START_STREAM;
              char captured_message[64];
              sprintf(captured_message, "FACE CAPTURED FOR %s", st_face_list.tail->id_name);
              client.send(captured_message);
              send_face_list(client);
            }
          }

          if (g_state == START_RECOGNITION  && (st_face_list.count > 0)) {
            face_id_node *f = recognize_face_with_name(&st_face_list, out_res.face_id);
            
            if (f) {
              char recognised_message[64];
              sprintf(recognised_message, "Detected %s", f->id_name);
              face_detected(f->id_name);
              client.send(recognised_message);
            }
            else {
              client.send("FACE NOT RECOGNISED");
            }
          }
          dl_matrix3d_free(out_res.face_id);
        }
        dl_lib_free(out_res.net_boxes->score);
        dl_lib_free(out_res.net_boxes->box);
        if (out_res.net_boxes->landmark != NULL) dl_lib_free(out_res.net_boxes->landmark);
        dl_lib_free(out_res.net_boxes);
      }
      else {
        if (g_state != START_DETECT) {
          client.send("NO FACE DETECTED");
        }
      }

      if (g_state == START_DETECT && millis() - last_detected_millis > 500) { // Detecting but no face detected
        client.send("DETECTING");
      }
    }

    client.sendBinary((const char *)fb->buf, fb->len); // Send frame buffer (jpg picture) to client

    esp_camera_fb_return(fb);  // Release frame buffer
    fb = NULL;
  }
}
