/*
 * This file is part of the pegoste2023 project, https://github.com/gluonsandquarks/pegoste2023
 * firmware.ino - main entry point describing the functionality of the board.
 * This file contains the main logic of the board.
 * 
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 gluons.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <avr/sleep.h>            // Power saving library

/* 
 * Pins PB1 and PB0 on the ATTiny1616 are PWM, so analogWrite() should work on them
 * More about how the megaTinyCore lib maps the arduino pins: https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/ATtiny_x16.md
 * More about the schematic/pinout of the tiny-osean board: https://github.com/gluonsandquarks/tiny-osean/blob/main/imgs/schematic.PNG
 */
#define STAR   PIN_PB0     // PB1 -> STAR
#define LEYE   PIN_PB1     // PB0 -> LEFT EYE
#define REYE   PIN_PA4     // PA4 -> RIGHT EYE
#define TONGUE PIN_PA5     // PA5 -> TONGUE
#define BUTTON PIN_PC2     // PC2

enum State
{
  ON = 0,
  BREATH_LO,
  ALT_LO_CLK,
  ALT_HI_CLK,
  ALT_LO_ANTICLK,
  ALT_HI_ANTICLK,
  RAND_DELAY,
  SOS,
  TONGUE_FN,
  STAR_FN,
  PULSE,
  SMOOTH,
  SLEEP,
  MAX_STATE
};

/* Global variables */
State stat = ON;

/* Forward declarations */
void on();
void breathing(int ms);
void alternating(int ms);
void rand_delay();
void sos();
void smooth();


/* Interrupt Service Routines (ISRs) */
void on_press();
void go_to_sleep();


void on()
{
  analogWrite(LEYE, 230);
  analogWrite(STAR, 200);
  analogWrite(TONGUE, 200);
  analogWrite(REYE, 230);
}

/* Function definitions for the different light shows */

/*
 * @brief Light show that smoothly dims and lights the LEDs through PWM.
 * 
 * @param ms Delay in milliseconds
 * @return Void
 */
void breathing(int ms)
{
  analogWrite(LEYE,   230);
  analogWrite(REYE,   230);
  analogWrite(TONGUE, 240);
  int limit = 200;
  State temp = stat;
  for (int i = 0; i <= limit; i++)
  {
    if (temp != stat) { break; }
    analogWrite(STAR, 200 - i);
    delay(ms);
  }
  for (int i = 200 - limit; i < 200 + 1; i++)
  {
    if (temp != stat) { break; }
    analogWrite(STAR, i);
    delay(ms);
  }

}

/*
 * @brief Light show that alternates both LEDs on/off.
 * 
 * @param ms Delay in milliseconds
 * @return Void
 */
void alternating(int ms)
{
  State temp = stat;
  analogWrite(LEYE, 255);
  analogWrite(STAR, 150);
  analogWrite(TONGUE, 255);
  analogWrite(REYE, 255);
  if (temp != stat) { return; }
  delay(ms);
  analogWrite(LEYE, 255);
  analogWrite(STAR, 255);
  analogWrite(TONGUE, 255);
  analogWrite(REYE, 150);
  if (temp != stat) { return; }
  delay(ms);
  analogWrite(LEYE, 255);
  analogWrite(STAR, 255);
  analogWrite(TONGUE, 150);
  analogWrite(REYE, 255);
  if (temp != stat) { return; }
  delay(ms);
  analogWrite(LEYE, 150);
  analogWrite(STAR, 255);
  analogWrite(TONGUE, 255);
  analogWrite(REYE, 255);
  if (temp != stat) { return; }
  delay(ms);
}

/*
 * @brief Light show that alternates both LEDs on/off.
 * 
 * @param ms Delay in milliseconds
 * @return Void
 */
void alternating_anti(int ms)
{
  State temp = stat;
  analogWrite(LEYE, 255);
  analogWrite(STAR, 150);
  analogWrite(TONGUE, 255);
  analogWrite(REYE, 255);
  if (temp != stat) { return; }
  delay(ms);
  analogWrite(LEYE, 150);
  analogWrite(STAR, 255);
  analogWrite(TONGUE, 255);
  analogWrite(REYE, 255);
  if (temp != stat) { return; }
  delay(ms);
  analogWrite(LEYE, 255);
  analogWrite(STAR, 255);
  analogWrite(TONGUE, 150);
  analogWrite(REYE, 255);
  if (temp != stat) { return; }
  delay(ms);
  analogWrite(LEYE, 255);
  analogWrite(STAR, 255);
  analogWrite(TONGUE, 255);
  analogWrite(REYE, 150);
  if (temp != stat) { return; }
  delay(ms);
}

/*
 * @brief Light show that lights each LED at random intervals.
 * 
 * @return Void
 */
void rand_delay()
{
  digitalWrite(LEYE, HIGH);
  digitalWrite(STAR, HIGH);
  digitalWrite(TONGUE, HIGH);
  digitalWrite(REYE, HIGH);
  unsigned long timer_1 = 570;
  unsigned long last_timer_1 = 0;
  unsigned long timer_2 = 350;
  unsigned long last_timer_2 = 0;
  int pinzR[] = {LEYE, TONGUE};
  int pinzL[] = {STAR, REYE};
  int selection = 0;
  State temp = stat;
  while (temp == stat)
  {
    selection = random(2);
    if (millis() - last_timer_1 >= timer_1)
    {
      float multiplier = random(4.0F);
      for (float i = 0; i < 256 * multiplier; i++)
      {
        if (temp != stat) { break ;}
        analogWrite(pinzR[selection], (i / multiplier));
      }
      last_timer_1 = millis() * random(500);
    }
    selection = random(2);
    if (millis() - last_timer_2 >= timer_2)
    {
      float multiplier = random(6.0F);
      for (float i = 0; i < 256 * multiplier; i++)
      {
        if (temp != stat) { break; }
        analogWrite(pinzL[selection], (i / multiplier));
      }
      last_timer_2 = millis() * random(500);
    }
  }
}

/*
 * @brief Light show that emits SOS in morse code.
 * 
 * @param ms Delay in milliseconds
 * @return Void
 */
void sos(int ms)
{
  State temp = stat;
  unsigned long timer = 400;
  unsigned long last_timer = 0;
  for (int i = 0; i < 3; i++)
  {
    if (temp != stat) { break; }
    analogWrite(LEYE, 180);
    analogWrite(STAR, 180);
    analogWrite(TONGUE, 180);
    analogWrite(REYE, 180);
    delay(ms);
    analogWrite(LEYE, 255);
    analogWrite(STAR, 255);
    analogWrite(TONGUE, 255);
    analogWrite(REYE, 255);
    delay(ms);
  }

  for (int i = 0; i < 3; i++)
  {
    if (temp != stat) { break; }
    analogWrite(LEYE, 180);
    analogWrite(STAR, 180);
    analogWrite(TONGUE, 180);
    analogWrite(REYE, 180);
    delay(ms * 2);
    analogWrite(LEYE, 255);
    analogWrite(STAR, 255);
    analogWrite(TONGUE, 255);
    analogWrite(REYE, 255);
    delay(ms * 2);
  }

  for (int i = 0; i < 3; i++)
  {
    if (temp != stat) { break; }
    analogWrite(LEYE, 180);
    analogWrite(STAR, 180);
    analogWrite(TONGUE, 180);
    analogWrite(REYE, 180);
    delay(ms);
    analogWrite(LEYE, 255);
    analogWrite(STAR, 255);
    analogWrite(TONGUE, 255);
    analogWrite(REYE, 255);
    delay(ms);
  }
  last_timer = millis();
  while (temp == stat)
  {
    if (millis() - last_timer >= timer)
    {
      last_timer = millis();
      break;
    }
  }
}

void tongue(int ms)
{
  State temp = stat;
  analogWrite(LEYE, 220);
  analogWrite(REYE, 220);
  analogWrite(STAR, 230);
  analogWrite(TONGUE, 230);
  delay(ms/2);
  if (temp != stat) { return; }
  analogWrite(TONGUE, 230);
  delay(ms/2);
  if (temp != stat) { return; }
  analogWrite(TONGUE, 255);
  delay(ms/2);
  if (temp != stat) { return; }
  analogWrite(TONGUE, 255);
  delay(ms/2);
}

void star(float multiplier)
{
  State temp = stat;
  analogWrite(LEYE, 220);
  analogWrite(REYE, 220);
  analogWrite(TONGUE, 230);
  for (float i = 0; i < 256 * multiplier; i++)
  {
    if (temp != stat) { break; }
    analogWrite(STAR, (i / multiplier));
  }
}

void pulse(float multiplier)
{
  State temp = stat;
  for (float i = 200; i < 256 * multiplier; i++)
  {
    if (temp != stat) { break; }
    analogWrite(STAR,   (i / multiplier));
    analogWrite(TONGUE, (i / multiplier));
    analogWrite(REYE,   (i / multiplier));
    analogWrite(LEYE,   (i / multiplier));
  }
}

/*
 * @brief Light show that transitions smoothly between both LEDs on board.
 * 
 * @param ms Delay in milliseconds
 * @return Void
 */
void smooth(int ms)
{
  digitalWrite(LEYE, HIGH);
  digitalWrite(STAR, HIGH);
  digitalWrite(TONGUE, HIGH);
  digitalWrite(REYE, HIGH);
  State temp = stat;
  int i = 0;

  for (i; i < 70; i++)
  {
    if (temp != stat) { break; }
    if (i > 32) { i += 30; }
    analogWrite(LEYE, 255 - i);
    analogWrite(REYE, 255 - i);
    analogWrite(TONGUE, 255 - i);
    delay(ms);
  }
  //i = 70;
  for (i; i > 0; i--)
  {
    if (temp != stat) { break; }
    if (i > 32) { i -= 30; }
    analogWrite(LEYE, 255 - i);
    analogWrite(REYE, 255 - i);
    analogWrite(TONGUE, 255 - i);
    delay(ms);
  }
  i = 0;
  analogWrite(LEYE, 255 - i);
  analogWrite(REYE, 255 - i);
  analogWrite(TONGUE, 255 - i);
  for (i; i < 256; i++)
  {
    if (temp != stat) { break; }
    if (i > 32) { i += 30; }
    analogWrite(STAR, 255 - i);
    delay(ms);
  }

  i = 255;
  for (i; i > 0; i--)
  {
    if (temp != stat) { break; }
    if (i > 32) { i -= 30; }
    analogWrite(STAR, 255 - i);

    delay(ms);
  }
  i = 0;
  analogWrite(STAR, 255 - i);

}

/*
 * @brief ISR called when a button is pressed.
 * 
 * @return Void
 */
void on_press()
{
  stat = State(stat + 1);
  if (stat >= MAX_STATE) { stat = ON; }
}

/*
 * @brief Last mode of the device. Shuts down LEDs and goes to sleep.
 * 
 * @return Void
 */
void go_to_sleep()
{
  digitalWrite(LEYE, HIGH);
  digitalWrite(STAR, HIGH);
  digitalWrite(TONGUE, HIGH);
  digitalWrite(REYE, HIGH);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  // TODO: research these parameters for the ATTiny1616. For some reason
  // these guys aren't defined for this MCU. Look into the avr/sleep.h implementation.
  //  byte adcsra = ADCSRA;                             // save ADCSRA
  //  ADCSRA &= ~_BV(ADEN);                             // disable ADC
  cli();                                                 // stop interrupts to ensure the BOD timed sequence executes as required
  //  byte mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);     //turn off the brown-out detector
  //  byte mcucr2 = mcucr1 & ~_BV(BODSE);
  //  MCUCR = mcucr1;
  //  MCUCR = mcucr2;
  sei();                                                // ensure interrupts enabled so we can wake up again
  sleep_cpu();                                          // go to sleep
  sleep_disable();                                      // wake up here
  //  ADCSRA = adcsra;                                  // restore ADCSRA
}

void setup()
{
  /* Initialize peripherals */
  pinMode(LEYE, OUTPUT);
  pinMode(STAR, OUTPUT);
  pinMode(TONGUE, OUTPUT);
  pinMode(REYE, OUTPUT);
  pinMode(BUTTON, INPUT);
  /* Disable floating pins */
  pinMode(PIN_PA1, INPUT_PULLUP);
  pinMode(PIN_PA2, INPUT_PULLUP);
  pinMode(PIN_PA3, INPUT_PULLUP);
  pinMode(PIN_PA6, INPUT_PULLUP);
  pinMode(PIN_PA7, INPUT_PULLUP);
  pinMode(PIN_PB2, INPUT_PULLUP);
  pinMode(PIN_PB3, INPUT_PULLUP);
  pinMode(PIN_PB4, INPUT_PULLUP);
  pinMode(PIN_PB5, INPUT_PULLUP);
  pinMode(PIN_PC0, INPUT_PULLUP);
  pinMode(PIN_PC1, INPUT_PULLUP);
  pinMode(PIN_PC3, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(BUTTON), on_press, RISING);
}

void loop()
{
  switch(stat)
  {
    case ON:
      on();
      break;
    case BREATH_LO:
      breathing(2);
      break;
    case ALT_LO_CLK:
      alternating(100);
      break;
    case ALT_HI_CLK:
      alternating(70);
      break;
    case ALT_LO_ANTICLK:
      alternating_anti(100);
      break;
    case ALT_HI_ANTICLK:
      alternating_anti(70);
      break;
    case RAND_DELAY:
      rand_delay();
      break;
    case SOS:
      sos(100);
      break;
    case TONGUE_FN:
      tongue(500);
      break;
    case STAR_FN:
      star(3.0F);
      break;
    case PULSE:
      pulse(4.0F);
      break;
    case SMOOTH:
      smooth(50);
      break;
    case SLEEP:
      go_to_sleep();
      break;
    default:
      break;
  }
}
