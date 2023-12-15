 
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "lekdi";
const char* password = "teloletom";
long duration;
int distance;
const int trigPin = 13;
const int echoPin = 15;

int std_bersih = 91402;
int std_kotor = 90024;
int selisih = std_kotor - std_bersih;
String serverName = "47.254.244.187";
String serverPath = "/file-upload-toiled";
const int serverPort = 1895;
// Initialize Telegram BOT
String BOTtoken = "5872708952:AAHk7WHs1QAOdT1ZqF9te7JPaYh4YNp7Ejs";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "1109292261";

bool sendPhoto = false;
bool statusCamera = false;
bool statusBersih = false;
WiFiClient client;
WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

unsigned long previousMillisNew = 0; // Variabel untuk menyimpan waktu sebelumnya
const unsigned long intervalNew = 10800000; // Interval waktu dalam milisecond (3 jam = 3 x 60 x 60 x 1000)

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


String sendPhoto2() {
  String getAll;
  String getBody;

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"file\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(tail);
    
    esp_camera_fb_return(fb);
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        }
        else if (c != '\r') { getAll += String(c); }
        if (state==true) { getBody += String(c); }
        startTimer = millis();
      }
      if (getBody.length()>0) { break; }
    }
    Serial.println();
    client.stop();
//    int startIndex = getBody.indexOf("\"\":") + 9; // Menemukan indeks awal nilai ""
//    int endIndex = getBody.indexOf("\n", startIndex); // Menemukan indeks akhir nilai ""
//    String pixelsStr = getBody.substring(startIndex, endIndex); // Mengambil substring yang berisi nilai "pixels"
//    int pixels = pixelsStr.toInt(); // Mengkonversi nilai "pixels" menjadi integer
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, getBody);

  if(error){
    Serial.print("error ");
    Serial.println(error.c_str());
    return ""; 
  }

  const char* statusdata = doc["data"]["status"];
  Serial.print("sdasd ");
  Serial.println(statusdata);
 int startIndex = getBody.indexOf("\"pixels\":") + 9; // Mengambil posisi awal nilai pixels
  int endIndex = getBody.indexOf("}", startIndex); // Mengambil posisi akhir nilai pixels
  
  String pixelsValue = getBody.substring(startIndex, endIndex);
  int pixels = pixelsValue.toInt();
  
    Serial.print("Nilai pixels: ");
    Serial.println(pixels);
    Serial.print("Nilai pixels str: ");
    Serial.println(pixelsValue);
    float percentage = ( pixels-std_bersih )/selisih*100;
    
      String welcome = "Status Sensing Sensor pengambilan gambar\n";
      welcome += "pixel = "+String(pixels)+String("\n");
      welcome += "selisih = "+String(selisih)+String("\n");
      //welcome += "status = "+String(selisih>200?"kotor":"bersih")+String("\n");
     // bot.sendMessage(CHAT_ID, welcome, "");
      statusBersih = false;
      if (String(statusdata) == "True") {
      String welcome = "Toilet bersih, Nilai RGB Sesuai .";
//      String welcome = "Terimakasih telah menggunakan layanan ,,,\n";
//      welcome += "pixel = "+String(pixels)+String("\n");
//      welcome += "selisih = "+String(selisih)+String("\n");
      welcome += "status = "+String("Toilet Bersih")+String("\n");
      bot.sendMessage(CHAT_ID, welcome, "");
      statusBersih = true;
    }else{
      String welcome = "Toilet Kotor, Nilai RGB diluar batas \n";
//      welcome += "pixel = "+String(pixels)+String("\n");
//      welcome += "selisih = "+String(selisih)+String("\n");
//      welcome += "status = "+String("kotor")+String("\n");;
      welcome += "status = "+String("Silahkan bersihkan toilet, Ketik 'dibersihkan' saat membersihkan toilet")+String("\n");
      statusBersih = false;
      bot.sendMessage(CHAT_ID, welcome, "");
      }
    Serial.println("body");
    Serial.println(getBody);
  }
  else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }
  return getBody;
}

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  // Set LED Flash as output
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  // Config and init the camera
  configInitCamera();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP()); 
}

void loop() {
  if (sendPhoto) { //kondisi ketika foto
    int jarakCM = distanceCM();
    Serial.println(jarakCM+String("cm"));
    while(jarakCM<20){
      jarakCM = distanceCM();
      Serial.println(jarakCM+String("cm"));
      delay(1000);
    }
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(1000);
    Serial.println("Preparing photo");
    sendPhotoTelegram(); 
    sendPhoto2();
    sendPhoto = false;
    delay(1000); 
    digitalWrite(FLASH_LED_PIN, LOW);
  }
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  if(statusCamera){
    
  unsigned long currentMillis = millis(); // Membaca waktu saat ini
  // Periksa apakah sudah melewati interval waktu 3 jam
  if (currentMillis - previousMillisNew >= intervalNew) {
    previousMillisNew = currentMillis; // Simpan waktu saat ini sebagai waktu sebelumnya
    // Lakukan tugas yang perlu dilakukan setiap 3 jam di sini
    sendPhoto=true;
    String welcome = "Pengecekan tiap 3 jam\n";
    welcome += "mohon tunggu sistem sedang memeriksa kondisi \n";
    welcome += "\n";
    welcome += "\n";
    bot.sendMessage(CHAT_ID, welcome, ""); //sendchat
  }
  }
}
