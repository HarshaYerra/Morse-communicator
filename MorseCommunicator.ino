/* B2 - Morse communicator
   Peer: B1 MAC 78:1C:3C:B7:B9:24
   Buttons (internal pull-up, button -> GND):
     DOT  -> GPIO 14
     DASH -> GPIO 27
     SPACE -> GPIO 26
     SEND -> GPIO 25
     BACK -> GPIO 33
   I2C LCD: SDA=21, SCL=22, addr=0x27
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// Buttons
const int PIN_DOT = 14;
const int PIN_DASH = 27;
const int PIN_SPACE = 26;
const int PIN_SEND = 25;
const int PIN_BACK = 33;

// Timing
const unsigned long ELEMENT_TIMEOUT = 500; // ms to finalize morse char
const unsigned long CURSOR_BLINK_MS = 500;

// Peer (B1)
uint8_t peerAddress[] = {0x78,0x1C,0x3C,0xB7,0xB9,0x24};

// Morse map (A-Z, 0-9, punctuation . , ? !)
struct MItem { const char* code; char ch; };
const MItem MORSE_TABLE[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..",'D'}, {".",'E'},
  {"..-.",'F'}, {"--.",'G'}, {"....",'H'}, {"..",'I'}, {".---",'J'},
  {"-.-",'K'}, {".-..",'L'}, {"--",'M'}, {"-.",'N'}, {"---",'O'},
  {".--.",'P'}, {"--.-",'Q'}, {".-.",'R'}, {"...",'S'}, {"-",'T'},
  {"..-",'U'}, {"...-",'V'}, {".--",'W'}, {"-..-",'X'}, {"-.--",'Y'},
  {"--..",'Z'},
  {"-----",'0'}, {".----",'1'}, {"..---",'2'}, {"...--",'3'}, {"....-",'4'},
  {".....",'5'}, {"-....",'6'}, {"--...",'7'}, {"---..",'8'}, {"----.",'9'},
  {".-.-.-",'.'}, {"--..--",','}, {"..--..",'?'}, {"-.-.--",'!'}
};
const int MORSE_TABLE_N = sizeof(MORSE_TABLE)/sizeof(MItem);

// Packet struct for ESP-NOW
typedef struct {
  char text[128];    // decoded text (so far or final)
  bool typing;       // true while composing (live update), false for final send
} Packet;

// State
String currentMorse = "";   // building sequence e.g. ".-"
String decodedText = "";    // line2 text
unsigned long lastElementTime = 0;
unsigned long lastBlink = 0;
bool cursorOn = true;
bool needDisplayUpdate = true;
bool showingReceivedMsg = false;
String receivedMsg = "";

// Button state tracking for proper debouncing
bool lastButtonState[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool buttonPressed[5] = {false, false, false, false, false};
unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0};
const unsigned long DEBOUNCE_MS = 50;

void setupPins(){
  pinMode(PIN_DOT, INPUT_PULLUP);
  pinMode(PIN_DASH, INPUT_PULLUP);
  pinMode(PIN_SPACE, INPUT_PULLUP);
  pinMode(PIN_SEND, INPUT_PULLUP);
  pinMode(PIN_BACK, INPUT_PULLUP);
}

char decodeMorse(const String &code){
  if (code.length() == 0) return '\0';
  for (int i=0;i<MORSE_TABLE_N;i++){
    if (String(MORSE_TABLE[i].code) == code) return MORSE_TABLE[i].ch;
  }
  return '?';
}

void sendPacket(bool typingFlag){
  Packet p;
  memset(&p,0,sizeof(p));
  strncpy(p.text, decodedText.c_str(), sizeof(p.text)-1);
  p.typing = typingFlag;
  esp_err_t res = esp_now_send(peerAddress, (uint8_t*)&p, sizeof(p));
  if (res != ESP_OK) {
    Serial.print("esp_now_send failed: "); Serial.println(res);
  }
}

void showLocal(){
  // Line1: current morse (trim to 16)
  lcd.clear();
  lcd.setCursor(0,0);
  
  if (showingReceivedMsg) {
    lcd.print("Received:");
    lcd.setCursor(0,1);
    String msg = receivedMsg;
    if (msg.length() > 16) msg = msg.substring(0, 16);
    lcd.print(msg);
  } else {
    String l1 = currentMorse;
    if (l1.length() > 16) l1 = l1.substring(l1.length()-16);
    lcd.print(l1);

    // Line2: decodedText + blinking cursor
    lcd.setCursor(0,1);
    String l2 = decodedText;
    if (cursorOn) l2 += "_";
    if (l2.length() > 16) l2 = l2.substring(l2.length()-16);
    lcd.print(l2);
  }
  needDisplayUpdate = false;
}

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status){
  Serial.print("SendStatus: "); Serial.println(status==ESP_NOW_SEND_SUCCESS?"OK":"FAIL");
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len){
  if (len >= sizeof(Packet)) {
    Packet *p = (Packet*)data;
    if (!p->typing) {
      // Final message received - display it indefinitely
      showingReceivedMsg = true;
      receivedMsg = String(p->text);
      needDisplayUpdate = true;
    }
  }
}

bool checkButton(int pin, int idx) {
  bool currentState = digitalRead(pin);
  bool triggered = false;
  
  if (currentState != lastButtonState[idx]) {
    lastDebounceTime[idx] = millis();
  }
  
  if ((millis() - lastDebounceTime[idx]) > DEBOUNCE_MS) {
    if (currentState == LOW && !buttonPressed[idx]) {
      buttonPressed[idx] = true;
      triggered = true;
    } else if (currentState == HIGH) {
      buttonPressed[idx] = false;
    }
  }
  
  lastButtonState[idx] = currentState;
  return triggered;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  lcd.init();
  lcd.backlight();

  setupPins();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Morse Communicator");
  lcd.setCursor(0,1);
  lcd.print("B2 Ready...");
  delay(1000);
  lcd.clear();

  // ESP-NOW init
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  }

  // Initial display update
  needDisplayUpdate = true;
  showLocal();
}

void loop(){
  unsigned long now = millis();

  // Blink cursor
  if (now - lastBlink >= CURSOR_BLINK_MS) {
    cursorOn = !cursorOn;
    lastBlink = now;
    needDisplayUpdate = true;
  }

  // Handle buttons with proper debouncing
  if (checkButton(PIN_DOT, 0)) {
    showingReceivedMsg = false;  // Clear received message when typing
    currentMorse += ".";
    lastElementTime = now;
    needDisplayUpdate = true;
    sendPacket(true); // live typing
  }
  if (checkButton(PIN_DASH, 1)) {
    showingReceivedMsg = false;  // Clear received message when typing
    currentMorse += "-";
    lastElementTime = now;
    needDisplayUpdate = true;
    sendPacket(true);
  }
  if (checkButton(PIN_SPACE, 2)) {
    showingReceivedMsg = false;  // Clear received message when typing
    // finalize current morse (if any)
    if (currentMorse.length() > 0) {
      char c = decodeMorse(currentMorse);
      decodedText += c;
      currentMorse = "";
    }
    decodedText += ' ';
    needDisplayUpdate = true;
    sendPacket(true);
  }
  if (checkButton(PIN_BACK, 3)) {
    showingReceivedMsg = false;  // Clear received message when typing
    // if building morse, remove last element, else remove last decoded char
    if (currentMorse.length() > 0) {
      currentMorse.remove(currentMorse.length()-1);
    } else if (decodedText.length() > 0) {
      decodedText.remove(decodedText.length()-1);
    }
    needDisplayUpdate = true;
    sendPacket(true);
  }

  if (checkButton(PIN_SEND, 4)) {
    // finalize any current morse element
    if (currentMorse.length() > 0) {
      char c = decodeMorse(currentMorse);
      decodedText += c;
      currentMorse = "";
    }
    // send final message (typing=false)
    sendPacket(false);
    // show sent on local for a short time then clear
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("Sent:");
    lcd.setCursor(0,1); lcd.print(decodedText.length()?decodedText:"(empty)");
    delay(800);
    decodedText = "";
    needDisplayUpdate = true;
    sendPacket(true); // clear typing state to peer (they will show Ready)
  }

  // finalize morse element if timeout reached
  if (currentMorse.length() > 0 && (now - lastElementTime >= ELEMENT_TIMEOUT)) {
    char c = decodeMorse(currentMorse);
    decodedText += c;
    currentMorse = "";
    needDisplayUpdate = true;
    sendPacket(true);
  }

  if (needDisplayUpdate) showLocal();
}