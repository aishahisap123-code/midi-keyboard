#pragma once

#include <Arduino.h>

// ==========================================
// SYSTEM CONFIGURATION
// ==========================================
#define DEVICE_NAME "MIDI35 Controller"
#define FIRMWARE_VERSION "1.0.0"

// Task Periods (ms)
#define TOUCH_TASK_PERIOD_MS 5     // 200 Hz polling
#define ANALOG_TASK_PERIOD_MS 10   // 100 Hz polling
#define MIDI_TASK_PERIOD_MS 2      // 500 Hz processing

// ==========================================
// HARDWARE PIN MAPPINGS
// ==========================================

// Capacitive Touch Channels (ESP32-S3 native touch pads)
constexpr uint8_t TOUCH_KEY_PINS[7] = {T1, T2, T3, T4, T5, T6, T7};
constexpr uint8_t TOUCH_DRUM_PINS[4] = {T8, T9, T10, T11};

// Analog Slider Pins (ADC1)
constexpr uint8_t SLIDER_PINS[2] = {1, 2}; // GPIO 1 and 2

// ==========================================
// MIDI MAPPINGS & CONSTANTS
// ==========================================
constexpr uint8_t KEY_MIDI_NOTES[7] = {60, 62, 64, 65, 67, 69, 71}; // C4 Major Scale
constexpr uint8_t DRUM_MIDI_NOTES[4] = {36, 38, 42, 46};             // GM Percussion

constexpr uint8_t MIDI_CHANNEL_KEYS = 1;
constexpr uint8_t MIDI_CHANNEL_DRUMS = 10;

constexpr uint8_t SLIDER_CC_NUMBERS[2] = {1, 7}; // CC 1 (Modulation), CC 7 (Volume)

// ==========================================
// DSP & FILTER PARAMETERS
// ==========================================
#define CALIBRATION_SAMPLES 64
#define TOUCH_PRESS_HYSTERESIS_RATIO 0.25f // 25% drop from baseline triggers press
#define TOUCH_RELEASE_HYSTERESIS_RATIO 0.15f // 15% drop from baseline triggers release

#define ANALOG_EMA_ALPHA 0.2f               // Exponential Moving Average filter coefficient
#define ANALOG_DEADZONE 1                   // Ignore 1-bit jitter

#pragma once

#include "MIDI35_Config.h"

struct TouchSensor {
    uint8_t pin;
    uint8_t midiNote;
    uint8_t midiChannel;
    uint32_t baseline;
    uint32_t currentRead;
    bool isPressed;
    
    uint32_t pressThreshold;
    uint32_t releaseThreshold;
};

class TouchEngine {
public:
    TouchEngine();
    void begin();
    void calibrate();
    void update(void (*onNoteOn)(uint8_t note, uint8_t vel, uint8_t ch),
                void (*onNoteOff)(uint8_t note, uint8_t vel, uint8_t ch));

private:
    TouchSensor keys[7];
    TouchSensor drums[4];
    
    void processSensor(TouchSensor &sensor,
                       void (*onNoteOn)(uint8_t, uint8_t, uint8_t),
                       void (*onNoteOff)(uint8_t, uint8_t, uint8_t));
};

#include "TouchEngine.h"

TouchEngine::TouchEngine() {
    for (int i = 0; i < 7; i++) {
        keys[i] = {TOUCH_KEY_PINS[i], KEY_MIDI_NOTES[i], MIDI_CHANNEL_KEYS, 0, 0, false, 0, 0};
    }
    for (int i = 0; i < 4; i++) {
        drums[i] = {TOUCH_DRUM_PINS[i], DRUM_MIDI_NOTES[i], MIDI_CHANNEL_DRUMS, 0, 0, false, 0, 0};
    }
}

void TouchEngine::begin() {
    calibrate();
}

void TouchEngine::calibrate() {
    // Perform dynamic ambient baseline calibration
    for (int i = 0; i < 7; i++) {
        uint64_t sum = 0;
        for (int s = 0; s < CALIBRATION_SAMPLES; s++) {
            sum += touchRead(keys[i].pin);
            delayMicroseconds(500);
        }
        keys[i].baseline = sum / CALIBRATION_SAMPLES;
        keys[i].pressThreshold = keys[i].baseline * (1.0f - TOUCH_PRESS_HYSTERESIS_RATIO);
        keys[i].releaseThreshold = keys[i].baseline * (1.0f - TOUCH_RELEASE_HYSTERESIS_RATIO);
    }

    for (int i = 0; i < 4; i++) {
        uint64_t sum = 0;
        for (int s = 0; s < CALIBRATION_SAMPLES; s++) {
            sum += touchRead(drums[i].pin);
            delayMicroseconds(500);
        }
        drums[i].baseline = sum / CALIBRATION_SAMPLES;
        drums[i].pressThreshold = drums[i].baseline * (1.0f - TOUCH_PRESS_HYSTERESIS_RATIO);
        drums[i].releaseThreshold = drums[i].baseline * (1.0f - TOUCH_RELEASE_HYSTERESIS_RATIO);
    }
}

void TouchEngine::processSensor(TouchSensor &sensor,
                                void (*onNoteOn)(uint8_t, uint8_t, uint8_t),
                                void (*onNoteOff)(uint8_t, uint8_t, uint8_t)) {
    sensor.currentRead = touchRead(sensor.pin);

    // Hysteresis State Machine
    if (!sensor.isPressed && sensor.currentRead < sensor.pressThreshold) {
        sensor.isPressed = true;
        if (onNoteOn) onNoteOn(sensor.midiNote, 127, sensor.midiChannel);
    } 
    else if (sensor.isPressed && sensor.currentRead > sensor.releaseThreshold) {
        sensor.isPressed = false;
        if (onNoteOff) onNoteOff(sensor.midiNote, 0, sensor.midiChannel);
    }
}

void TouchEngine::update(void (*onNoteOn)(uint8_t, uint8_t, uint8_t),
                        void (*onNoteOff)(uint8_t, uint8_t, uint8_t)) {
    for (int i = 0; i < 7; i++) {
        processSensor(keys[i], onNoteOn, onNoteOff);
    }
    for (int i = 0; i < 4; i++) {
        processSensor(drums[i], onNoteOn, onNoteOff);
    }
}

#pragma once
#include "MIDI35_Config.h"

class AnalogEngine {
public:
    AnalogEngine() {
        for (int i = 0; i < 2; i++) {
            filteredVal[i] = 0.0f;
            lastMidiCC[i] = 255; // Force initial update
        }
    }

    void begin() {
        analogReadResolution(12); // ESP32 12-bit ADC (0-4095)
        for (int i = 0; i < 2; i++) {
            pinMode(SLIDER_PINS[i], INPUT);
            filteredVal[i] = analogRead(SLIDER_PINS[i]);
        }
    }

    void update(void (*onControlChange)(uint8_t cc, uint8_t val, uint8_t ch)) {
        for (int i = 0; i < 2; i++) {
            uint16_t raw = analogRead(SLIDER_PINS[i]);
            
            // Exponential Moving Average Filter
            filteredVal[i] = (ANALOG_EMA_ALPHA * raw) + ((1.0f - ANALOG_EMA_ALPHA) * filteredVal[i]);
            
            uint8_t midiVal = (uint8_t)map((long)filteredVal[i], 0, 4095, 0, 127);

            // Apply deadzone jitter filtering
            if (abs((int)midiVal - (int)lastMidiCC[i]) > ANALOG_DEADZONE) {
                lastMidiCC[i] = midiVal;
                if (onControlChange) {
                    onControlChange(SLIDER_CC_NUMBERS[i], midiVal, MIDI_CHANNEL_KEYS);
                }
            }
        }
    }

private:
    float filteredVal[2];
    uint8_t lastMidiCC[2];
};

#pragma once
#include "MIDI35_Config.h"

class AnalogEngine {
public:
    AnalogEngine() {
        for (int i = 0; i < 2; i++) {
            filteredVal[i] = 0.0f;
            lastMidiCC[i] = 255; // Force initial update
        }
    }

    void begin() {
        analogReadResolution(12); // ESP32 12-bit ADC (0-4095)
        for (int i = 0; i < 2; i++) {
            pinMode(SLIDER_PINS[i], INPUT);
            filteredVal[i] = analogRead(SLIDER_PINS[i]);
        }
    }

    void update(void (*onControlChange)(uint8_t cc, uint8_t val, uint8_t ch)) {
        for (int i = 0; i < 2; i++) {
            uint16_t raw = analogRead(SLIDER_PINS[i]);
            
            // Exponential Moving Average Filter
            filteredVal[i] = (ANALOG_EMA_ALPHA * raw) + ((1.0f - ANALOG_EMA_ALPHA) * filteredVal[i]);
            
            uint8_t midiVal = (uint8_t)map((long)filteredVal[i], 0, 4095, 0, 127);

            // Apply deadzone jitter filtering
            if (abs((int)midiVal - (int)lastMidiCC[i]) > ANALOG_DEADZONE) {
                lastMidiCC[i] = midiVal;
                if (onControlChange) {
                    onControlChange(SLIDER_CC_NUMBERS[i], midiVal, MIDI_CHANNEL_KEYS);
                }
            }
        }
    }

private:
    float filteredVal[2];
    uint8_t lastMidiCC[2];
};


#include "MIDI35_Config.h"
#include "TouchEngine.h"
#include "AnalogEngine.h"
#include "MidiManager.h"

// Hardware Engines
TouchEngine touchEngine;
AnalogEngine analogEngine;

// FreeRTOS Task Handles
TaskHandle_t TaskTouchHandle = NULL;
TaskHandle_t TaskAnalogHandle = NULL;

// Dispatch Callbacks
void handleNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
    MidiManager::sendNoteOn(note, velocity, channel);
}

void handleNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
    MidiManager::sendNoteOff(note, velocity, channel);
}

void handleControlChange(uint8_t cc, uint8_t value, uint8_t channel) {
    MidiManager::sendControlChange(cc, value, channel);
}

// Task 1: High-Priority Touch Scanning Core
void TaskTouchProcessing(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TOUCH_TASK_PERIOD_MS);

    for (;;) {
        touchEngine.update(handleNoteOn, handleNoteOff);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Task 2: Analog Filtering & CC Core
void TaskAnalogProcessing(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(ANALOG_TASK_PERIOD_MS);

    for (;;) {
        analogEngine.update(handleControlChange);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000); // Wait for CDC Serial initialization

    Serial.println("==========================================");
    Serial.printf("Initializing %s v%s\n", DEVICE_NAME, FIRMWARE_VERSION);
    Serial.println("==========================================");

    // Initialize Transports and Engines
    MidiManager::begin();
    touchEngine.begin();
    analogEngine.begin();

    // Create FreeRTOS Tasks bound to specific CPU Cores
    xTaskCreatePinnedToCore(
        TaskTouchProcessing,
        "TouchTask",
        4096,
        NULL,
        2,                  // Priority 2 (High)
        &TaskTouchHandle,
        1                   // Core 1
    );

    xTaskCreatePinnedToCore(
        TaskAnalogProcessing,
        "AnalogTask",
        3072,
        NULL,
        1,                  // Priority 1 (Medium)
        &TaskAnalogHandle,
        0                   // Core 0
    );

    Serial.println("System Initialization Complete. Running Tasks...");
}

void loop() {
    // Main loop remains empty as processing runs entirely in FreeRTOS Tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}