#include <Arduino.h>
#include "AudioTools.h"
#include "BluetoothA2DP.h"

#define I2S_LRC 14
#define I2S_BCLK 27
#define I2S_DIN 26
#define LED_PIN 13

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

volatile esp_a2d_connection_state_t current_state = ESP_A2D_CONNECTION_STATE_DISCONNECTED;

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr) {
  current_state = state;

  Serial.print("Bluetooth state changed: ");
  Serial.println(a2dp_sink.to_str(state));

  if(state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    Serial.println("Device paired");
    digitalWrite(LED_PIN, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting..");

  pinMode(LED_PIN, OUTPUT);

  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = I2S_BCLK;
  cfg.pin_ws = I2S_LRC;
  cfg.pin_data = I2S_DIN;
  i2s.begin(cfg);

  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  Serial.println("Starting A2DP Sink as 'esp_speaker'");
  a2dp_sink.start("esp_speaker");
  a2dp_sink.set_volume(50);
}

void loop()
{
  if (current_state != ESP_A2D_CONNECTION_STATE_CONNECTED) {
    digitalWrite(LED_PIN, (millis() / 500) % 2);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}
