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

// According to https://shopsolarkits.com/blogs/learning-center/marine-battery-voltage-chart,
// 12.2V is 50% capacity, 12.0V is 25%
constexpr float warningThreshold_v = 12.2f;
constexpr float offThreshold_v = 12.0f;

float adcReadingToVoltage(float rawAdc);
float voltageToUndividedVoltage(float voltage);
float correctedVoltage(float voltage);

void IRAM_ATTR buttonInterrupt() {
  show = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(VOLTAGE_PIN, INPUT);

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  // The boot button is connected to GPIO0
  pinMode(0, INPUT);
  attachInterrupt(0, buttonInterrupt, FALLING);

  // Calculate a line of best fit from ADC reports and ones from my multimeter
  constexpr float multimeterReadings_v[] = {11.98, 11.97, 12.47, 12.43, 12.80, 12.67, 12.88, 12.76, 12.49};
  constexpr float adcReadings_v[] =  {11.60, 11.59, 12.16, 12.12, 12.50, 12.35, 12.60, 12.44, 12.12};
  static_assert(COUNT_OF(multimeterReadings_v) == COUNT_OF(adcReadings_v));
  float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
  const int n = COUNT_OF(multimeterReadings_v);
  for (int i = 0; i < n; i++) {
    sum_x += adcReadings_v[i];
    sum_y += multimeterReadings_v[i];
    sum_xy += adcReadings_v[i] * multimeterReadings_v[i];
    sum_x2 += adcReadings_v[i] * adcReadings_v[i];
  }
  adcSlopeCorrection = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
  adcInterceptCorrection = (sum_y - adcSlopeCorrection * sum_x) / n;
  Serial.printf("true V = %f * adc V + %f\n", adcSlopeCorrection, adcInterceptCorrection);
  for (int i = 0; i < COUNT_OF(multimeterReadings_v); ++i) {
    const float corrected = correctedVoltage(adcReadings_v[i]);
    Serial.printf(
      "true V:%0.2f adc V:%0.2f corrected:%0.2f diff:%0.4f\n",
      multimeterReadings_v[i],
      adcReadings_v[i],
      corrected,
      multimeterReadings_v[i] - corrected
    );
  }

  // If the battery is good, then turn on the LEDs immediately
  const int adcReading = analogRead(VOLTAGE_PIN);
  const float adc_v = adcReadingToVoltage(adcReading);
  const float battery_v = voltageToUndividedVoltage(adc_v);
  const float correctedBattery_v = correctedVoltage(battery_v);
  if (correctedBattery_v > offThreshold_v) {
    // When wiring, hook the strips to the relay normally open, so that we need to take action to close it
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
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
  const int adcReadingCount = 10;

  static_assert(warningThreshold_v > offThreshold_v);

  // We don't want to keep switching the relay, and because there's voltage drop
  // under load, just shut it off if it drops below offThreshold_v and keep it
  // off until it goes back above warningThreshold_v
  const decltype(millis()) relayToggleDelay_ms = 3 * 60 * 1000;
  auto relayToggleTime_ms = millis();

  bool ledsOn;
  {
    const int adcReading = analogRead(VOLTAGE_PIN);
    const float adc_v = adcReadingToVoltage(adcReading);
    const float battery_v = voltageToUndividedVoltage(adc_v);
    const float correctedBattery_v = correctedVoltage(battery_v);
    ledsOn = (correctedBattery_v > offThreshold_v);
  }

  enum class LedColor {
    Green,
    Yellow,
    Red,
  };
  LedColor color = ledsOn ? LedColor::Green : LedColor::Red;

  decltype(millis()) next_ms = 0;

  while (1) {
    // Average a few readings
    float adcReading = 0.0f;
    for (int i = 0; i < adcReadingCount; ++i) {
      adcReading += analogRead(VOLTAGE_PIN);
    }
    adcReading /= adcReadingCount;
    const float adc_v = adcReadingToVoltage(adcReading);
    const float battery_v = voltageToUndividedVoltage(adc_v);
    const float correctedBattery_v = correctedVoltage(battery_v);

    // Only update the LEDs occasionally, to avoid flickering
    if (millis() > next_ms || show) {
      Serial.printf(
        "Corrected batV:%0.2f (adc:%0.2f adcV:%0.2f, batV:%0.2f)\n",
        correctedBattery_v,
        adcReading,
        adc_v,
        battery_v);

      next_ms = millis() + 5000;
    }

    if (show) {
      char buffer[6];
      snprintf(buffer, 6, "%05.2f", correctedBattery_v);
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

    if (correctedBattery_v > warningThreshold_v) {
      if (!ledsOn && millis() > relayToggleTime_ms + relayToggleDelay_ms) {
        Serial.printf("Turning on strips, V=%0.2f>%0.2f\n", correctedBattery_v, warningThreshold_v);
        digitalWrite(RELAY_PIN, HIGH);
        relayToggleTime_ms = millis();
        ledsOn = true;
        color = LedColor::Green;
      }
    } else if (correctedBattery_v > offThreshold_v) {
      if (millis() > relayToggleTime_ms + relayToggleDelay_ms) {
        color = LedColor::Yellow;
      }
    } else {
      if (ledsOn && millis() > relayToggleTime_ms + relayToggleDelay_ms) {
        Serial.printf("Turning off strips, V=%0.2f<%0.2f\n", correctedBattery_v, offThreshold_v);
        digitalWrite(RELAY_PIN, LOW);
        relayToggleTime_ms = millis();
        ledsOn = false;
        color = LedColor::Red;
      }
    }

    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
    if (ledsOn) {
      switch (color) {
        case LedColor::Green:
          digitalWrite(GREEN_PIN, HIGH);
          break;
        case LedColor::Yellow:
          digitalWrite(YELLOW_PIN, HIGH);
          break;
        case LedColor::Red:
          digitalWrite(RED_PIN, HIGH);
          break;
      }
    } else {
      // it
      if ((millis() >> 8) & 1) {
        switch (color) {
          case LedColor::Green:
            digitalWrite(GREEN_PIN, HIGH);
            break;
          case LedColor::Yellow:
            digitalWrite(YELLOW_PIN, HIGH);
            break;
          case LedColor::Red:
            digitalWrite(RED_PIN, HIGH);
            break;
        }
      }
    }
  }
}

void blinkNumber(const int count) {
  if (count < 0 || count > 9) {
    Serial.printf("Tried to blink %d\n", count);
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

float adcReadingToVoltage(const float adcReading) {
  const float maxReading = 4095.0f;
  const float referenceVoltage = 3.3f;
  return adcReading / maxReading * referenceVoltage;
}

float voltageToUndividedVoltage(const float voltage) {
  const float R1 = 20000.0f;
  const float R2 = 5100.0f;
  return voltage * (R1 + R2) / R2;
}

float correctedVoltage(const float voltage) {
  return voltage * adcSlopeCorrection + adcInterceptCorrection;
}
