#include <Arduino.h>
#include <TM1637TinyDisplay.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>

#define COUNT_OF(x) (sizeof((x)) / sizeof((0[x])))

#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

static const int CLK = 16;
static const int DIO = 17;

static const int VOLTAGE_PIN = 34;
static const int GREEN_PIN = 21;
static const int YELLOW_PIN = 22;
static const int RED_PIN = 18;
static const int RELAY_PIN = 32;

static TM1637TinyDisplay display(CLK, DIO);
static bool show = false;
static float adcSlopeCorrection = 0.0f;
static float adcInterceptCorrection = 0.0f;

float correctedAdcVoltage(float adcReading_v);

void IRAM_ATTR buttonInterrupt() {
  show = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(VOLTAGE_PIN, INPUT);

  pinMode(GREEN_PIN, OUTPUT);
  digitalWrite(GREEN_PIN, LOW);
  pinMode(YELLOW_PIN, OUTPUT);
  digitalWrite(YELLOW_PIN, LOW);
  pinMode(RED_PIN, OUTPUT);
  digitalWrite(RED_PIN, LOW);

  // The boot button is connected to GPIO0
  pinMode(0, INPUT);
  attachInterrupt(0, buttonInterrupt, FALLING);

  // Calculate a line of best fit from ADC reports and ones from my multimeter
  constexpr float multimeterReadings_v[] = {11.97, 11.98, 12.43, 12.47, 12.49, 12.67, 12.76, 12.8, 12.88};
  constexpr float adcReadings_v[] = {11.59, 11.6, 12.12, 12.12, 12.16, 12.35, 12.44, 12.5, 12.6};
  float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
  const int n = COUNT_OF(multimeterReadings_v);
  for (int i = 0; i < n; i++) {
    sum_x += multimeterReadings_v[i];
    sum_y += adcReadings_v[i];
    sum_xy += multimeterReadings_v[i] * adcReadings_v[i];
    sum_x2 += multimeterReadings_v[i] * multimeterReadings_v[i];
  }
  static_assert(COUNT_OF(multimeterReadings_v) == COUNT_OF(adcReadings_v));
  adcSlopeCorrection = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
  adcInterceptCorrection = (sum_y - adcSlopeCorrection * sum_x) / n;
  Serial.printf("true V = %f * adc V + %f\n", adcSlopeCorrection, adcInterceptCorrection);
  for (int i = 0; i < COUNT_OF(multimeterReadings_v); ++i) {
    const float corrected = correctedAdcVoltage(adcReadings_v[i]);
    Serial.printf(
      "true V:%f adc V:%f corrected:%f diff:%f\n",
      multimeterReadings_v[i],
      adcReadings_v[i],
      corrected,
      multimeterReadings_v[i] - corrected
    );
  }

  setCpuFrequencyMhz(80);

  // Shut off peripherals
#define CHECK(func) if (func != ESP_OK) { Serial.println(#func " failed"); }
  CHECK(esp_wifi_stop());
  CHECK(esp_bluedroid_deinit());
  CHECK(esp_bluedroid_disable());
  CHECK(esp_bt_controller_disable());
  CHECK(esp_bt_controller_deinit());

  Serial.println("setup done");
}

void loop() {
  const float R1 = 20000.0f;
  const float R2 = 5100.0f;
  const float referenceVoltage = 3.3f;
  const float maxReading = 4095;

  // According to https://shopsolarkits.com/blogs/learning-center/marine-battery-voltage-chart,
  // 12.2V is 50% capacity, 12.0V is 25%
  constexpr float warningThreshold_v = 12.2f;
  constexpr float offThreshold_v = 12.0f;
  static_assert(warningThreshold_v > offThreshold_v);

  // We don't want to keep switching the relay, so only toggle it if it's been
  // on above or below a threshold for this long
  const decltype(millis()) relayToggleDelay_ms = 10 * 60 * 1000;
  bool previouslyAboveThreshold = true;
  auto thresholdConsistentTime_ms = millis();

  decltype(millis()) next_ms = 5000;

  while (1) {
    const int adcReading = analogRead(VOLTAGE_PIN);
    const float rawAdcVoltage = static_cast<float>(adcReading) / maxReading * referenceVoltage;
    const float adcVoltage = rawAdcVoltage * (R1 + R2) / R2;
    const float voltage = correctedAdcVoltage(adcVoltage);

    const bool aboveThreshold = voltage > offThreshold_v;
    if (previouslyAboveThreshold != aboveThreshold) {
      thresholdConsistentTime_ms = millis();
    } else if (millis() > thresholdConsistentTime_ms + relayToggleDelay_ms) {
      if (aboveThreshold) {
        digitalWrite(RELAY_PIN, LOW);
      } else {
        digitalWrite(RELAY_PIN, HIGH);
      }
    }

    if (millis() > next_ms || show) {
      // Only update the LEDs occasionally, to avoid flickering
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(RED_PIN, LOW);
      if (voltage > warningThreshold_v) {
        digitalWrite(GREEN_PIN, HIGH);
      } else if (voltage > offThreshold_v) {
        digitalWrite(YELLOW_PIN, HIGH);
      } else {
        digitalWrite(RED_PIN, HIGH);
      }

      Serial.printf(
        "Voltage:%0.2f (adc:%d rawAdcV:%0.2f, adcV:%0.2f)\n",
        voltage,
        adcReading,
        rawAdcVoltage,
        adcVoltage);
      next_ms = millis() + 5000;
    }

    if (show) {
      char buffer[6];
      snprintf(buffer, 6, "%05.2f", voltage);
      const int voltageTens = buffer[0] - '0';
      const int voltageOnes = buffer[1] - '0';
      const int voltageTenths = buffer[3] - '0';
      const int voltageHundredths = buffer[4] - '0';

      blinkNumber(voltageTens);
      blinkNumber(voltageOnes);
      blinkNumber(voltageTenths);
      blinkNumber(voltageHundredths);

      show = false;
    }
  }
}

void blinkNumber(const int count) {
  if (count < 0 || count > 9) {
    for (int i = 0; i < 10; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
    }
  } else if (count == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    for (int i = 0; i < count; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }
  delay(500);
}

float correctedAdcVoltage(const float adcReading_v) {
  return adcReading_v * adcSlopeCorrection + adcInterceptCorrection;
}
