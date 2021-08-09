#define DEBUG 0

// RoxLed Modes
#define ROX_DEFAULT 0
#define ROX_BLINK 1
#define ROX_PULSE 2
#include <RoxFlags.h>

#define N_MUX_POTS 16
#define N_POT_MUXES ((N_MUX_POTS + 15) / 16)
#define N_LOOSE_POTS 2
#define N_POTS (N_MUX_POTS + N_LOOSE_POTS)
#include <RoxPot.h>
RoxPot pots[N_POTS];

#define PIN_CH1 2
#define PIN_CH2 3
#define PIN_CH3 4
#define PIN_CH4 5
#define PIN_ANALOG_READ A0
#define N_MUXES N_POT_MUXES
#include <Rox74HC40XX.h>
Rox74HC4067<N_MUXES> muxes;

#define PIN_FREE_ANALOG_READ A1

#define N_ENCODERS 3
#include <RoxEncoder.h>
RoxEncoder encoders[N_ENCODERS];
byte encoderIndexes[N_ENCODERS] = {8, 28, 54};

#define N_BUTTONS 58
#include <RoxButton.h>
RoxButton buttons[N_BUTTONS];

#define PIN_IN_REG_DATA 6   // 9
#define PIN_IN_REG_LOAD 7   // 1
#define PIN_IN_REG_CLOCK 8  // 2
#define N_IN_SHIFT_REGS ((N_BUTTONS + N_ENCODERS * 2 + 7) / 8)
#include <Rox74HC165.h>
Rox74HC165<N_IN_SHIFT_REGS> inputShiftRegs;

#define N_SIMPLE_LEDS 40
#define N_SIMPLE_LEDS_SHIFT_REGS ((N_SIMPLE_LEDS + 7) / 8)

#define RIGHT_VU_REG_NUMBER 3
#define LEFT_VU_REG_NUMBER 4
#define N_VU_METERS 2
#define N_LEDS_PER_METER 8
#define N_VUS_SHIFT_REGS ((N_VU_METERS * N_LEDS_PER_METER + 7) / 8)
#include <RoxLedMeter.h>
RoxLedMeter<N_LEDS_PER_METER, 0, 0, 127> rightVU;
RoxLedMeter<N_LEDS_PER_METER, 0, 0, 127> leftVU;

#define PIN_OUT_REG_DATA 9    // 14
#define PIN_OUT_REG_LATCH 10  // 12
#define PIN_OUT_REG_CLOCK 11  // 11
#define PIN_OUT_REG_PWM -1
#define N_OUT_SHIFT_REGS (N_SIMPLE_LEDS_SHIFT_REGS + N_VUS_SHIFT_REGS)
#include <Rox74HC595.h>
Rox74HC595<N_OUT_SHIFT_REGS> outputShiftRegs;

void setupComponents() {
  muxes.begin(PIN_CH1, PIN_CH2, PIN_CH3, PIN_CH4);
  muxes.setSignalPin(0, PIN_ANALOG_READ);

  for (byte i = 0; i < N_MUX_POTS; i++) {
    pots[i].begin();
  }

  inputShiftRegs.begin(PIN_IN_REG_DATA, PIN_IN_REG_LOAD, PIN_IN_REG_CLOCK);

  for (byte i = 0; i < N_ENCODERS; i++) {
    encoders[i].begin();
  }

  for (byte i = 0; i < N_BUTTONS; i++) {
    buttons[i].begin();
  }

  outputShiftRegs.begin(PIN_OUT_REG_DATA, PIN_OUT_REG_LATCH, PIN_OUT_REG_CLOCK,
                        PIN_OUT_REG_PWM);
  outputShiftRegs.setBrightness(255);

  leftVU.onUpdate([](uint16_t n, bool state) {
    outputShiftRegs.writePin(LEFT_VU_REG_NUMBER * 8 + n, state);
    outputShiftRegs.update();
  });

  rightVU.onUpdate([](uint16_t n, bool state) {
    outputShiftRegs.writePin(RIGHT_VU_REG_NUMBER * 8 + n, state);
    outputShiftRegs.update();
  });
}

#include <MIDI.h>
#define MIDI_CHANNEL 1
MIDI_CREATE_DEFAULT_INSTANCE();

#define LEFT_VU 0
#define RIGHT_VU 1

// clang-format off
// 255: button does not have an LED
const byte ledMap[64] PROGMEM = {
    0,  255,    1,  255,  255,  255,    2,    3,
  255,  255,    8,   10,   11,   12,  255,  255,
   19,   18,   17,   16,   23,   22,   21,   20,
    9,  255,   55,  255,  255,  255,  255,  255,
   13,   14,  255,   15,   51,   50,  255,   49,
   40,   41,   42,   43,   44,   45,   46,   47,
  255,   52,   53,   54,   48,  255,  255,  255,
    4,    6,  255,  255,    7,    5,  255,  255,
};
// clang-format on

/* LED map
 * 00: B loop in
 * 01: B loop out
 * 02: B beat sync
 * 03: B key sync
 * 04: A key sync
 * 05: A loop out
 * 06: A beat sync
 * 07: A loop in
 * 08: B filter
 * 09: B hot cue
 * 10: B beat jump
 * 11: B beat loop
 * 12: B sampler
 * 13: B phones
 * 14: B cue
 * 15: B play
 * 16: B pad 2
 * 17: B pad 1
 * 18: B pad 3
 * 19: B pad 4
 * 20: B pad 5
 * 21: B pad 6
 * 22: B pad 7
 * 23: B pad 8
 * 24: B VU 1
 * 25: B VU 2
 * 26: B VU 3
 * 27: B VU 4
 * 28: B VU 5
 * 29: B VU 6
 * 30: B VU 7
 * 31: B VU 8
 * 32: A VU 1
 * 33: A VU 2
 * 34: A VU 3
 * 35: A VU 4
 * 36: A VU 5
 * 37: A VU 6
 * 38: A VU 7
 * 39: A VU 8
 * 40: A pad 2
 * 41: A pad 1
 * 42: A pad 3
 * 43: A pad 4
 * 44: A pad 5
 * 45: A pad 6
 * 46: A pad 7
 * 47: A pad 8
 * 48: A filter
 * 49: A play
 * 50: A cue
 * 51: A phones
 * 52: A hotcue
 * 53: A beat jump
 * 54: A beat loop
 * 55: A sampler
 * 56: -
 * 57: -
 * 58: -
 * 59: -
 * 60: -
 * 61: -
 * 62: -
 * 63: -
 */

void handleNote(byte channel, byte note, byte velocity) {
  if (note >= 64 || pgm_read_byte_near(&ledMap[note]) == 255) {
    return;
  }
  outputShiftRegs.writePin(pgm_read_byte_near(&ledMap[note]), velocity > 0);
  outputShiftRegs.update();
}

void handleControlChange(byte channel, byte number, byte value) {
  switch (number) {
    case LEFT_VU:
      leftVU.setValue(value);
      break;

    case RIGHT_VU:
      rightVU.setValue(value);
      break;

    default:
      break;
  }
}

void setupMIDI() {
  MIDI.turnThruOff();
  MIDI.setInputChannel(MIDI_CHANNEL);
  MIDI.setHandleNoteOn(handleNote);
  MIDI.setHandleNoteOff(handleNote);
  MIDI.setHandleControlChange(handleControlChange);
}

byte debugMeter = 0;
byte debugLedIndex = 0;
void readDigital() {
  inputShiftRegs.update();
  byte encoder = 0;
  byte button = 0;
  for (byte i = 0; i < N_ENCODERS * 2 + N_BUTTONS; i++) {
    if (encoderIndexes[encoder] == i) {
      if (encoders[encoder].update(inputShiftRegs.readPin(i),
                                   inputShiftRegs.readPin(i + 1))) {
        bool clockwise = encoders[encoder].clockwise();
        // Bad wiring lead to this encoder rotating the other way around
        if (i == 54) {
          clockwise = !clockwise;
        }
        if (DEBUG) {
          outputShiftRegs.writePin(debugLedIndex, 0);
          debugLedIndex = (debugLedIndex + clockwise * 2 + 63) % 64;
          outputShiftRegs.writePin(debugLedIndex, 1);
          outputShiftRegs.update();

          if (!clockwise && debugMeter < 16) {
            debugMeter = 16;
          }
          debugMeter = debugMeter + clockwise * 32 - 16;
          if (debugMeter > 128) {
            debugMeter = 128;
          }
          leftVU.setValue(debugMeter);
          rightVU.setValue(128 - debugMeter);
        } else {
          MIDI.sendControlChange(N_POTS + i, clockwise * 126 + 1, MIDI_CHANNEL);
        }
      }
      i++;
      encoder++;
    } else {
      if (buttons[button].update(inputShiftRegs.readPin(i))) {
        if (DEBUG) {
          if (pgm_read_byte_near(&ledMap[i]) == 255) {
            return;
          } else {
            outputShiftRegs.togglePin(pgm_read_byte_near(&ledMap[i]));
            outputShiftRegs.update();
          }
          Serial.println(i);
        } else {
          MIDI.sendNoteOn(i, 127 * buttons[button].pressed(), MIDI_CHANNEL);
        }
      }
      button++;
    }
  }
}

void readAnalog() {
  muxes.update();
  for (byte i = 0; i < N_MUX_POTS; i++) {
    if (pots[i].update(muxes.read(i))) {
      MIDI.sendControlChange(i, pots[i].read(), MIDI_CHANNEL);
    }
  }
  for (byte i = 0; i < N_LOOSE_POTS; i++) {
    if (pots[N_MUX_POTS + i].update(analogRead(PIN_FREE_ANALOG_READ + i))) {
      MIDI.sendControlChange(N_MUX_POTS + i, pots[N_MUX_POTS + i].read(),
                             MIDI_CHANNEL);
    }
  }
}

void setup() {
  setupComponents();
  setupMIDI();
  if (DEBUG) {
    Serial.begin(9600);
  } else {
    Serial.begin(31250);
  }
}

void loop() {
  while (MIDI.read()) {
  }
  readDigital();
  readAnalog();
}
