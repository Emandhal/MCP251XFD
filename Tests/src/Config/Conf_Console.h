


// TODO ADD DISCLAMER


#ifndef CONF_CONSOLE_H_
#define CONF_CONSOLE_H_
//=============================================================================

//-----------------------------------------------------------------------------





// This define adds the code for console transmit
#define USE_CONSOLE_TX
#ifdef USE_CONSOLE_TX
   // This define adds the code for ANSI Escape sequences on transmit
#  define USE_ANSI_ESCAPE_TX
#endif

// This define adds the code for console receive
#define USE_CONSOLE_RX




//-----------------------------------------------------------------------------
#endif /* CONF_CONSOLE_H_ */