#include <stdint.h>
#include <stdio.h>
#include <LiquidCrystal.h>

#define BACKLIGHT 6

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//
// Clock presets
//
uint16_t presets[][3] = {
  30 * 60, 2 * 60, 30,
  20 * 60, 2 * 60, 30,
  15 * 60, 2 * 60, 30,
  10 * 60, 2 * 60, 30,
  30 * 60, 2 * 60, 45,
  20 * 60, 2 * 60, 45,
  15 * 60, 2 * 60, 45,
  10 * 60, 2 * 60, 45,
  0, 0, 0
};
int preset = -1;

//
// Clocks are in deciseconds
//
uint16_t score_a = 0;
uint16_t score_b = 0;
int16_t jam_duration;
int16_t lineup_duration;
int16_t period_clock;
int16_t jam_clock;
uint8_t backlight = 32;
enum {
  SETUP,
  JAM,
  LINEUP,
  TIMEOUT
} 
state = SETUP;

char *state_char[] = {
  "S", "J", "L", "T"
};

void next_preset() {
  preset += 1;
  if (presets[preset][0] == 0) {
    preset = 0;
  }
  period_clock = -presets[preset][0] * 10;
  jam_duration = -presets[preset][1] * 10;
  lineup_duration = -presets[preset][2] * 10;
}


void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(BACKLIGHT, OUTPUT);

  next_preset();
  analogWrite(BACKLIGHT, backlight);
}

void draw_clock(char *fmt, int16_t clock) {
  char time[10];

  clock = abs(clock) / 10;
  snprintf(time, 10, fmt, clock / 60, clock % 60);
  lcd.print(time);
}

void draw() {
  lcd.setCursor(1, 0);
  draw_clock("%02d:%02d", period_clock);
  lcd.setCursor(0, 1);
  lcd.print(state_char[state]);
  lcd.print(" ");
  draw_clock("%01d:%02d", (state == SETUP)?lineup_duration:jam_clock);  
}

int last_button = 0;

void read_buttons() {
  int this_button = digitalRead(0) + (digitalRead(1) << 1);

  // This sort of debounces things.
  if (this_button <= last_button) {
    last_button = this_button;
    return;
  }
  last_button = this_button;

  switch (this_button) {
  case 1:
    if (state == SETUP) {
      next_preset();
    } 
    else {
      state = LINEUP;
      jam_clock = lineup_duration;
    }
    break;
  case 2:
    if (state == SETUP) {
      lcd.clear();
    }
    state = JAM;
    jam_clock = jam_duration;
    break;
  case 3:
    if (state == SETUP) {
      backlight += 32;
      analogWrite(BACKLIGHT, backlight);
    } else {
      state = TIMEOUT;
      jam_clock = 1;
    }
    break;
  }
}

unsigned long then = 0;

void loop() {
  unsigned long now = millis() / 100;

  read_buttons();

  if (now > then) {
    int delta = now - then;

    switch (state) {
    case SETUP:
      break;
    case JAM:
    case LINEUP:
      if (period_clock) {
        period_clock += delta;
      }
      // fall through
    case TIMEOUT:
      if (jam_clock) {
        jam_clock += delta;
      }
      break;
    }
    then = now;
    draw();
  }
}



