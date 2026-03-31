#include <Arduino.h>
#include "AudioTools.h"
#include "BluetoothA2DP.h"

#define I2S_LRC 14
#define I2S_BCLK 27
#define I2S_DIN 26

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup()
{
  Serial.begin(115200);

  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = I2S_BCLK;
  cfg.pin_ws = I2S_LRC;
  cfg.pin_data = I2S_DIN;
  i2s.begin(cfg);

  a2dp_sink.start("esp_speaker");
  a2dp_sink.set_volume(50);
}

void loop()
{

}
