/*
 * Main.h
 *
 * Created: 09/04/2020 15:57:04
 *  Author: Fabien
 */


#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "conf_board.h"
#include "Console.h"
#include "Interface/Console_V71Interface.h"





// System defines
#	define SYSTEM_TICK_MS  (1) // 1ms for system tick



//! MCAN can errors
typedef enum
{
  CAN_NO_ERROR = 0,
  CAN_PROTOCOL_ERROR,
  CAN_BUS_OFF,
} CAN_Error;



//! Button state
typedef enum
{
  PUSHED = 0,
  RELEASED,
} ButtonState;



//! Console commands
typedef enum
{
  NO_COMMAND = 0,
  START,
  STOP,
  SLEEP,
  DEEPSLEEP,
  WAKEUP,
  DIAGEXT1,
  DIAGEXT2,
  FPS,
} eConsoleCommand;



#define FPS_COUNTER_OFF  0xFFFFFFFF



//! Milli-seconds count from start of the system
extern volatile uint32_t msCount;
//! True if a MCP251XFD is present on EXT1 and configured
extern bool Ext1ModulePresent;
//! True if a MCP251XFD is present on EXT2 and configured
extern bool Ext2ModulePresent;
//! Ext1 message counter for the FPS (Frame Per Second) command
extern uint32_t Ext1MessageCounter;
//! Do not send messages at startup
extern bool ContinuousSendMessagesExt1;
//! Ext1 CLKO counter
extern uint32_t Ext1CLKOCounter;
//! Ext CLKO tick count
extern uint32_t ExtCLKOTickCount;



//! Enum Ext2 counters
typedef enum
{
  SID_0x000_to_0x1FF = 0,
  SID_0x200_to_0x3FF,
  SID_0x400_to_0x5FF,
  SID_0x600_to_0x7FF,
  EID_0x00000_to_0x07FFF,
  EID_0x08000_to_0x0FFFF,
  EID_0x10000_to_0x17FFF,
  EID_0x18000_to_0x1FFFF,
  EID_0x20000_to_0x27FFF,
  EID_0x28000_to_0x2FFFF,
  EID_0x30000_to_0x37FFF,
  EID_0x38000_to_0x3FFFF,
  EXT2_ALL,
  EXT2_COUNTERS_COUNT     // Stay last
} eExt2Counters;

typedef union Ext2MessagesStats
{
  uint8_t Bytes[8];
  struct
  {
    uint32_t MessagesPerSecond;
    uint32_t ByteCount;
  };
} Ext2MessagesStats;

//! Ext2 message stats
extern Ext2MessagesStats Ext2Stats[EXT2_COUNTERS_COUNT];
//! Ext2 sequence counter
extern uint32_t Ext2SequenceCounter;
//! Ext2 CLKO counter
extern uint32_t Ext2CLKOCounter;





//**********************************************************************************************************************************************************



// Test
void TestFunc(void);





#endif /* MAIN_H_ */