
enum aresult { START, WAIT, DONE, VALUE };

struct async
{
  aresult kind;
  int value;

  bool waiting() const { return kind == WAIT; }
  bool done() const { return kind == DONE; }
  bool hasvalue() const { return kind == VALUE; }
  void reset() { kind = START; }

  async() : kind(START), value(0) {}
  async(aresult k) : kind(k), value(0) {}
  async(int v) : kind(VALUE), value(v) {}
};

bool Debug = true;

#define ATOKEN2(X, Y) X ## Y
#define ATOKEN(X, Y) ATOKEN2(X, Y)
#define AVARNAME(n) ATOKEN(AV_##n##_, __LINE__)

#define marktime(V) 					 \
  state=__LINE__;					 \
  if (Debug) { Serial.print("State "); Serial.println(state); }	\
  static unsigned long ATOKEN(T,V) = 0;			 \
  ATOKEN(T,V) = millis();				 \
  Serial.print("Mark "); Serial.println(ATOKEN(T,V)); \
 case __LINE__:	;

#define since(V) (millis() - ATOKEN(T,V))

#define endtime(V) ATOKEN(T,V) = 0;

#define abegin static int state=0; \
   switch (state) { case 0:

#define adelay(x) {					 \
   state=__LINE__;					 \
   if (Debug) { Serial.print("State "); Serial.println(state); }	\
   static unsigned long AVARNAME(1);					 \
   AVARNAME(1) = millis();					 \
 case __LINE__:						 \
   if ((millis() - AVARNAME(1)) < x) return async(WAIT);	 \
}

#define at(t, V)  {					 \
   state=__LINE__;					 \
   if (Debug) { Serial.print("State "); Serial.println(state); }	\
 case __LINE__:						 \
   if ((millis() - ATOKEN(T,V)) < t) return async(WAIT);
  
#define astep(f) {					\
   state=__LINE__;					\
   Serial.print("State "); Serial.println(state); \
 case __LINE__:					        \
   static async AVARNAME(1);					\
   AVARNAME(1) = f();						\
   if (AVARNAME(1).waiting()) return async(WAIT);		\
}

#define await(f, res) {					\
    state=__LINE__;						\
    if (Debug) { Serial.print("State "); Serial.println(state); }	\
  case __LINE__:					        \
    static async AVARNAME(1);					\
    AVARNAME(1) = f();						\
    if (AVARNAME(1).waiting()) return async(WAIT);		\
    if (AVARNAME(1).hasvalue()) res = AVARNAME(1).value;		\
}

#define whenvalue(f, x)						\
   static async AVARNAME(1);					\
   AVARNAME(1) = f();						\
   if (AVARNAME(1).hasvalue() && ((x = AVARNAME(1).value) || true))

/* Our only real form of parallelism: fork-join */

#define together(f1, f2) {						\
    state=__LINE__;							\
    if (Debug) { Serial.print("State "); Serial.println(state); }	\
  case __LINE__:							\
    static async AVARNAME(1);						\
    static async AVARNAME(2);						\
    if ( ! AVARNAME(1).done()) AVARNAME(1) = f1();				\
    if ( ! AVARNAME(2).done()) AVARNAME(2) = f2();				\
    if (AVARNAME(1).waiting() || AVARNAME(2).waiting()) return async(WAIT);	\
    AVARNAME(1).reset(); AVARNAME(2).reset();				\
}

#define until(c) while (! (c))

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

