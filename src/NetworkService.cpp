#include <Arduino.h>
#include <HTTPClient.h>
#include <mbedtls/base64.h>
#include <algorithm>
#include "AppConfig.h"
#include "AppState.h"
#include "NetworkService.h"

const char *fetchText() {
  if (currentChat == nullptr) return nullptr;
  if (currentChat->updateNeeded) updateCode();
  int page = currentChat->currentPage;
  if (page >= currentChat->pageCount) return nullptr;
  if (page >= currentChat->hash.size()) {
    updateHash(currentChat);
    if (page >= currentChat->hash.size()) return "Hashing error\nIndex out of range!";
  }
  String hash1 = currentChat->hash.at(page);
  if (hash1 == "") return nullptr;
  for (auto it = hashList.begin(); it != hashList.end(); ++it) {
    if (it->hash->equals(hash1)) {
      HashPiece hp = *it;
      hashList.erase(it);
      hashList.push_front(hp);
      return hp.value->c_str();
    }
  }
  downloadContent(hash1);
  for (auto it = hashList.begin(); it != hashList.end(); ++it) {
    if (it->hash->equals(hash1)) return it->value->c_str();
  }
  return nullptr;
}

void downloadContent(const String &hash) {
  if (!WiFiEnabled) return;
  String decoded = "Empty page!";
  Chat *chat = currentChat;
  if (chat == nullptr) return;
  int block = chat->currentPage / 16;
  String uri = String(API_BASE_URL) + "/api/pages/cont/" + String(chat->id) + "?username=" + String(username) + "&password=" + password + "&block=" + block;
  if (https->begin(*client, uri)) {
    int httpCode = https->GET();
    if (httpCode <= 0) { error = "downloadContent:\nFailed to connect. Network issues!"; return; }
    if (httpCode != 200) { error = https->getString(); return; }
    String response = https->getString();
    https->end();
    response.remove(0, 2);
    std::vector<String> content = splitString(response, ';');
    for (auto it = content.begin(); it != content.end(); it++) {
      String d_hash = it->substring(0, 24);
      it->remove(0, 25);
      String encoded = *it;
      if (!encoded.isEmpty()) {
        encoded.replace('-', '+');
        encoded.replace('_', '/');
        size_t inputLen = strlen(encoded.c_str());
        size_t outLen = inputLen + 1;
        unsigned char outputBuffer[outLen];
        size_t actualOut = 0;
        int result = mbedtls_base64_decode(outputBuffer, outLen, &actualOut, (const unsigned char *)encoded.c_str(), inputLen);
        if (result == 0) { outputBuffer[actualOut] = '\0'; decoded = String((char *)outputBuffer); }
      }
      HashPiece hp;
      hp.hash = new String(d_hash);
      hp.value = new String(decoded);
      if (hashList.size() >= MAX_HASH_BLOCKS) {
        HashPiece &old = *(hashList.rbegin());
        delete old.hash;
        delete old.value;
        hashList.pop_back();
      }
      hashList.push_front(hp);
    }
  }
}

void refresh() {
  if (!WiFiEnabled) return;
  int savedId = -1;
  String savedTitle;
  if (currentChat != nullptr) { savedId = currentChat->id; savedTitle = currentChat->title; }


  updateChatNames();
  //updating turns currentChat to nullptr
  //restore current chat
  if (savedId != -1 && savedId < chats.size() && chats[savedId]->title.equals(savedTitle)) {
    currentChat = chats[savedId];
  } else if (currentState == STATE_CHAT_VIEW) {
    currentState = STATE_MENU;
    redrawScreen = true;
  }
  if (currentChat != nullptr) 
  { 
    updatePageLen(currentChat); updateHash(currentChat); 
  }
}

void updateHash(Chat *chat) {
  if (chat == nullptr) return;
  if (!WiFiEnabled) { chat->updateNeeded = true; return; }

  int block = chat->currentPage / 16;
  String uri = String(API_BASE_URL) + "/api/pages/hash/" + String(chat->id) + "?username=" + String(username) + "&password=" + password + "&block=" + block;
  if (https->begin(*client, uri)) {
    int httpCode = https->GET();
    if (httpCode <= 0) { error = "updateHash:\nFailed to connect. Network issues!"; chat->updateNeeded = true; return; }
    if (httpCode != 200) { error = https->getString(); return; }
    
    String response = https->getString();
    https->end();
    response.remove(0, 2);
    
    if (chat->hash.size() <= block * 16 + 15) chat->hash.resize((block + 1) * 16);
    std::vector<String> hashes = splitString(response, ';', 16);
    for (size_t i = 0; i < hashes.size(); i++) chat->hash.at(block * 16 + i) = hashes[i];
    
    chat->updateNeeded = false;
  }
}

void updatePageLen(Chat *chat) {
  if (chat == nullptr) return;
  if (!WiFiEnabled) { chat->updateNeeded = true; return; }
  
  String uri = String(API_BASE_URL) + "/api/pages/len/" + String(chat->id) + "?username=" + String(username) + "&password=" + password;
  if (https->begin(*client, uri)) {
    int httpCode = https->GET();
    if (httpCode <= 0) { error = "updatePageLen:\nFailed to connect. Network issues!"; chat->updateNeeded = true; return; }
    if (httpCode != 200) { error = https->getString(); return; }
    
    String response = https->getString();
    https->end();
    response.remove(0, 1);
    chat->pageCount = response.toInt();
    if (chat->currentPage >= chat->pageCount) chat->currentPage = chat->pageCount - 1;
  }
}

void updateCode() {
  if (!WiFiEnabled) { CODE = '-'; return; }
  
  String uri = String(API_BASE_URL) + "/api/chat/updateCode?username=" + String(username) + "&password=" + password;
  if (https->begin(*client, uri)) {
    int httpCode = https->GET();
    if (httpCode <= 0) { error = "updateCode:\nFailed to connect. Network issues!"; return; }
    if (httpCode != 200) { error = https->getString(); return; }
  
    const char newCode = https->getString()[0];
    https->end();
    if (newCode != CODE) { CODE = newCode; refresh(); }
  }
}

std::vector<String> splitString(const String &str, char delimiter) { return splitString(str, delimiter, -1); }

std::vector<String> splitString(const String &str, char delimiter, const int &limit) {
  std::vector<String> tokens;
  int pos_start = 0, pos_end = 0, slices = 0;
  
  while ((pos_end = str.indexOf(delimiter, pos_start)) != -1) {
    tokens.push_back(str.substring(pos_start, pos_end));
    pos_start = pos_end + 1;
    if (limit != -1 && ++slices >= limit) break;
  }

  // Adds the last part of the string after the last delimiter
  if (pos_start < str.length()) tokens.push_back(str.substring(pos_start));
  return tokens;
}

void updateChatNames() {
  if (!WiFiEnabled) { CODE = '-'; return; }
  currentChat = nullptr;
  
  String uri = String(API_BASE_URL) + "/api/chat/names?username=" + String(username) + "&password=" + password;
  if (https->begin(*client, uri)) {
    int httpCode = https->GET();
    if (httpCode <= 0) { error = "updateChatNames\nFailed to connect. Network issues!"; CODE = '-'; return; }
    if (httpCode != 200) { error = https->getString(); return; }
   
    String response = https->getString();
    https->end();
    response.remove(0, 1);
    int size = response.toInt();
    
    int s = 1;
    int sizet = size;
    while (sizet > 0) { sizet /= 10; s++; }
    response.remove(0, s);
    //reverse order
    std::vector<String> split = splitString(response, ';', size);
    int index = 0; //next place
    std::reverse(chats.begin(), chats.end());//old ... new
    for (auto it = split.begin(); it != split.end(); ++it) {// split: old .. new
      if (index < chats.size()) {
        bool found = false;
        while (index < chats.size()) {
          if (chats[index]->title.equals(*it)) { found = true; index++; break; }
          delete chats[index];
          chats.erase(chats.begin() + index);
        }
        if (found) continue;
      }
      Chat *chat = new Chat();
      chat->title = *it;
      chats.push_back(chat);
      index++;
    }
    chats.resize(index);

    std::reverse(chats.begin(), chats.end());//new ... old
    //fixing id's
    for (int i = 0; i < chats.size(); i++) chats[i]->id = i;
    redrawScreen = true;
  }
}
