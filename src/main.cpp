#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include "TimesNRCyr8.h"
#include "AppConfig.h"
#include "AppState.h"
#include "ConfigPortal.h"
#include "DisplayUI.h"
#include "NetworkService.h"

void setup() {
  Serial.begin(115200);
  pinMode(BTN_ENTER_BACK, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 320);
  tft.setRotation(1);
  tft.setFont(&TimesNRCyr8pt8b);
  tft.setTextSize(1);
  tft.fillScreen(COLOR_BG);
  tft.setCursor(30, 50);
  tft.setTextSize(3);
  tft.println("Valkyrie");
  tft.setTextSize(1);

  pref.begin("creds", true);
  String savedUsername = pref.getString("username", "test");
  savedUsername.toCharArray(username, 64);
  String savedPassword = pref.getString("password", "test");
  savedPassword.toCharArray(password, 64);
  pref.end();

  tft.print("Connecting WiFi...");
  client = new WiFiClientSecure();
  https = new HTTPClient();
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  int tries = 0;
  client->setInsecure();
  while (WiFi.status() != WL_CONNECTED && tries < 20) 
  { 
    delay(500); tries++;
  }
  //???? timezones auto 
  configTime(3600 * 3, 0, "pool.ntp.org"); // GMT+2

  //DEBUG
  auto d = WiFi.localIP();
  auto failReason = WiFi.status();
  auto sig = WiFi.RSSI(); 
   __asm__("nop"); 

  Chat *chat = new Chat();
  chat->title = "Nothing to show!";
  chats.push_back(chat);
  updateCode();//includes refresh()

  drawStatusBar();
  drawMenu();
}

void loop() {
  //200ms delayy 
  if (millis() - lastDebounce < 200) return;
  lastDebounce = millis();

  if (!error.isEmpty()) {
    drawError();
    delay(500);
    error = "";
    if (currentState == STATE_MENU) drawMenu();
    else if (currentState == STATE_CHAT_VIEW) drawChatPage();
    else if (currentState == STATE_SETTINGS) drawSetting();
    return;
  }

  bool btn1 = !digitalRead(BTN_ENTER_BACK);
  bool btn2 = !digitalRead(BTN_DOWN);
  bool btn3 = !digitalRead(BTN_UP);

  if (!btn1 && !btn2 && !btn3 && !guiUpdateNeeded) {
    //status bar update once in 5sec
    if (millis() - lastStatusUpdate > 5000) {
      drawStatusBar();
      lastStatusUpdate = millis();
    }

    if (WiFiEnabled && (millis() - lastNetworkUpdate > 10000)) {
      if (WiFi.status() == WL_CONNECTED) updateCode();
      else { WiFi.disconnect(); WiFi.begin(); }
      lastNetworkUpdate = millis();
    }

    if (redrawScreen) {
      redrawScreen = false;
      if (currentState == STATE_MENU) { drawStatusBar(); drawMenu(); }
      else if (currentState == STATE_CHAT_VIEW) { drawChatPage(); drawStatusBar(); }
      else if (currentState == STATE_SETTINGS) { drawSetting(); drawStatusBar(); }
    }
    return;
  }

  if (currentState == STATE_MENU) {
    if (btn1) {
      if (btn1PressStart == 0) { btn1PressStart = millis(); guiUpdateNeeded = true; }
      else if (millis() - btn1PressStart > 3000) {
        guiUpdateNeeded = false;
        currentState = STATE_SETTINGS;
        selectedSettingsIndex = 0;
        drawSetting();
        btn1PressStart = 0;
        while (!digitalRead(BTN_ENTER_BACK)) delay(200);
      }
    } else if (btn1PressStart != 0) {
      btn1PressStart = 0;
      guiUpdateNeeded = false;
      currentChat = chats.at(selectedChatIndex);
      currentState = STATE_CHAT_VIEW;
      drawStatusBar();
      drawChatPage();
    }

    if (btn2) {
      selectedChatIndex++;
      if (selectedChatIndex >= TOTAL_CHATS) selectedChatIndex = 0;
      drawMenu();
    }
    
    if (btn3) {
      selectedChatIndex--;
      if (selectedChatIndex < 0) selectedChatIndex = TOTAL_CHATS - 1;
      drawMenu();
    }
  } else if (currentState == STATE_CHAT_VIEW) {
    if (btn1 || currentChat == nullptr) { currentState = STATE_MENU; drawStatusBar(); drawMenu(); }
    if (btn2) {
      currentChat->currentPage++;
      if (currentChat->currentPage >= currentChat->pageCount) currentChat->currentPage = 0;
      drawChatPage();
    }
    if (btn3) {
      currentChat->currentPage--;
      if (currentChat->currentPage < 0) currentChat->currentPage = currentChat->pageCount == 0 ? 0 : currentChat->pageCount - 1;
      drawChatPage();
    }
  } else {
    if (btn1) {
      switch (selectedSettingsIndex) {
        case 0:
          WiFiEnabled = !WiFiEnabled;
          if (WiFiEnabled) WiFi.begin(); else WiFi.disconnect(true);
          drawSetting();
          break;
        case 1: startConfigMode(); drawSetting(); break;
        case 2: drawInfo(); delay(3000); drawSetting(); break;
        default: currentState = STATE_MENU; drawStatusBar(); drawMenu(); break;
      }
    }
    if (btn2) {
      selectedSettingsIndex++;
      if (selectedSettingsIndex >= TOTAL_SETTING) selectedSettingsIndex = 0;
      drawSetting();
    }
    if (btn3) {
      selectedSettingsIndex--;
      if (selectedSettingsIndex < 0) selectedSettingsIndex = TOTAL_SETTING - 1;
      drawSetting();
    }
  }
}
