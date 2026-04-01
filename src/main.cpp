#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "AudioTools.h"
#include "BluetoothA2DP.h"

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define I2S_LRC 26
#define I2S_BCLK 27
#define I2S_DIN 14

#define LED_PIN 13

#define BTN_NEXT 25
#define BTN_PREV 32
#define BTN_PLAY 33
#define POT_PIN 34

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

#define COLOR_BG 0x0000
#define COLOR_ACCENT 0x5ADB
#define COLOR_TEXT 0xFFFF
#define COLOR_SUBTEXT 0x7BEF
#define COLOR_BAR 0xFEA0

volatile esp_a2d_connection_state_t current_state = ESP_A2D_CONNECTION_STATE_DISCONNECTED;
volatile esp_a2d_audio_state_t audio_state = ESP_A2D_AUDIO_STATE_STOPPED;

volatile bool requestFullRedraw = false;
volatile bool requestTextRedraw = false;
volatile bool requestIconRedraw = false;
bool isSearchingScreen = false;

char song_title[60] = "Not Playing";
char song_artist[60] = "Unknown Artist";
char song_duration[10] = "0:00";

int last_volume = -1;

bool lastNextState = HIGH;
bool lastPrevState = HIGH;
bool lastPlayState = HIGH;

unsigned long lastScrollTime = 0;
int scrollIndex = 0;

void drawPlayPauseIcon()
{
  tft.fillRect(73, 78, 22, 24, COLOR_BG);

  if (audio_state == ESP_A2D_AUDIO_STATE_STARTED)
  {
    tft.fillRect(75, 80, 5, 20, COLOR_BAR);
    tft.fillRect(85, 80, 5, 20, COLOR_BAR);
  }
  else
  {
    tft.fillTriangle(75, 80, 75, 100, 93, 90, COLOR_BAR);
  }
}

void drawVolumeBar(int vol)
{
  tft.fillRect(10, 115, 140, 15, COLOR_BG);

  tft.setCursor(10, 115);
  tft.setTextColor(COLOR_SUBTEXT);
  tft.print("Vol:");
  tft.print(vol);
  tft.print("%");

  int barWidth = map(vol, 0, 100, 0, 90);
  tft.drawRect(60, 115, 90, 8, COLOR_SUBTEXT);
  tft.fillRect(60, 115, barWidth, 8, COLOR_BAR);
}

void drawTitleFrame()
{
  tft.fillRect(10, 25, 140, 10, COLOR_BG);
  tft.setCursor(10, 25);
  tft.setTextColor(COLOR_TEXT);

  int len = strlen(song_title);
  if (len <= 23)
  {
    // Fits on screen perfectly
    tft.print(song_title);
  }
  else
  {
    char buf[24];
    for (int i = 0; i < 23; i++)
    {
      int charIdx = (scrollIndex + i) % (len + 4);
      if (charIdx < len)
      {
        buf[i] = song_title[charIdx];
      }
      else
      {
        buf[i] = ' ';
      }
    }
    buf[23] = '\0';
    tft.print(buf);
  }
}

// Draw the metadata text
void drawTextAreas()
{
  tft.fillRect(0, 5, 160, 35, COLOR_BG);
  tft.fillRect(115, 50, 45, 15, COLOR_BG);

  tft.setTextSize(1);

  tft.setCursor(10, 10);
  tft.setTextColor(COLOR_SUBTEXT);
  tft.println(song_artist);

  drawTitleFrame();

  tft.setCursor(120, 55);
  tft.setTextColor(COLOR_TEXT);
  tft.print(song_duration);
}

void performFullRedraw()
{
  if (current_state != ESP_A2D_CONNECTION_STATE_CONNECTED)
    return;

  tft.fillScreen(COLOR_BG);

  tft.drawRoundRect(10, 45, 140, 4, 2, COLOR_SUBTEXT);

  tft.fillTriangle(60, 80, 60, 100, 45, 90, COLOR_ACCENT);
  tft.fillTriangle(100, 80, 100, 100, 115, 90, COLOR_ACCENT);

  drawTextAreas();
  drawPlayPauseIcon();
  if (last_volume != -1)
    drawVolumeBar(last_volume);
}

void metadata_callback(uint8_t id, const uint8_t *text)
{
  if (id == ESP_AVRC_MD_ATTR_TITLE)
  {
    strncpy(song_title, (char *)text, 59);
    song_title[59] = '\0';
    scrollIndex = 0;
    lastScrollTime = millis();
    requestTextRedraw = true;
  }
  if (id == ESP_AVRC_MD_ATTR_ARTIST)
  {
    strncpy(song_artist, (char *)text, 59);
    song_artist[59] = '\0';
    requestTextRedraw = true;
  }
  if (id == ESP_AVRC_MD_ATTR_PLAYING_TIME)
  {
    long ms = atol((char *)text);
    sprintf(song_duration, "%01ld:%02ld", ms / 60000, (ms % 60000) / 1000);
    requestTextRedraw = true;
  }
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  audio_state = state;
  requestIconRedraw = true;
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  current_state = state;
  Serial.printf("Bluetooth state changed: %d\n", state);

  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    requestFullRedraw = true;
  }
  else
  {
    isSearchingScreen = false;
  }
}

void setup()
{
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.setTextWrap(false);
  tft.fillScreen(COLOR_BG);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_PLAY, INPUT_PULLUP);

  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = I2S_BCLK;
  cfg.pin_ws = I2S_LRC;
  cfg.pin_data = I2S_DIN;
  i2s.begin(cfg);

  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_on_audio_state_changed(audio_state_changed);
  a2dp_sink.set_avrc_metadata_callback(metadata_callback);

  a2dp_sink.start("esp_speaker");
}

void loop()
{
  if (current_state != ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    if (!isSearchingScreen)
    {
      tft.fillScreen(COLOR_BG);
      tft.setCursor(20, 60);
      tft.setTextColor(ST7735_RED);
      tft.println("SEARCHING...");
      isSearchingScreen = true;
    }
    digitalWrite(LED_PIN, (millis() / 500) % 2);
    return;
  }

  if (requestFullRedraw)
  {
    requestFullRedraw = false;
    isSearchingScreen = false;
    tft.fillScreen(COLOR_BG);
    tft.setTextSize(1);
    tft.setCursor(10, 60);
    tft.setTextColor(ST7735_GREEN);
    tft.println("Bluetooth Connected");
    digitalWrite(LED_PIN, LOW);
    delay(1000);
    performFullRedraw();
    requestTextRedraw = false;
    requestIconRedraw = false;
  }
  else if (requestTextRedraw)
  {
    requestTextRedraw = false;
    drawTextAreas();
  }
  else if (requestIconRedraw)
  {
    requestIconRedraw = false;
    drawPlayPauseIcon();
  }

  // Execute Non-blocking Scrolling
  if (strlen(song_title) > 23)
  {
    if (millis() - lastScrollTime > 250)
    { // Scroll speed (250ms)
      scrollIndex++;
      if (scrollIndex >= strlen(song_title) + 4)
        scrollIndex = 0;
      drawTitleFrame();
      lastScrollTime = millis();
    }
  }

  long sum = 0;
  for (int i = 0; i < 32; i++)
  {
    sum += analogRead(POT_PIN);
  }

  int avgRead = sum / 32;
  int current_vol = map(avgRead, 0, 4095, 0, 100);

  if (abs(current_vol - last_volume) > 2)
  {
    a2dp_sink.set_volume(current_vol);
    drawVolumeBar(current_vol);
    last_volume = current_vol;
  }

  bool nextState = digitalRead(BTN_NEXT);
  bool prevState = digitalRead(BTN_PREV);
  bool playState = digitalRead(BTN_PLAY);

  if (nextState == LOW && lastNextState == HIGH)
  {
    a2dp_sink.next();
    delay(50);
  }

  if (prevState == LOW && lastPrevState == HIGH)
  {
    a2dp_sink.previous();
    delay(50);
  }

  if (playState == LOW && lastPlayState == HIGH)
  {
    if (audio_state == ESP_A2D_AUDIO_STATE_STARTED)
    {
      a2dp_sink.pause();
      audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
    }
    else
    {
      a2dp_sink.play();
      audio_state = ESP_A2D_AUDIO_STATE_STARTED;
    }

    drawPlayPauseIcon();
    delay(50);
  }

  lastNextState = nextState;
  lastPrevState = prevState;
  lastPlayState = playState;
}