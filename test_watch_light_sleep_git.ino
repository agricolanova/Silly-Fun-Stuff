#include <WiFi.h>
#include <time.h>
#include "esp_sleep.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

uint8_t hh, mm, ss;

byte last_ss = 99;
byte last_mm = 99;

#define BUTTON_PIN 33
//#define deep_PIN 17
#define LED_PIN 15
#define trig_Pin 21
#define echo_Pin 22


const char* ssid = "wifi";
const char* password = "wifipass";


const char* ssid2 = "backupwifi";
const char* password2 = "wifipass2";


uint32_t targetTime = 0;       // for next 1 second timeout

byte omm = 99;
byte oss = 99;
bool initial = 1;
byte xcolon = 0;
unsigned int colour = 0;

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

//uint8_t hh=conv2d(__TIME__), mm=conv2d(__TIME__+3), ss=conv2d(__TIME__+6);  // Get H, M, S from compile time old






void setup(void) {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  targetTime = millis() + 1000; 
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  // put your setup code here, to run once:
  //dht.begin();
  delay(1000);
  pinMode(trig_Pin, OUTPUT);  
	pinMode(echo_Pin, INPUT);  
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  //esp_sntp_servermode_dhcp(1);  // (optional)
  int attempts = 0;
  int attempts2 =0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED) {
  Serial.println("\nWechsle zu Backup WLAN...");
  WiFi.disconnect(true,true);   
  delay(1000);
  WiFi.mode(WIFI_OFF);           //
  delay(500);

  WiFi.mode(WIFI_STA);           // 
  delay(500);
  WiFi.begin(ssid2, password2);
  }
  while (WiFi.status() != WL_CONNECTED && attempts2 < 20) {
    delay(500);
    Serial.print(".");
    attempts2++;
  }
  if (WiFi.status() != WL_CONNECTED) {
  Serial.println("\n no wifi...");
  hh=10;
  mm=10;
  ss=10;
  }



  //sntp_set_time_sync_notification_cb(timeavailable);
 // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);  // the old one
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");
 // gpio_wakeup_enable( GPIO_NUM_21 , GPIO_INTR_LOW_LEVEL ); // 

  // Light Sleep aktivieren
 // esp_light_sleep_start();
 struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    hh = timeinfo.tm_hour;
    mm = timeinfo.tm_min;
    ss = timeinfo.tm_sec;
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  drawClock();


}

void updateTime() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis >= 1000) {
    lastMillis += 1000;
    ss++;
    if (ss >= 60) {
      ss = 0;
      mm++;
    }
    if (mm >= 60) {
      mm = 0;
      hh++;
    }
    if (hh >= 24) {
      hh = 0;
    }
  }
}


void drawClock() {
  if (mm != last_mm) {
    last_mm = mm;
    tft.fillRect(0, 0, 280, 50, TFT_BLACK);
    tft.setTextColor(0xFBE0, TFT_BLACK);
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hh, mm);
    tft.drawString(timeStr, 10, 10, 7);
    //tft.drawString(String(hh) + ":" + String(mm), 10, 10, 7);
  }
  // Sekunden separat
  if (ss != last_ss) {
    last_ss = ss;
     char secStr[3];              // "00" + Nullterminator
    sprintf(secStr, "%02d", ss);
    tft.setTextColor(0xFBE0, TFT_BLACK);
    tft.drawString(secStr, 160, 37, 4);
  }
}

float readDistance() {
  float duration;
  digitalWrite(trig_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Pin, LOW);
  duration = pulseIn(echo_Pin, HIGH, 30000);
  return (duration * 0.0343) / 2;
}

void drawDistance(float distance) {
  
   // Note: the new fonts do not draw the background colour

  tft.fillRect(0, 95, 280, 10, TFT_BLACK);
  tft.setTextColor(tft.color565(135, 206, 250), TFT_BLACK);
  String sdistance = String(distance) + " cm      ";
tft.drawString(sdistance, 82, 95, 4);  
}
void drawPierre(float distance) {
  
   // Note: the new fonts do not draw the background colour

//  tft.fillRect(0, 70, 280, 10, TFT_BLACK);
  tft.setTextColor(tft.color565(255, 255, 0), TFT_BLACK);
  String sdistance = String(distance) + " cm  ";
 // tft.drawString(String(distance) + " Pierre is here", 30, 70, 3);
tft.drawString("Pierre ist hier", 42, 70, 4);

//tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  
}


void loop() {

  uint32_t sleepStart = micros();

updateTime();
drawClock();
float d = readDistance();
drawDistance(d);
drawPierre(d);
uint32_t workTime = micros() - sleepStart;
uint32_t sleepTime = 1000000 - workTime;

if (sleepTime > 0) {
  esp_sleep_enable_timer_wakeup(sleepTime);
  esp_light_sleep_start();
}
}







