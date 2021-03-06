
#include <stdint.h>

#ifndef ADEL_V2
#define ADEL_V2

extern uint16_t adel_step[64];
extern uint32_t adel_wait[64];
extern uint16_t adel_current;

#define achild(c) adel_step[(a_me << 1) + c]

#define ADEL_FINALLY 99999

/** adel
 * 
 *  All Adel functions return an enum that indicates whether the routine is
 *  done or has more work to do.
 */
class Adel
{
public:
  typedef enum { NONE, DONE, CONT } _state;
  
private:
  _state m_state;
  
public:
  Adel(_state s) : m_state(s) {}
  Adel() : m_state(NONE) {}
  Adel(const Adel& other) : m_state(other.m_state) {}
  explicit Adel(bool b) : m_state(NONE) {}
  
  bool done() const { return m_state == DONE; }
  bool cont() const { return m_state == CONT; }
};
  
/** astart
 *
 *  Use astart in the Arduino loop function to initiate the Adel function f
 *  (and run all Adel functions below it).
 */
#define astart( f ) \
  adel_current = 0; \
  f;

/** abegin
 *
 * Always add abegin and aend to every adel function
 */
#define abegin						\
  Adel f_state, g_state;				\
  int a_me = adel_current;				\
  switch (adel_step[a_me]) { 				\
  case 0:

#define aend						\
  case ADEL_FINALLY: ;					\
  }							\
  adel_step[a_me] = ADEL_FINALLY;			\
  return Adel::DONE;

/** afinally
 *
 *  Optionally, end with afinally to do some action whenever the function
 *  returns (for any reason)
 */
#define afinally( f )					\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild(1)] = 0;				\
  case ADEL_FINALLY:					\
    adel_current = achild(1);				\
    f_state = f;					\
    if ( f_state.cont() ) return Adel::CONT;

/** adelay
 *
 *  Semantics: delay this function for t milliseconds
 */
#define adelay(t)					\
    adel_step[a_me] = __LINE__;				\
    adel_wait[a_me] = millis() + t;			\
  case __LINE__:					\
  if (millis() < adel_wait[a_me]) return Adel::CONT;

/** andthen
 *
 *  Semantics: execute f synchronously, until it is done (returns false)
 *  Example use:
 *     andthen( turn_on_light() );
 *     andthen( turn_off_light() );
 */
#define andthen( f )					\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild(1)] = 0;				\
 case __LINE__:						\
    adel_current = achild(1);				\
    f_state = f;					\
    if ( f_state.cont() ) return Adel::CONT;

/** await
 *  Wait asynchronously for a condition to become true.
 */
#define await( c )					\
    adel_step[a_me] = __LINE__;				\
 case __LINE__:						\
    if ( ! ( c ) ) return Adel::CONT

/** adountil
 *
 *  Semantics: do f until it completes, or until the timeout
 */
#define adountil( t, f )				\
    adel_step[a_me] = __LINE__;				\
    adel_wait[a_me] = millis() + t;			\
  case __LINE__:					\
    f_state = f;					\
    if (f_state.cont() && millis() < adel_wait[a_me]) return Adel::CONT;

/** aboth
 *
 *  Semantics: execute f and g asynchronously, until *both* are done
 *  (both return false). Example use:
 *      atogether( flash_led(), play_sound() );
 */
#define aboth( f , g )					\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild(1)] = 0;				\
    adel_step[achild(2)] = 0;				\
  case __LINE__: {					\
    adel_current = achild(1);				\
    f_state = f;					\
    adel_current = achild(2);				\
    g_state = g;					\
    if (f_state.cont() || g_state.cont())		\
      return Adel::CONT;   }

/** auntileither
 *
 *  Semantics: execute c and f asynchronously until either one of them
 *  finishes (contrast with aboth). This construct behaves like a
 *  conditional statement: it should be followed by a true and option false
 *  statement, which are executed depending on whether the first function
 *  finished first or the second one did. Example use:
 *     auntil( button(), flash_led() ) { 
 *       // button finished first 
 *     } else {
 *       // light finished first
 *     }
 */
#define auntileither( f , g )				\
    adel_step[a_me] = __LINE__;				\
    adel_step[achild(1)] = 0;				\
    adel_step[achild(2)] = 0;				\
  case __LINE__: 					\
    adel_current = achild(1);				\
    f_state = f;					\
    adel_current = achild(2);				\
    g_state = g;					\
    if (f_state.cont() && g_state.cont())		\
      return Adel::CONT;				\
    if (f_state.done())

/** areturn
 * 
 *  Semantics: leave the function immediately, and communicate to the
 *  caller that it is done.
 */
#define areturn				   \
    adel_step[a_me] = ADEL_FINALLY;	   \
    return Adel::CONT;

#endif
