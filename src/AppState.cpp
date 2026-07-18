#include <Adafruit_ST7789.h>
#include "AppConfig.h"
#include "AppState.h"

std::list<HashPiece> hashList = {};
std::vector<Chat *> chats = {};
Chat *currentChat = nullptr;
String error;

char username[64] = "test";
char password[64] = "test";

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
AppState currentState = STATE_MENU;

WiFiManager *wm;
WiFiManagerParameter *wm_username_field;
WiFiManagerParameter *wm_password_field;
WiFiClientSecure *client;
HTTPClient *https;
Preferences pref;

int selectedChatIndex = 0;
int selectedSettingsIndex = 0;

unsigned long btn1PressStart = 0;
unsigned long lastDebounce = 0;
unsigned long lastNetworkUpdate = 0;
bool guiUpdateNeeded = false;
bool redrawScreen = false;
unsigned long lastStatusUpdate = 0;
char CODE = '-';
bool WiFiEnabled = true;
