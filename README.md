# adel
A new way to program microcontrollers

Adel is a library that makes it easier to program microcontrollers, such as the Arduino. The main idea is to provide a simple kind of concurrency, similar to coroutines. Using Adel, any function can be made into an asynchronous function, which can be called concurrently with other Adel functions without interfering with them. The library is implemented entirely as a set of C/C++ macros, so it requires no new compiler tools or flags. Just download and install the Adel directory in your Arduino IDE libraries folder.

Adel came out of my particular frustration with microcontroller programming: seemingly simple behavior can be very hard to implement. As a simple example, imagine a function that blinks an LED attached to some pin every N milliseconds:

     void blink(int some_pin, int N) {
       while (1) {
         digitalWrite(some_pin, HIGH);
         delay(N);
         digitalWrite(some_pin, LOW);
         delay(N);
        }
      }

OK, that's easy enough. I can call it will two different values, say 500 and 300:

    blink(3, 500);
    blink(4, 300);

But what if I want to blink them **at the same time**? Now, suddenly, I have no composability. I have to write completely different, and much more complex code:

    uint32_t t = millis();
    if (t - last300 > 300) {
      if (pin3_on) digitalWrite(3, LOW);
      else digitalWrite(3, HIGH);
      last300 = t;
    }
    if (t - last500 > 500) {
      ... etc ...

Similar problems arise with input as well: I might want to check for a button (including debouncing the signal) while blinking a light. The central problem is the `delay()` function, which makes timing easy, but blocks the whole processor. So, the key feature of Adel is an asynchronous delay function called `adelay` (hence the name Adel). The `adelay` function works just like `delay`, but allows other code to run concurrently. 

Concurrency in Adel works on the function granularity, using a fork-join style of parallelism. Adel functions are defined in a stylized way, and can use any of the Adel library routines. In addition to `adelay`, the following routines are supported:

* `aboth( f , g )` : run Adel functions f and g concurrently until they **both** finish.
* `auntileither( f , g )` : runt Adel functions f and g concurrently until **one** of them finishes.
* `andthen( f )` : run Adel function f to completion before continuing.
* `areturn` : finish executing the current function

Using these routines we can rewrite the blink routine:

    Adel blink(int some_pin, int N) {
      abegin;
      while (1) {
        digitalWrite(some_pin, HIGH);
        adelay(N);
        digitalWrite(some_pin, LOW);
        adelay(N);
      }
      aend;
    }

Every Adel function must have a minimum of three things: return type `Adel`, and macros `abegin` and `aend` at the begining and end of the function. But otherwise, the code is almost identical. The key feature is that we can run blink concurrently, like this:

    aboth( blink(3, 500), blink(4, 500) );

This code does exactly what we want: it blinks the two lights at different intervals at the same time. We can aluse the `auntileither` macro to call two routines and check which one finished first. This macro is useful for things like timeouts:

    Adel timeout(int ms) {
      abegin;
      adelay(ms);
      end;
    }
    Adel button_or_timeout() {
      abegin;
      auntileither( button(), timeout(2000) ) {
        // -- Button was pushed (button() finished fist)
      } else {
        // -- Timeout finished first
      }
    }

Notice that the `auntileither` macro is set up to look like a control structure, which allows it to have arbitrary code for handling to two cases (which routine finished first).
