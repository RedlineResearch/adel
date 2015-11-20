
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

#define since(T) (millis() - T)

#define abegin static int state=0; switch (state) { case 0:

#define adelay(x)					 \
do {							 \
  state=__LINE__;					 \
  static int T##__LINE__;				 \
  T##__LINE__ = millis();				 \
 case __LINE__:						 \
   if (since(T##__LINE__) < x) return async(WAIT);	 \
 } while (0);

#define areturn(x)					\
  do {							\
    state=__LINE__;					\
    return async(x);					\
  case __LINE__: ;					\
  } while (0);

#define seq(f)						\
  do {							\
    state=__LINE__;					\
  case __LINE__:					\
    static async R##__LINE__;				\
    R##__LINE__ = f();					\
    if (R##__LINE__.waiting()) return async(WAIT);	\
  } while (0);

#define when(f, x)						\
    state=__LINE__;						\
  case __LINE__:						\
    static async R##__LINE__;					\
    R##__LINE__ = f();						\
    if (R##__LINE__.hasvalue())					\
      x = R##__LINE__.value;					\
    if (R##__LINE__.waiting())					\
      return R##__LINE__;

#define together(f1, f2)				\
  do {							\
    state=__LINE__;					\
  case __LINE__:					\
    static async R1##__LINE__;				\
    static async R2##__LINE__;						\
    if ( ! R1##__LINE__.done()) R1##__LINE__ = f1();			\
    if ( ! R2##__LINE__.done()) R2##__LINE__ = f2();			\
    if (R1##__LINE__.waiting() || R2##__LINE__.waiting()) return async(WAIT); \
  } while (0);

#define afinish } state = 0; return async(DONE);

