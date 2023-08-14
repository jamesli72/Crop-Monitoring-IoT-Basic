#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"
#include "time.h"
#include "HardwareSerial.h"
HardwareSerial swSer(1);
//Provide the token generation process info.
#include <addons/TokenHelper.h>
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
//Replace with your network credentials
//const char* ssid = "James Ly";
//const char* password = "12332133";
const char* ssid = "Hờ Mờ Cờ";
const char* password = "hmc01062002";
//const char* ssid = "TitanFall.com";
//const char* password = "haogia123";
const char* ntpServer = "asia.pool.ntp.org";
//=============================================================
//=============================================================
String uid;
String databasePath;
String tempPath = "/temperature";
String humPath = "/humidity";
String timePath = "/timestamp";
String downLoad = "/URL";
String parentPath;
int timestamp;
FirebaseJson json;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 60000;  //1phut
#define button 10
#define DHTPIN 2
#define DHTTYPE DHT11

// Insert Firebase project API Key
#define SECRET "8oKfQ45RJPekQIhpf9sIkcpNkXetPMigZOaabFdw"
#define API_KEY "AIzaSyA9CPS8w-YZzqm93_SPDxOHbP4bF-8idWE"
#define DATABASE_URL "https://projectii-b2a91-default-rtdb.firebaseio.com/"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "lygiahuy561@gmail.com"
#define USER_PASSWORD "12332133"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "projectii-b2a91.appspot.com"

// Photo File Name to save in SPIFFS
//#define FILE_PHOTO "/data/photo.jpg"
unsigned int pictureNumber = 0;
String photo_path = "/data/photo" + String(pictureNumber) + ".jpg";

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

boolean takeNewPhoto = true;
DHT dht(DHTPIN, DHTTYPE);
//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;


byte data1,data2,data3,data4;
byte lastData1 = 10, lastData2 = 10, lastData3, lastData4;
//==========================================
//=======================================================

bool taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto(fs::FS& fs) {
  File f_pic = fs.open(photo_path.c_str());
  unsigned int pic_sz = f_pic.size();
  return (pic_sz > 100);
}
// ================================================
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}
// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs(void) {
  digitalWrite(4, HIGH);
  camera_fb_t* fb = NULL;  // pointer
  bool ok = 0;             // Boolean indicating if the picture has been taken correctly
  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");
    fb = esp_camera_fb_get();  
    delay(1000);//This is key to avoid an issue with the image being very dark and green. If needed adjust total delay time.

    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    // Photo file name
    Serial.printf("Picture file name: %s\n", photo_path.c_str());
    File file = SPIFFS.open(photo_path.c_str(), FILE_WRITE);
    delay(10);
    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    } else {
      file.write(fb->buf, fb->len);  // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(photo_path.c_str());
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while (!ok);
  digitalWrite(4, LOW);
}
//============================================================
void initWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) Serial.print(".");
  }
}
//============================================================
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  } else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}
//=============================================================
void initCamera() {
  // OV2640 camera module
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

  if (psramFound()) {
      config.frame_size = FRAMESIZE_UXGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
  sensor_t* s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV2640_PID) {
    s->set_brightness(s, 1);                  // up the brightness just a bit
    s->set_saturation(s, -2);                 // lower the saturation
    s->set_aec2(s, 1);                        // 0 = disable , 1 = enable // *** User settings = 1 (AEC DSP)
    s->set_bpc(s, 1);                         // 0 = disable , 1 = enable // *** User settings = 1 (BPC)
    s->set_gainceiling(s, (gainceiling_t)6);  // 0 to 6
  }
  Serial.print("init cam success..");
}
//================================================

//================================================
void sendData(int temp, int hum) {
    swSer.print("S");
    swSer.print(temp); swSer.print("A");
    swSer.println(hum);
    delay(100);
}
//================================================
void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  swSer.begin(9600, SERIAL_8N1, 14, 15);
  pinMode(4, OUTPUT);
  initWiFi();
  initSPIFFS();
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  delay(10);
  initCamera();
  configTime(0, 0, ntpServer);
  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  // Assign the RTDB URL (required)
  configF.database_url = DATABASE_URL;
  configF.host = "projectii-b2a91-default-rtdb.firebaseio.com";
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h
  configF.max_token_generation_retry = 5;
  dht.begin();
  Firebase.begin(&configF, &auth);
  fbdo.setResponseSize(4096);
  Firebase.reconnectWiFi(true);
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);
  // Update database path
  databasePath = "/UsersData/" + uid + "/readings/";
  //================================================
}

void loop() {
  if (Firebase.ready() && ((millis() - sendDataPrevMillis > timerDelay) || (sendDataPrevMillis == 0))) {
    sendDataPrevMillis = millis();
    //Get current timestamp
    timestamp = getTime();
    Serial.print("time: ");
    Serial.println(timestamp);
    taskCompleted = false;
    parentPath = databasePath + "/" + String(timestamp);
    sendData((int)dht.readTemperature(), (int)dht.readHumidity());
    json.set(tempPath.c_str(), String(dht.readTemperature()));
    json.set(humPath.c_str(), String(dht.readHumidity()));
    json.set(timePath.c_str(), String(timestamp));
    if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    }
    Serial.println(SPIFFS.exists(photo_path.c_str()) ? "File exists" : "File doesn't exist");
    if(pictureNumber > 10) pictureNumber = 0;
  }
  delay(100);
  //==================================================================================================

  if (Firebase.RTDB.getString(&fbdo, "/Switch/WaterPress")){      
        if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string){
              data1 = fbdo.stringData().toInt();             
              if(data1 != lastData1){
                    Serial.print("Data received 1: ");
                    Serial.println(data1); //print the data received from the Firebase database 
                    swSer.print("L");
                    swSer.println(data1);
                    lastData1 = data1;
                }
              }
            }
        else {Serial.println(fbdo.errorReason());} //print he error (if any) 

  if (Firebase.RTDB.getString(&fbdo, "/Switch/Blinder")){
        if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string){
              data2 = fbdo.stringData().toInt();
              if(data2 != lastData2){
              Serial.print("Data received 2: ");
              Serial.println(data2); //print the data received from the Firebase database 
                    swSer.print("B");
                    swSer.println(data2);
                    lastData2 = data2;
                }
              } 
            }
        else {Serial.println(fbdo.errorReason()); //print he error (if any)
              }

  if (Firebase.RTDB.getString(&fbdo, "/WaterPress/SetSpeed")){
        if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string){
              data3 = fbdo.stringData().toInt();
              if(data3 != lastData3){
              Serial.print("Data received 3: ");
              Serial.println(data3); //print the data received from the Firebase database 
              swSer.print("D");
              swSer.println(data3);
              lastData3 = data3;
              delay(10);
                } 
              }
            }
        else
              {Serial.println(fbdo.errorReason()); //print he error (if any)
              }
  
  if (Firebase.RTDB.getString(&fbdo, "/Slider/SetSpeed")){
        if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string){
              data4 = fbdo.stringData().toInt();
              if(data4 != lastData4){
              Serial.print("Data received 4: ");
              Serial.println(data4); //print the data received from the Firebase database 
                    swSer.print("C");
                    swSer.println(data4);
                    lastData4 = data4;
                    delay(10);
                } 
            }
      }
        else
              {Serial.println(fbdo.errorReason()); //print he error (if any)
              } 

  //================================================================================================
  if (Firebase.ready() && !taskCompleted) {
    Serial.print("Uploading picture... ");
    taskCompleted = true;
    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload( &fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */,  photo_path.c_str()/* path to local file */,
    mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, photo_path.c_str() /* path of remote file stored in the bucket */,
    "image/jpeg" /* mime type */)){

    json.set(downLoad.c_str(), fbdo.downloadURL().c_str());
    Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, databasePath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    } else {
    Serial.println(fbdo.errorReason());
  }
  pictureNumber++;
  photo_path = "/data/photo" + String(pictureNumber) + ".jpg";
  Serial.println(photo_path.c_str());
  }
}