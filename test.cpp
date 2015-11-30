
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "adel.h"

using namespace std;

time_t base_time = 0;

#define HIGH 255
#define LOW 0

/** millis()
 *  A simulation of the Arduino library function millis(). It uses
 *  gettimeofday to get the time and return it in milliseconds. The
 *  first time it is called it adjusts the relative time to 0.
 */
int millis()
{
  struct timeval curtime;
  if (gettimeofday(&curtime, 0) == 0) {
    // -- Compute milliseconds relative to a base time
    time_t adjusted = curtime.tv_sec - base_time;
    time_t partial_millis = ((time_t) curtime.tv_usec) / 1000;
    time_t result = (adjusted * 1000) + partial_millis;

    // -- When base_time is zero it means we're calling millis() for
    //    the first time, so record this time as the base time.
    if (base_time == 0) {
      base_time = curtime.tv_sec;
      return 0;
    } else
      return (int) result;
  }
  
  return 0; // Error
}

/** digitalRead
 */
int digitalRead(int pin)
{
  static int counter = 0;
  counter++;
  if (counter > 1500 && counter < 1700) {
    // printf("%9d: read %d from pin %d\n", millis(), HIGH, pin);
    return HIGH;
  }
  
  // printf("%9d: read %d from pin %d\n", millis(), LOW, pin);
  return LOW;
}

/** digitalWrite
 */
void digitalWrite(int pin, int value)
{
  printf("%9d: write %d to pin %d\n", millis(), value, pin);
}

/** Accelerometer
 * Fake numbers for the accelerometer
 */
void read_accel(int & x, int & y, int & z)
{
  x = 0;
  y = 0;
  z = 0;
}

/** blink500
 *  Blink an LED every 500ms for 10 cycles
 */
async blink500()
{
  abegin;
  static int i = 0;
  while (i < 10) {
    digitalWrite(7, HIGH);
    adelay(500);
    digitalWrite(7, LOW);
    adelay(500);
    i++;
  }
  i = 0;
  afinish;
}

/** blink500
 *  Blink an LED every 300ms for 10 cycles
 */
async blink300()
{
  abegin;
  static int i = 0;
  while (i < 10) {
    digitalWrite(11, HIGH);
    adelay(300);
    digitalWrite(11, LOW);
    adelay(300);
    i++;
  }
  i = 0;
  afinish;
}  


/** Examples
 *
 *  (1) When the user pushes a button, light an LED for 5 seconds
 *  (2) While the user holds a button, light an LED
 *  (3) Blink an LED until the user presses a button
 *
 * Introduce a signal idea?
 * A variable that can change asynchronously? Then we introduce race conditions
 * 

Viz Timer

 Program mode:
   blink cursor
   when user presses the button add a 15s
   while user holds the button add 1m
   render (is render a separate thing?)
 */

/** button
 *  Check for a button press
 */

#define PRESS 1
#define RELEASE 2
#define HOLD 3

async button()
{
  abegin;
  
  marker(start);
  if (digitalRead(6) == HIGH) {
    adelay(50);
    if (digitalRead(6) == HIGH) {
      while (digitalRead(6) != LOW) {
	adelay(20);
	if ((since(start) >= 500) &&
	    (since(start) % 1000)) {
	  areturn(HOLD);
	}
      }
      
      if (since(start) < 500)
	areturn(PRESS);
      
      if (since(start) >= 500)
	areturn(RELEASE);
    }
  }
  
  afinish;
}

async cursor()
{
  abegin;
  
  adelay(400);
  areturn(1);
  adelay(400);
  areturn(0);

  afinish;
}

async orient_up()
{
  abegin;
  
  static int x = 0;
  static int y = 0;
  static int z = 0;
  do {
    adelay(20);
    read_accel(x, y, z);
  } until (x == 0 && y > 200 && z == 0);

  afinish;
}

async is_orient_down()
{
  abegin;
  
  static int x = 0;
  static int y = 0;
  static int z = 0;
  do {
    adelay(20);
    read_accel(x, y, z);
  } until (x == 0 && y < -200 && z == 0);

  areturn(1);
  
  afinish;
}


void render(int total_time, bool show_cursor)
{
  // FastLED stuff here
}

void finale()
{
}

async program()
{
  static int total_time = 0;
  static bool show_cursor = false;
  static bool done = false;

  abegin;
  
  // -- Wait for orientation to be up
  orient_up();

  // -- Main programming mode loop
  do {
    // -- Async check for button: kind of a combination of "together" and "if"
    when (button, PRESS) {
      total_time += 15000;
      render(total_time, show_cursor);
    }
    
    when (button, HOLD) {
      total_time += 60000;
      render(total_time, show_cursor);
    }
  
    when (cursor, 1) {
      render(total_time, show_cursor);
    }

    when (is_orient_down, 1) {
      done = true;
    }
  } until (done);

  areturn(total_time);
  afinish;
}

async viztimer()
{
  static int total_time = 0;
  abegin;
  while (1) {
    seqf(program, total_time);
    // countdown(total_time);
    // finale();
  }
  afinish;
}

/** button test
 *
 */
async button_test()
{
  abegin;
  static int v = 0;
  when (button, v) {
    printf("Got %d\n", v);
    seq(blink300);
  }
  
  afinish;
}

/** test
 *  Call blink300 and blink 500 in two different ways:
 *  - Sequential (do all the 300ms blinks, then all the 500ms blinks)
 *  - Together (do them concurrently)
 *  Note that the "together" construct does not move on to the next 
 *  statement until *both* routines are done.
 */
async test()
{
  abegin;
  together(button_test, blink500);
  /*
  seq(blink300);
  seq(blink500);
  together(blink300, blink500);
  */
  afinish;
}

int main(int argc, char * argv[])
{
  while (test().waiting()) ;
}
