
int millis();

enum aresult { NONE, WAIT, DONE, VALUE };

struct async
{
  aresult kind;
  int value;

  bool waiting() const { return kind == WAIT; }
  bool done() const { return kind == DONE; }
  bool hasvalue() const { return kind == VALUE; }
  
  async() : kind(NONE), value(0) {}
  async(aresult k) : kind(k), value(0) {}
  async(int v) : kind(VALUE), value(v) {}
};

#define ATOKEN2(X, Y) X ## Y
#define ATOKEN(X, Y) ATOKEN2(X, Y)
#define AVAR ATOKEN(AV_, __LINE__)
#define AVARN(N) ATOKEN(AV##N##_, __LINE__)

#define marker(V) 					 \
  state=__LINE__;					 \
  static int ATOKEN(T,V) = 0;			 \
  ATOKEN(T,V) = millis();			 \
 case __LINE__:	;


#define since(V) (millis() - ATOKEN(T,V))

#define abegin static int state=0; switch (state) { case 0:

#define adelay(x) {					 \
  state=__LINE__;					 \
  static int AVAR;				 \
  AVAR = millis();				 \
 case __LINE__:						 \
   if ((millis() - AVAR) < x) return async(WAIT); \
}

#define areturn(x) {					\
  state=__LINE__;					\
  return async(x);					\
 case __LINE__:	;				\
}

/* Every call to an async function must be made using either seq (for
   sequential semantics) or together (for parallel semantics. */

#define seq(f) {					\
  state=__LINE__;					\
 case __LINE__:					\
  static async AVAR;				\
  AVAR = f();					\
  if (AVAR.waiting()) return async(WAIT);	\
}

#define seqf(f, res) {					\
  state=__LINE__;					\
 case __LINE__:					\
  static async AVAR;				\
  AVAR = f();					\
  if (AVAR.waiting()) return async(WAIT);	\
  if (AVAR.hasvalue()) res = AVAR.value;        \
}

/* Our only real form of parallelism: fork-join */
#define together(f1, f2) {				\
  state=__LINE__;					\
 case __LINE__:					\
   static async AVARN(1);						\
   static async AVARN(2);						\
   if ( ! AVARN(1).done()) AVARN(1) = f1();				\
   if ( ! AVARN(2).done()) AVARN(2) = f2();				\
   if (AVARN(2).waiting() || AVARN(2).waiting()) return async(WAIT);	\
}

/* The semantics of when are hard to implement with macros because we need
   to call the function f every time through the concrete execution, rather
   than stepping through as we would with an if */

#define when(f, x)							\
  state=__LINE__;							\
 case __LINE__:								\
   static async AVAR;							\
   AVAR = f();								\
   if (AVAR.waiting()) return async(WAIT);				\
   if (AVAR.hasvalue() && AVAR.value == x)					

/* We don't want a "go" operation -- then we're actually
   dealing with threads, and that's bad 
#define go(f)					\
  do { \
  state=__LINE__;
*/

#define until(c) while (! (c))

#define afinish } state = 0; return async(DONE);

