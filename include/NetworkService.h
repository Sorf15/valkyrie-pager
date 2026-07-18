#pragma once

#include <Arduino.h>
#include "AppState.h"

const char *fetchText();
void refresh();
void updateCode();
void updateChatNames();
void updatePageLen(Chat *chat);
void updateHash(Chat *chat);
void downloadContent(const String &hash);
std::vector<String> splitString(const String &str, char delimiter);
std::vector<String> splitString(const String &str, char delimiter, const int &limit);
