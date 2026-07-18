#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "AppConfig.h"
#include "AppState.h"
#include "DisplayUI.h"
#include "NetworkService.h"

float readBattery() {
  return (analogRead(BAT_PIN) / 4095.0) * 3.3 * 2.0;
}

//TOP
void drawStatusBar() {
  tft.fillRect(0, 0, TFT_WIDTH, BAR_HEIGHT, COLOR_BAR);
  tft.setTextColor(COLOR_TEXT, COLOR_BAR);

  //LEFT
  tft.setCursor(5, 16);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100)) {
    tft.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  } else {
    if (WiFi.status() == WL_CONNECTED) {
      configTime(3600 * 3, 0, "pool.ntp.org");
    }
    tft.print("--:--");
  }

  //MIDDLE
  tft.setCursor(120, 16);
  if (!WiFiEnabled) {
    tft.print("WiFi: OFF");
  } else if (WiFi.status() == WL_CONNECTED) {
    tft.print("WiFi: "); tft.print(WiFi.RSSI());
  } else {
    tft.print("No WiFi");
  }

  //RIGHT
  tft.setCursor(260, 16);
  tft.print(readBattery(), 1);
  tft.print("V");
}

void drawBottomBar() {
  tft.fillRect(0, TFT_HEIGHT - BAR_HEIGHT, TFT_WIDTH, BAR_HEIGHT, COLOR_BAR);
  tft.setTextColor(COLOR_TEXT, COLOR_BAR);
  tft.setTextSize(1);
  tft.setCursor(80, TFT_HEIGHT - 8);

  if (currentState == STATE_MENU) {
    tft.print("Chats: "); tft.print(TOTAL_CHATS);
  } else if (currentState == STATE_CHAT_VIEW) {
    int tot = currentChat == nullptr ? 0 : currentChat->pageCount;
    if (tot == 0) {
      tft.print("Page 0 / 0");
      return;
    }
    int cupg = currentChat == nullptr ? 0 : currentChat->currentPage;
    tft.print("Page "); tft.print(cupg + 1); tft.print(" / "); tft.print(tot);
  }
}

void drawMenu() {
  // clear
  tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);
  tft.setTextSize(1);
  int startY = 45;
  int s = TOTAL_CHATS;
  if (selectedChatIndex >= s) selectedChatIndex = 0;
  int chatBlock = selectedChatIndex / 6;
  int size = ((chatBlock + 1) * 6 > s) ? s % 6 : 6;
  for (int i = 0; i < size; i++) {
    // highlight
    if (chatBlock * 6 + i == selectedChatIndex) {
      tft.fillRect(10, startY + (i * 30) - 18, 300, 26, COLOR_SELECT);
      tft.setTextColor(COLOR_SEL_TXT, COLOR_SELECT);
    } else {
      tft.setTextColor(COLOR_TEXT, COLOR_BG);
    }
    tft.setCursor(15, startY + (i * 30));
    tft.print(chats.at(chatBlock * 6 + i)->title);
  }
  drawBottomBar();
}

void drawError() {
  tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);

  tft.setTextColor(COLOR_RED, COLOR_BG);
  tft.setCursor(15, 45);
  tft.print(error);
}

void drawInfo() {
  tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(15, 45);
  tft.print("WiFi SSID: "); tft.print(WiFi.SSID());
  tft.print("\nWiFi Pass: "); tft.print(WiFi.psk());
  tft.print("\nUsername: "); tft.print(username);
  tft.print("\nUser Pass: "); tft.print(password);
}

void drawChatPage() {
  //clear
  tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(0, BAR_HEIGHT + 20);
  tft.print("Loading...");

   //raise condition if chats were changed ! pohui
  size_t tries = 0;
  while (WiFiEnabled && currentChat != nullptr && currentChat->updateNeeded && tries < 3) {
    updatePageLen(currentChat);
    updateHash(currentChat);
    if (!error.isEmpty()) {
      drawError();
      delay(500);
      error = "";
    }
    tries++;
  }

  int count = currentChat == nullptr ? 0 : currentChat->pageCount;
  if (count == 0) {
    tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);
    tft.setCursor(0, BAR_HEIGHT + 20);
    tft.print("Empty chat!");
  } else {
    const char *text = fetchText();
    if (currentChat == nullptr) {
      currentState = STATE_MENU;
      drawStatusBar();
      drawMenu();
      return;
    }
    if (text != nullptr) {
      tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);
      tft.setCursor(10, BAR_HEIGHT + 20);
      tft.print(text);
    }
  }
  drawBottomBar();
}

static void printSettingText(const int &y, const String &txt, const bool &selected) { //y = startY + (i * 30)
  if (selected) {
    tft.fillRect(10, y - 18, 300, 26, COLOR_SELECT);
    tft.setTextColor(COLOR_SEL_TXT, COLOR_SELECT);
  } else {
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
  }
  tft.setCursor(15, y);
  tft.print(txt);
}

void drawSetting() {
  tft.fillRect(0, BAR_HEIGHT, TFT_WIDTH, TFT_HEIGHT - 2 * BAR_HEIGHT, COLOR_BG);
  tft.setTextSize(1);
  int startY = 45;
  String wifiState = WiFiEnabled ? "WiFi.....ON" : "WiFi.....OFF";

  //WIFI CONFIG INFO EXIT
  printSettingText(startY, wifiState, selectedSettingsIndex == 0);
  printSettingText(startY + 30, "Enter Config Mode...", selectedSettingsIndex == 1);
  printSettingText(startY + 60, "Show Info", selectedSettingsIndex == 2);
  printSettingText(startY + 90, "Exit", selectedSettingsIndex == 3);
}
