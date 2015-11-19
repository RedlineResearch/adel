
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
  if (counter > 1000 && counter < 1100) {
    counter++;
    printf("%9d: read %d from pin %d\n", millis(), HIGH, pin);
    return HIGH;
  }
  
  printf("%9d: read %d from pin %d\n", millis(), LOW, pin);
  return LOW;
}

/** digitalWrite
 */
void digitalWrite(int pin, int value)
{
  printf("%9d: write %d to pin %d\n", millis(), value, pin);
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

/** button
 *  Wait for a button press
 *  NOTE: The semantics of this kind of coroutine doesn't seem quite right
 *        to me. The problem is that we need to know when we're expecting 
 *        a button push; but that's not how buttons work. Need to fix this.
 */
async button()
{
  abegin;
  while (1) {
    if (digitalRead(6) == HIGH) {
      adelay(50);
      if (digitalRead(6) == HIGH) {
	while (digitalRead(6) != LOW) {
	  adelay(20);
	}
	areturn(99);
      }
    }
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
    digitalWrite(11, HIGH);
    adelay(300);
    digitalWrite(11, LOW);
    adelay(300);
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
  seq(blink300);
  seq(blink500);
  together(blink300, blink500);
  afinish;
}

int main(int argc, char * argv[])
{
  while (test().waiting()) ;
}
