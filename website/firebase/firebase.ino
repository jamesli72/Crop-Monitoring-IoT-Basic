#include <FirebaseESP32.h> //khai báo thư viện kết nối firebase
#include <ESP8266WiFi.h> //khai báo thư viện wifi
#include <WiFiClient.h>
#include <OneWire.h> //giao tiếp onewire
#include <DallasTemperature.h> //thư viện lấy giá trị ds18b20
#include <DHT.h> //khai bao cac thu vien su dung
const float BETA = 3950; //khai bao bien BETA tinh toan NTC

#define FIREBASE_HOST "https://longmon-5ed38-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "lV7rOnwbqjrswjNEFnmZjj1FTRNI2UG3y11FVV3g"


#define WIFI_SSID "Hờ Mờ Cờ" // Thay đổi tên wifi của bạn
#define WIFI_PASSWORD "hmc01062002" // Thay đổi password wifi của bạn

#define ONE_WIRE_BUS 5 //định nghĩa chân giao tiếp
OneWire oneWire(ONE_WIRE_BUS); //kết nối chân với function
DallasTemperature sensors(&oneWire); //lấy địa chỉ chân vào function

#define led 14 //dinh nghia chan led o gpio 26

#define DHTPIN 2 // dinh nghia chan dht o gpio 15 (adc2)
#define DHTTYPE DHT11 // dinh nghia loai DHT se su dung
DHT dht(DHTPIN, DHTTYPE); //dat ten va dua bien vao ham DHT

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


void setup() {
  Serial.begin(9600);// mở cổng giao tiếp
  delay(1000);

  sensors.begin();
  dht.begin(); // khoi dong ham dht
  pinMode(led, OUTPUT); //cau hinh ngo ra cho led
  pinMode(A0, INPUT);
  // Tạo một luồng (task) để chớp LED


  WiFi.begin (WIFI_SSID, WIFI_PASSWORD); //khởi tạo wifi
  Serial.print("Dang ket noi");
  while (WiFi.status() != WL_CONNECTED) { //kết nối wifi
    Serial.print(".");
    delay(500);
  }
  Serial.println ("");
  Serial.println ("Da ket noi WiFi!");
  Serial.println(WiFi.localIP()); //địa chỉ IP
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION); //kiểm tra version
  config.api_key = FIREBASE_AUTH; //đăng nhập
  config.database_url = FIREBASE_HOST;
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //khởi tạo firebase
  Firebase.reconnectWiFi(true); //kiểm tra kết nối wifi thêm 1 lần
}

void loop() {
  sensors.requestTemperatures(); //yêu cầu gửi nhiệt độ
  float t = sensors.getTempCByIndex(0); //đọc giá trị nhiệt độ

  float temperature = dht.readTemperature(); // đọc ADC DHT.
  float humidity = dht.readHumidity();// đọc ADC DHT.
  int analogValue = analogRead(34);// đọc ADC NTC.

  Serial.print("Nhiet do ds18b20:");
  Serial.println(t);
  Serial.print("Nhiet do dht22:");
  Serial.println(temperature);
  Serial.print("Do am dht11:");
  Serial.println(humidity);
  Serial.print("khi gas MQ2:");
  Serial.println(analogRead(A0));


  //Firebase.getInt(fbdo,"/esp32");
  //i = fbdo.intData();
  Firebase.setFloat(fbdo, "/nhietdo",temperature);
  Firebase.setInt(fbdo, "/doam",humidity);
  Firebase.setInt(fbdo, "/gas",analogRead(A0));
  delay(500);
}