#include <TFT_eSPI.h>
#include <array>
#include "pico/util/queue.h"
#include "SerialComms.hpp"

#define FAST_MEM __not_in_flash("data")

constexpr size_t MULTIPLEX_S0 = 14;
constexpr size_t MULTIPLEX_S1 = 15;
constexpr size_t MULTIPLEX_S2 = 16;


TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
struct repeating_timer timerDisplay;

//sensors
std::array<size_t, 8> FAST_MEM sensorReadings, sensorReadingsCore1;
static queue_t FAST_MEM q_sensors;


inline bool __not_in_flash_func(displayUpdate)(__unused struct repeating_timer *t) {
  constexpr int barwidth = 16;
  constexpr int barheight=135;
  constexpr int barbase= 120 - (barheight/2.0);
  constexpr int barGap=18;
  constexpr int barcol = TFT_RED;
  constexpr int barcol2 = TFT_BLUE;
  constexpr int barLeft = 120 - (barGap*4);
  if (queue_try_remove(&q_sensors, sensorReadingsCore1.data())) {
    for(size_t i=0; i < 8; i++) {
      int thisBarHeight = barheight * (sensorReadingsCore1[i] / 4096.0);
      thisBarHeight = max(5, thisBarHeight);
      thisBarHeight = min(barheight, thisBarHeight);
      Serial.printf("%d %d %d\n", i, thisBarHeight, barheight - thisBarHeight);
      tft.fillRect(barLeft + (i * barGap), barbase, barwidth, barbase + thisBarHeight, barcol);
      tft.fillRect(barLeft + (i * barGap), barbase + thisBarHeight, barwidth, barheight - thisBarHeight, barcol2);
    }
  }
  return true;
}


void setup() {
  // put your setup code here, to run once:
  pinMode(22,1);
  pinMode(MULTIPLEX_S0, 1);
  pinMode(MULTIPLEX_S1, 1);
  pinMode(MULTIPLEX_S2, 1);
  digitalWrite(22,1);
  analogReadResolution(11);

  
  queue_init(&q_sensors, sizeof(float)*sensorReadings.size(), 1);

  for(auto &v: sensorReadings) {
    v=0;
  }
  Serial.begin();
  Serial2.setRX(9);
  Serial2.setTX(8);  
  Serial2.begin(115200);

  tft.begin();
  tft.setRotation(1);
  tft.setFreeFont(&FreeSans18pt7b);
  tft.fillScreen(TFT_GREEN);



}



void loop() {

  for(size_t sensorIdx=0; sensorIdx < 8; sensorIdx++) {
    
    //set the input using the multiplexer data lines
    digitalWrite(MULTIPLEX_S0, sensorIdx & 0b1);
    digitalWrite(MULTIPLEX_S1, (sensorIdx>>1) & 0b1);
    digitalWrite(MULTIPLEX_S2, (sensorIdx>>2) & 0b1);
    //;let the multiplexer settle
    delayMicroseconds(50);

    //make a reading
    auto ain = analogRead(26);
    sensorReadings[sensorIdx] = ain;
    queue_try_add(&q_sensors, sensorReadings.data());
    sendOverSerial(sensorIdx, ain / 2047.0);
    delay(1);
  }
  delay(30);
}

void setup1() {
  add_repeating_timer_ms(-39, displayUpdate, NULL, &timerDisplay);
}

void loop1() {
}
