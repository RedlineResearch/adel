
#include <stdint.h>

#define ADEL_V2

#ifdef ADEL_V2

#define achild1(n) ((n << 1) + 1)
#define achild2(n) ((n << 1) + 2)

#define AFINALLY 65535

extern uint16_t adel_step[64];
extern uint32_t adel_wait[64];
extern uint16_t adel_current;

#define adel bool

#define ainit adel_current = 0

#define abegin						\
  int a_me = adel_current;				\
  switch (adel_step[a_me]) { 				\
  case 0:

#define adelay(t)					\
    adel_step[a_me] = __LINE__;				\
    adel_wait[a_me] = millis() + t;			\
  case __LINE__:					\
  if (millis() < adel_wait[a_me]) return true;

#define astep( f )					\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild1(a_me)] = 0;			\
  case __LINE__:					\
    adel_current = achild1(a_me);			\
    if ( f ) return true;

#define atogether( f , g )				\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild1(a_me)] = 0;			\
    adel_step[achild2(a_me)] = 0;			\
  case __LINE__:					\
    adel_current = achild1(a_me);			\
    if ( f ) {						\
      adel_current = achild2(a_me);			\
      if ( g ) return true;				\
    }

#define auntil(c , f )					\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild1(a_me)] = 0;			\
    adel_step[achild2(a_me)] = 0;			\
  case __LINE__:					\
    adel_current = achild1(a_me);			\
    if ( c ) {						\
      adel_current = achild2(a_me);			\
      f;						\
      return true;					\
    }							\
    adel_step[achild2(a_me)] = AFINALLY;		\
    adel_current = achild2(a_me);			\
    f;

#define afinally case AFINALLY: 

#define aend						\
  }							\
  adel_step[a_me] = AFINALLY;				\
  return false;

#endif

#ifdef ADEL_V1

enum astate { START, WAIT, DONE };

struct adel
{
  astate state;
  int value;

  bool waiting() const { return state == WAIT; }
  bool done() const { return state == DONE; }
  void start() { state = START; }

  async() : state(START), value(0) {}
  async(astate k) : state(k), value(0) {}
  async(int v) : state(VALUE), value(v) {}
};

bool Debug = false;

#define ainput adel& astatus

#define abegin						\
  static int step=0;					\
  static unsigned long start_time = 0;			\
  static unsigned long continue_at = 0;			\
  static adel status1, status2;				\
  switch (step) { case 0: start_time = millis();

#define adelay(t)						\
   step=__LINE__;						\
   if (Debug) { Serial.print("Step "); Serial.println(step); }	\
   continue_at = millis() + t;					\
 case __LINE__:							\
   if (millis() < continue_at) return adel(WAIT);

#define at(t)							\
   state=__LINE__;						\
   if (Debug) { Serial.print("Step "); Serial.println(step); }	\
   continue_at = start_time + t;				\
 case __LINE__:							\
   if (millis() < continue_at) return adel(WAIT);
  
#define astep(f) 						\
   state=__LINE__;						\
   Serial.print("State "); Serial.println(state);		\
  case __LINE__:					        \
   status1 = f;							\
   if (status1.waiting()) return async(WAIT);		

/* Our only real form of parallelism: fork-join */

#define together(f1, f2) {						\
    state=__LINE__;							\
    if (Debug) { Serial.print("State "); Serial.println(state); }	\
    status1.
  case __LINE__:							\
    static async AVARNAME(1);						\
    static async AVARNAME(2);						\
    if ( ! AVARNAME(1).done()) AVARNAME(1) = f1();				\
    if ( ! AVARNAME(2).done()) AVARNAME(2) = f2();				\
    if (AVARNAME(1).waiting() || AVARNAME(2).waiting()) return async(WAIT);	\
    AVARNAME(1).reset(); AVARNAME(2).reset();				\
}

// Really need a special signal to the function when it gets interrupted

#define until(c, f) { \
    state=__LINE__;							\
    if (Debug) { Serial.print("State "); Serial.println(state); }	\
  case __LINE__:							\
    static async AVARNAME(1);						\
    static async AVARNAME(2);						\
    if ( ! AVARNAME(1).done()) AVARNAME(1) = c();			\
    if ( ! AVARNAME(2).done()) AVARNAME(2) = f();			\
    if (AVARNAME(1).waiting()) return async(WAIT);			\
    AVARNAME(1).reset(); AVARNAME(2).reset();				\
}

#define ayield(x) {					\
   state=__LINE__;					\
   if (Debug) { Serial.print("State "); Serial.println(state); }	\
   return async(x);					\
 case __LINE__:	;				        \
}

#define areturn(x) { \
  state = 0;	     \
  return async(x); \
}

#define afinish } state = 0; return async(DONE);

#endif
