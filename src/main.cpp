#include <Arduino.h>
#include "AudioTools.h"
#include "BluetoothA2DP.h"

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

volatile esp_a2d_connection_state_t current_state = ESP_A2D_CONNECTION_STATE_DISCONNECTED;

int last_stable_volume = -1;
bool lastPlayPauseState = HIGH;
bool lastPrevState = HIGH;
bool lastNextState = HIGH;

bool isPlaying = false;

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  current_state = state;

  Serial.print("Bluetooth state changed: ");
  Serial.println(a2dp_sink.to_str(state));

  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    Serial.println("Device paired");
    digitalWrite(LED_PIN, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting..");

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
  Serial.println("Starting A2DP Sink as 'esp_speaker'");
  a2dp_sink.start("esp_speaker");
}

void loop()
{
  if (current_state != ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    digitalWrite(LED_PIN, (millis() / 500) % 2);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }

  long sum = 0;
  for (int i = 0; i < 64; i++)
  {
    sum += analogRead(POT_PIN);
  }

  int avgRead = sum / 64;
  int current_vol = map(avgRead, 0, 4095, 0, 100);

  if (abs(current_vol - last_stable_volume) > 3)
  {
    a2dp_sink.set_volume(current_vol);
    last_stable_volume = current_vol;
    Serial.printf("Volume: %d%%\n", current_vol);
  }

  if (digitalRead(BTN_NEXT) == LOW)
  {
    a2dp_sink.next();
    delay(300);
  }
  if (digitalRead(BTN_PREV) == LOW)
  {
    a2dp_sink.previous();
    delay(300);
  }
  if (digitalRead(BTN_PLAY) == LOW)
  {
    if (a2dp_sink.get_audio_state() == ESP_A2D_AUDIO_STATE_STARTED)
    {
      a2dp_sink.pause();
    }
    else
    {
      a2dp_sink.play();
    }

    delay(300);
  }
}
