#include <Arduino.h>
#include <WiFiManager.h>
#include <cstring>
#include "AppState.h"
#include "ConfigPortal.h"

void startConfigMode() {
  wm = new WiFiManager();
  wm_username_field = new WiFiManagerParameter("username", "Username", username, 64);
  wm_password_field = new WiFiManagerParameter("password", "Password", password, 64);
  wm->addParameter(wm_username_field);
  wm->addParameter(wm_password_field);
  Serial.println("Starting Config Portal...");
  tft.fillScreen(ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 20);
  tft.println("CONFIG MODE");
  tft.setCursor(10, 60);
  tft.println("1. Connect Phone to WiFi:");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, 80);
  tft.println("   Name: ESP32_SETUP");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 110);
  tft.println("2. Go to Address:");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, 130);
  tft.println("   192.168.4.1");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 160);
  tft.println("3. Enter WiFi Pass");

  wm->setConfigPortalTimeout(300);
  wm->startConfigPortal("ESP32_SETUP");

  strcpy(username, wm_username_field->getValue());
  strcpy(password, wm_password_field->getValue());
  pref.begin("creds", false);
  pref.putString("username", username);
  pref.putString("password", password);
  pref.end();
  delete wm;
  delete wm_password_field;
  delete wm_username_field;
  delay(2000);
}
