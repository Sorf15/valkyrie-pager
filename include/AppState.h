#pragma once

#include <Adafruit_ST7789.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <list>
#include <vector>

struct Chat {
  int id;
  String title;
  int pageCount = 0;
  int currentPage = 0;
  std::vector<String> hash;
  bool updateNeeded = true;

  Chat() : id(0), title(""), pageCount(0), currentPage(0), hash(), updateNeeded(true) {}
};

struct HashPiece {
  String *hash;
  String *value;
};

enum AppState { STATE_MENU, STATE_SETTINGS, STATE_CHAT_VIEW };

extern std::list<HashPiece> hashList;
extern std::vector<Chat *> chats;
extern Chat *currentChat;
extern String error;
extern char username[64];
extern char password[64];
extern Adafruit_ST7789 tft;
extern AppState currentState;
extern WiFiManager *wm;
extern WiFiManagerParameter *wm_username_field;
extern WiFiManagerParameter *wm_password_field;
extern WiFiClientSecure *client;
extern HTTPClient *https;
extern Preferences pref;
extern int selectedChatIndex;
extern int selectedSettingsIndex;
extern unsigned long btn1PressStart;
extern unsigned long lastDebounce;
extern unsigned long lastNetworkUpdate;
extern bool guiUpdateNeeded;
extern bool redrawScreen;
extern unsigned long lastStatusUpdate;
extern char CODE;
extern bool WiFiEnabled;

#define TOTAL_CHATS chats.size()
