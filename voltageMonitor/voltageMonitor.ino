#include <Arduino.h>
#include <TM1637TinyDisplay.h>

#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

static const int CLK = 16;
static const int DIO = 17;
static TM1637TinyDisplay display(CLK, DIO);

static bool show = false;
void IRAM_ATTR buttonInterrupt() {
  show = true;
}

void setup() {
  Serial.begin(115200);

  //pinMode(SWITCH_PIN, OUTPUT);
  //digitalWrite(SWITCH_PIN, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // The boot button is connected to GPIO0
  pinMode(0, INPUT);
  attachInterrupt(0, buttonInterrupt, FALLING);

  //setCpuFrequencyMhz(80);

  // Shut off peripherals
  // These are giving a compile error
  //esp_wifi_stop();
  //esp_bluedroid_disable();
  //esp_bluedroid_deinit();
  //esp_bt_controller_disable();
  //esp_bt_controller_deinit();

  Serial.println("setup done");
}

void loop() {
  if (show) {
    float voltage = 12.34;
    char array[6];
    snprintf(array, sizeof(array), "%0.2f", voltage);

    auto blinkChar = [array](char c) {
      int number = static_cast<int>(c - '0');
      blinkNumber(number);
    };
    blinkChar(array[0]);
    blinkChar(array[1]);
    blinkChar(array[3]);
    blinkChar(array[4]);

    show = false;
  }
}

void blinkNumber(const int count) {
  if (count == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    for (int i = 0; i < count; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
  delay(500);
}
