/*
 * Hardware setup:
 *
 * 1) Plug the mikroBus Xplained Pro adapter board into connector EXT1 of the SAM V71 Xplained Ultra evaluation kit.
 * 2) Select 3.3V at the jumper selection: VDD of the MCP2517FD/MCP2518FD.
 * 3) Select 40MHz at the jumper selection of the frequency of the MCP2517FD/MCP2518FD.
 * 4) Connect 5V of the POWER connector of SAM V71 Xplained Ultra to 5V External Power Header of the adapter board (jumper wire): 5V for VCC of ATA6563.
 * 5) Plug MCP2517FD click or a MCP2518FD click into adapter board.
 * 6) Power SAM V71 Xplained by connecting a USB cable to the DEBUG connector and plugging it into your PC.
 * 7) Connect a terminal to see the console debug
 * 8) Connect a wire on the DB9 pin 7 MCP2517FD/MCP2518FD click and the other end to the J1000 pin 1 (CAN_H) of the SAM V71 Xplained Ultra
 * 9) Connect a wire on the DB9 pin 2 MCP2517FD/MCP2518FD click and the other end to the J1000 pin 2 (CAN_L) of the SAM V71 Xplained Ultra
 * 10) Make sure both ends of the CAN bus are terminated.
 * 
 * Bonus add a secondary board:
 * 11) Plug the mikroBus Xplained Pro adapter board into connector EXT2 of the SAM V71 Xplained Ultra evaluation kit.
 * 12) Select 3.3V at the jumper selection: VDD of the MCP2517FD/MCP2518FD.
 * 13) Select 40MHz at the jumper selection of the frequency of the MCP2517FD/MCP2518FD.
 * 14) Connect 5V of the POWER connector of SAM V71 Xplained Ultra to 5V External Power Header of the adapter board (jumper wire): 5V for VCC of ATA6563.
 * 15) Plug MCP2517FD click or a MCP2518FD click into adapter board.
 * 16) Connect the DB9 to a CAN to PC adapter to see some frame on the computer
 */
//=============================================================================

//-----------------------------------------------------------------------------
#include <asf.h> // Use Atmel Software Framework (ASF)
#include "string.h"
#include "Main.h"
#include "CANEXTFunctions.h"
#include "MCANFunctions.h"
#include "ShowDemoData.h"
//-----------------------------------------------------------------------------



const char ButtonStateStr[2][7+1/* \0 */] =
{
  "push",
  "release",
};

//const char* AutoRTRmessage = "DEMO";



// Variables of program
volatile uint32_t msCount;                        //! Milli-seconds count from start of the system
CAN_Error MCAN_LastError = CAN_NO_ERROR;          //! Last error on MCAN catch in the interrupt
bool Ext1ModulePresent = false;                   //! True if a MCP251XFD is present on EXT1 and configured
bool Ext2ModulePresent = false;                   //! True if a MCP251XFD is present on EXT2 and configured
uint32_t Ext1MessageCounter = FPS_COUNTER_OFF;    //! Ext1 message counter for the FPS (Frame Per Second) command
uint32_t Ext1FPSTickCount   = 0;                  //! Ext1 FPS tick count
uint32_t Ext1CLKOCounter    = 0;                  //! Ext1 CLKO counter
Ext2MessagesStats Ext2Stats[EXT2_COUNTERS_COUNT]; //! Ext2 message counter for the MPS (Messages Per Second) and byte count
uint32_t Ext2SequenceCounter = 0;                 //! Ext2 sequence counter
uint32_t Ext2FPSTickCount    = 0;                 //! Ext2 FPS tick count
uint32_t Ext2CLKOCounter     = 0;                 //! Ext2 CLKO counter
uint32_t ExtCLKOTickCount    = 0;                 //! Ext1 CLKO tick count
ButtonState LastButton1State = RELEASED;
ButtonState LastButton2State = RELEASED;
bool ContinuousSendMessagesExt1 = false;          //! Do not send messages at startup

//**********************************************************************************************************************************************************





/**
 * \brief Interrupt handler for MCAN,
 *   inlcuding RX,TX,ERROR and so on processes.
 */
void MCAN1_INT0_Handler(void)
{
	volatile uint32_t status;

	status = mcan_read_interrupt_status(&mcan_instance);
	if (status & MCAN_RX_BUFFER_NEW_MESSAGE)
  {
		mcan_clear_interrupt_status(&mcan_instance, MCAN_RX_BUFFER_NEW_MESSAGE);
//		for (uint32_t i = 0; i < CONF_MCAN0_RX_BUFFER_NUM; i++)
//		{
//  		if (mcan_rx_get_buffer_status(&mcan_instance, i))
//  		{
//    		uint32_t rx_buffer_index = i;
//    		mcan_rx_clear_buffer_status(&mcan_instance, i);
//    		mcan_get_rx_buffer_element(&mcan_instance, &rx_element_buffer, rx_buffer_index);
//    		if (rx_element_buffer.R0.bit.XTD)
//    		{
//      		LOGDEBUG("\n\r Extended FD message received in Rx buffer.");
//      		} else {
//      		LOGDEBUG("\n\r Standard FD message received in Rx buffer.");
//    		}
//
//  		}
//    }

    // Do stuff
	}

	if (status & MCAN_RX_FIFO_0_NEW_MESSAGE)
  {
		mcan_clear_interrupt_status(&mcan_instance, MCAN_RX_FIFO_0_NEW_MESSAGE);
//		mcan_get_rx_fifo_0_element(&mcan_instance, &rx_element_fifo_0, standard_receive_index);
//		mcan_rx_fifo_acknowledge(&mcan_instance, 0, standard_receive_index);
//		standard_receive_index++;
//		if (standard_receive_index == CONF_MCAN0_RX_FIFO_0_NUM) standard_receive_index = 0;
//
//		if (rx_element_fifo_0.R1.bit.EDL)
//    {
//			LOGDEBUG("Standard FD message received in FIFO 0.");
//		}
//    else
//    {
//			LOGDEBUG("Standard normal message received in FIFO 0.");
//		}

    // Do stuff
	}

	if (status & MCAN_RX_FIFO_1_NEW_MESSAGE) {
		mcan_clear_interrupt_status(&mcan_instance, MCAN_RX_FIFO_1_NEW_MESSAGE);
//		mcan_get_rx_fifo_1_element(&mcan_instance, &rx_element_fifo_1, extended_receive_index);
//		mcan_rx_fifo_acknowledge(&mcan_instance, 0, extended_receive_index);
//		extended_receive_index++;
//		if (extended_receive_index == CONF_MCAN0_RX_FIFO_1_NUM) extended_receive_index = 0;

    // Do stuff
	}

	if ((status & MCAN_ACKNOWLEDGE_ERROR) || (status & MCAN_FORMAT_ERROR))
  {
		mcan_clear_interrupt_status(&mcan_instance, MCAN_ACKNOWLEDGE_ERROR | MCAN_FORMAT_ERROR);
    MCAN_LastError = CAN_PROTOCOL_ERROR;
	}

	if (status & MCAN_BUS_OFF)
  {
		mcan_clear_interrupt_status(&mcan_instance, MCAN_BUS_OFF);
		mcan_stop(&mcan_instance);
		MCAN_LastError = CAN_BUS_OFF;
		ConfigureMCAN1Device();
	}
}





//=============================================================================
// SysTick Handler
//=============================================================================
void SysTick_Handler(void)
{
  msCount++;
  if (Ext1MessageCounter != FPS_COUNTER_OFF) Ext1FPSTickCount++;
  Ext2FPSTickCount++;
  ExtCLKOTickCount++;
}





//=============================================================================
// Process command in buffer
//=============================================================================
static void ProcessCommand(void)
{
  if (CommandInput.ToProcess == false) return;
  CommandInput.ToProcess = false;
  if (CommandInput.Buffer[0] != '*') return;       // First char should be a '*'
  CommandInput.Buffer[0] = '\0';                   // Break the current command in the buffer

  for (size_t z = 1; z < COMMAND_BUFFER_SIZE; z++) // Put string in lower case
    CommandInput.Buffer[z] = LowerCase(CommandInput.Buffer[z]);

  char* pBuf = &CommandInput.Buffer[1];
  eERRORRESULT Error;

  //--- Check the command ---
  eConsoleCommand ConsoleCmd = NO_COMMAND;
  if (strncmp(pBuf, "start" , 6) == 0) ConsoleCmd = START;     // "*Start" command
  if (strncmp(pBuf, "stop"  , 5) == 0) ConsoleCmd = STOP;      // "*Stop" command
  if (strncmp(pBuf, "sleep" , 6) == 0) ConsoleCmd = SLEEP;     // "*Sleep" command
  if (strncmp(pBuf, "dsleep", 7) == 0) ConsoleCmd = DEEPSLEEP; // "*DSleep" command
  if (strncmp(pBuf, "wakeup", 7) == 0) ConsoleCmd = WAKEUP;    // "*WakeUp" command
  if (strncmp(pBuf, "diag1" , 6) == 0) ConsoleCmd = DIAGEXT1;  // "*Diag1" command
  if (strncmp(pBuf, "diag2" , 6) == 0) ConsoleCmd = DIAGEXT2;  // "*Diag2" command
  if (strncmp(pBuf, "fps"   , 4) == 0) ConsoleCmd = FPS;       // "*FPS" command

  if (ConsoleCmd == NO_COMMAND) return;
  SetStrToConsoleBuffer(CONSOLE_TX, "\r\n");

  //--- Do stuff ---
  eMCP251XFD_PowerStates PowerStateBeforeWakeUp;
  switch (ConsoleCmd)
  {
    case START:
      LOGTRACE("Ext1: Continuous send messages start");
      ContinuousSendMessagesExt1 = true;
      break;
    case STOP:
      LOGTRACE("Ext1: Continuous send messages stop");
      ContinuousSendMessagesExt1 = false;
      break;
    case SLEEP:
      Error = MCP251XFD_ConfigureSleepMode(CANEXT1, false, MCP251XFD_T11FILTER_300ns, true);
      if (Error == ERR_OK)
      {
        MCP251XFD_EnterSleepMode(CANEXT1);
        LOGINFO("Ext1: The device will enter sleep mode after the current transmit/receive");
      }
      else ShowDeviceError(CANEXT1, Error);
      break;
    case DEEPSLEEP:
      Error = MCP251XFD_ConfigureSleepMode(CANEXT1, true, MCP251XFD_T11FILTER_300ns, true);
      if (Error == ERR_OK)
      {
         MCP251XFD_EnterSleepMode(CANEXT1);
         LOGINFO("Ext1: The device will enter deep sleep mode after the current transmit/receive");
      }
      else
      {
        if (Error == ERR__NOT_SUPPORTED)
             LOGERROR("Ext1: Deep Sleep not supported on this device (not available on MCP2517FD)");
        else ShowDeviceError(CANEXT1, Error);
      }
      break;
    case WAKEUP:
      MCP251XFD_WakeUp(CANEXT1, &PowerStateBeforeWakeUp);
      LOGINFO("Ext1: The device is waking up");
      AfterWakeUpActionsOnEXT1(PowerStateBeforeWakeUp);
      break;
    case DIAGEXT1:
      GetAndShowMCP251XFD_SFRreg(CANEXT1);
      GetAndShowMCP251XFD_CANSFRreg(CANEXT1);
      GetAndShowMCP251XFD_FIFOreg(CANEXT1);
      GetAndShowMCP251XFD_FILTERreg(CANEXT1);
      break;
    case DIAGEXT2:
      GetAndShowMCP251XFD_SFRreg(CANEXT2);
      GetAndShowMCP251XFD_CANSFRreg(CANEXT2);
      GetAndShowMCP251XFD_FIFOreg(CANEXT2);
      GetAndShowMCP251XFD_FILTERreg(CANEXT2);
      break;
    case FPS:
      Ext1MessageCounter = 0;
      Ext1FPSTickCount   = 0;
      break;

    case NO_COMMAND:
    default: return;
  }
  TransmitMessageToEXT2(SID_MENU_EVENT, &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_1BYTE, &ConsoleCmd, MCP251XFD_TXQ, true); // Send the last console command by EXT2
}





//=============================================================================
// Main
//=============================================================================
int main (void)
{
  wdt_disable(WDT);

  //--- Configure system clock --------------------------
	sysclk_init();
  SystemCoreClock = sysclk_get_cpu_hz();

  //--- Initialize board --------------------------------
	board_init();
  // Configure the push button 2
  ioport_set_pin_dir(GPIO_PUSH_BUTTON_2, IOPORT_DIR_INPUT);
  ioport_set_pin_mode(GPIO_PUSH_BUTTON_2, GPIO_PUSH_BUTTON_2_FLAGS);
  ioport_set_pin_sense_mode(GPIO_PUSH_BUTTON_2, GPIO_PUSH_BUTTON_2_SENSE);
  MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO12; // PB12 function selected (Button2 is connected on PB12)
  MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;  // PB4 function selected (EXT2_INT1 connected on PB4)

  //--- Initialize the console UART ---------------------
  InitConsoleTx(CONSOLE_TX);
  InitConsoleRx(CONSOLE_RX);

  //--- Demo start --------------------------------------
  printf("\r\n\r\n");
  LOGTITLE("MCP251XFD Demo start...");

  //--- Configure SysTick base timer --------------------
  SysTick_Config(SystemCoreClock * SYSTEM_TICK_MS / 1000); // (Fmck(Hz)*1/1000)=1ms

  //--- Configure the MCP251XFD device on EXT1 ----------
//  eERRORRESULT ErrorExt1 = ERR_NOTAVAILABLE;
  eERRORRESULT ErrorExt1 = ConfigureMCP251XFDDeviceOnEXT1();
  if (ErrorExt1 != ERR_OK)
  {
    ioport_set_pin_level(LED0_GPIO, LED0_ACTIVE_LEVEL);
    ShowDeviceError(CANEXT1, ErrorExt1); // Show device error
    Ext1ModulePresent = false;
  }
  else
  {
    ShowDeviceDetected(CANEXT1, SYSCLK_Ext1);
    ShowDeviceConfiguration(&MCP2517FD_Ext1_BTStats);
    ShowDeviceFIFOConfiguration(&MCP2517FD_Ext1_FIFOlist[0], MCP2517FD_EXT1_FIFO_COUNT);
    ShowDeviceFilterConfiguration(&MCP2517FD_Ext1_FilterList[0], MCP2517FD_EXT1_FILTER_COUNT, false);
    ShowMoreDeviceConfiguration(CANEXT1);
    Ext1ModulePresent = true;
  }

  //--- Configure the MCP251XFD device on EXT2 ----------
//  eERRORRESULT ErrorExt2 = ERR_NOTAVAILABLE;
  eERRORRESULT ErrorExt2 = ConfigureMCP251XFDDeviceOnEXT2();
  if (ErrorExt2 != ERR_OK)
  {
    ioport_set_pin_level(LED1_GPIO, LED1_ACTIVE_LEVEL);
    ShowDeviceError(CANEXT2, ErrorExt2); // Show device error
    Ext2ModulePresent = false;
  }
  else
  {
    ShowDeviceDetected(CANEXT2, SYSCLK_Ext2);
    ShowDeviceConfiguration(&MCP2517FD_Ext2_BTStats);
    ShowDeviceFIFOConfiguration(&MCP2517FD_Ext2_FIFOlist[0], MCP2517FD_EXT2_FIFO_COUNT);
    ShowDeviceFilterConfiguration(&MCP2517FD_Ext2_FilterList[0], MCP2517FD_EXT2_FILTER_COUNT, false);
    ShowMoreDeviceConfiguration(CANEXT2);
    Ext2ModulePresent = true;
  }

  //--- Configure MCAN1 ---------------------------------
  ConfigureMCAN1Device();

  //--- Reset watchdog ----------------------------------
  wdt_restart(WDT);

  //--- Log ---------------------------------------------
	LOGTRACE("Initialization complete");

  //--- Display menu ------------------------------------
  LOGINFO("Available commands:");
  LOGINFO("  *Start : Start a continuous send of CAN messages on Ext1");
  LOGINFO("  *Stop  : Stop the continuous send of CAN messages on Ext1");
  LOGINFO("  *Sleep : Put the device into sleep");
  LOGINFO("  *DSleep: Put the device into deep sleep (only on MCP2518FD)");
  LOGINFO("  *WakeUp: Wake-up the device");
  LOGINFO("  *Diag1 : Show all status of SFR registers of the device in Ext1");
  LOGINFO("  *Diag2 : Show all status of SFR registers of the device in Ext2");
  LOGINFO("  *FPS   : Show the frame per second on the bus");

  //--- Miscellaneous -----------------------------------
  memset(&Ext2Stats, 0, sizeof(Ext2Stats));

  //=== The main loop ===================================
  while(1)
  {
    //--- Flush char by char console buffer ---
    TrySendingNextCharToConsole(CONSOLE_TX);

    //--- Process command if any available ---
    ProcessCommand();

    //--- Process FPS ---
    if (Ext1FPSTickCount > 999)
    {
      LOGINFO("FPS Ext1: %u", (unsigned int)Ext1MessageCounter);
      Ext1MessageCounter = FPS_COUNTER_OFF;
      Ext1FPSTickCount -= 1000;
    }
    if (ExtCLKOTickCount > 999)
    {
      MCP251XFD_uint32t_Conv ValueExt1, ValueExt2;
      ValueExt1.Uint32 = Ext1CLKOCounter;
      Ext1CLKOCounter = 0;
      ValueExt2.Uint32 = Ext2CLKOCounter;
      Ext2CLKOCounter = 0;

      TransmitMessageToEXT2(SID_EXT1_CLKO_EVENT, &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_4BYTE, &ValueExt1.Bytes[0], MCP251XFD_FIFO26, true);
      TransmitMessageToEXT2(SID_EXT2_CLKO_EVENT, &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_4BYTE, &ValueExt2.Bytes[0], MCP251XFD_FIFO27, true);
      ExtCLKOTickCount -= 1000;
    }
    if (Ext2FPSTickCount > 999)
    {
      // Send frames bytes count + fps
      TransmitMessageToEXT2(SID_0x000_to_0x1FF_MPS_EVENT    , &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[SID_0x000_to_0x1FF].Bytes[0]    , MCP251XFD_FIFO5 , true);
      TransmitMessageToEXT2(SID_0x200_to_0x3FF_MPS_EVENT    , &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[SID_0x200_to_0x3FF].Bytes[0]    , MCP251XFD_FIFO6 , true);
      TransmitMessageToEXT2(SID_0x400_to_0x5FF_MPS_EVENT    , &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[SID_0x400_to_0x5FF].Bytes[0]    , MCP251XFD_FIFO7 , true);
      TransmitMessageToEXT2(SID_0x600_to_0x7FF_MPS_EVENT    , &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[SID_0x600_to_0x7FF].Bytes[0]    , MCP251XFD_FIFO8 , true);
      TransmitMessageToEXT2(EID_0x00000_to_0x07FFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x00000_to_0x07FFF].Bytes[0], MCP251XFD_FIFO17, true);
      TransmitMessageToEXT2(EID_0x08000_to_0x0FFFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x08000_to_0x0FFFF].Bytes[0], MCP251XFD_FIFO18, true);
      TransmitMessageToEXT2(EID_0x10000_to_0x17FFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x10000_to_0x17FFF].Bytes[0], MCP251XFD_FIFO19, true);
      TransmitMessageToEXT2(EID_0x18000_to_0x1FFFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x18000_to_0x1FFFF].Bytes[0], MCP251XFD_FIFO20, true);
      TransmitMessageToEXT2(EID_0x20000_to_0x27FFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x20000_to_0x27FFF].Bytes[0], MCP251XFD_FIFO21, true);
      TransmitMessageToEXT2(EID_0x28000_to_0x2FFFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x28000_to_0x2FFFF].Bytes[0], MCP251XFD_FIFO22, true);
      TransmitMessageToEXT2(EID_0x30000_to_0x37FFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x30000_to_0x37FFF].Bytes[0], MCP251XFD_FIFO23, true);
      TransmitMessageToEXT2(EID_0x38000_to_0x3FFFF_MPS_EVENT, &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EID_0x38000_to_0x3FFFF].Bytes[0], MCP251XFD_FIFO24, true);
      TransmitMessageToEXT2(EID_ALL_EXT2_MESSAGES_MPS_EVENT , &Ext2SequenceCounter, MCP251XFD_EXTENDED_MESSAGE_ID, MCP251XFD_DLC_8BYTE, &Ext2Stats[EXT2_ALL].Bytes[0]              , MCP251XFD_FIFO30, true);
      memset(&Ext2Stats, 0, sizeof(Ext2Stats));
      Ext2FPSTickCount -= 1000;
    }



    //--- CAN bus errors seen by MCAN1 ---
    if (MCAN_LastError != CAN_NO_ERROR)
    {
      if (MCAN_LastError == CAN_PROTOCOL_ERROR) LOGERROR("Protocol error, please double check the clock in two boards.");
      if (MCAN_LastError == CAN_BUS_OFF       ) LOGERROR("MCAN bus off error, re-initialization.");
      MCAN_LastError = CAN_NO_ERROR;
    }

    //--- Add a new message in the transmission of MCAN ---
    if (ContinuousSendMessagesExt1)
      mcan_send_standard_message(MCAN_LOWEST_SIZE_SID, &TransmitMessageData[0], 0); // Send the lowest bit sized frame (45 bits per frame, stuffing include [frame is SID:0x084, RTR:0, IDE:0, r0:0, DLC:0]), for maximum frame on the bus



    //--- Get a frame if available in the MCP251XFD on Ext1 ---
    ReceiveMessageFromEXT1();

    //--- Check status of the MCP251XFD on Ext1 ---
    CheckDeviceINTOnEXT1();



    //--- Get a frame if available in the MCP251XFD on Ext2 ---
    ReceiveMessageFromEXT2();

    //--- Check status of the MCP251XFD on Ext2 ---
    CheckDeviceINTOnEXT2();



    //--- Use Button1 to change Ext1 GPIO0 state ---
    ButtonState NewButton1State = (ioport_get_pin_level(GPIO_PUSH_BUTTON_1) ? RELEASED : PUSHED);
    if (NewButton1State != LastButton1State)
    {
      LOGTRACE("Button1 state: %s", ButtonStateStr[NewButton1State]);
      LastButton1State = NewButton1State;
      if (Ext1ModulePresent == true)
      {
        if (NewButton1State == PUSHED) EXT1_GPIO0_Low; else EXT1_GPIO0_High;
      }
      ButtonState ReturnButton1State = (ioport_get_pin_level(EXT1_PIN_13) ? RELEASED : PUSHED);
      TransmitMessageToEXT2(SID_BUTTON1_EVENT, &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_1BYTE, &ReturnButton1State, MCP251XFD_TXQ, true); // Send the actual state of the Button1 through GPIO0 of Ext1 module which return to the MCU by EXT1 pin 13
    }

    //--- Send a message when pushing Button2 ---
    ButtonState NewButton2State = (ioport_get_pin_level(GPIO_PUSH_BUTTON_2) ? RELEASED : PUSHED);
    if (NewButton2State != LastButton2State)
    {
      LastButton2State = NewButton2State;
      if (NewButton2State == PUSHED) mcan_send_standard_message(MCAN_LOWEST_SIZE_SID, &TransmitMessageData[0], 0); // For maximum frame on the bus (45 bits per frame, stuffing include), frame is SID:0x084, RTR:0, IDE:0, r0:0, DLC:0)
    }

    //--- Fill RTR FIFO with a frame for automatic send ---
    // Moved to EXT2_INT on FIFO25_INTERRUPT for automatism
    /*if (Ext2ModulePresent == true)
    {
      eMCP251XFD_FIFOstatus FIFOstatus = 0;
      ErrorExt2 = MCP251XFD_GetFIFOStatus(CANEXT2, FIFO25, &FIFOstatus); // Get FIFO25 status (in the default demo configuration, FIFO25 is the FIFO with the automatic RTR response)
      if (((FIFOstatus & TX_FIFO_EMPTY) > 0) && (ErrorExt2 == ERR_OK))   // Check FIFO empty and no error while retrieve FIFO status
      {
        TransmitMessageToEXT2(SID_DEMO_RTR_RESPONSE, &Ext2SequenceCounter, STANDARD_MESSAGE_ID, DLC_4BYTE, (uint8_t*)AutoRTRmessage, FIFO25, false); // Prepare a message with "DEMO" in case of 0x000 message incoming
      }
    }*/


    //--- Send last error on Ext1 through Ext2 ---
    if (ErrorExt1 != ERR_OK)
    {
      TransmitMessageToEXT2(SID_EXT1_ERROR_EVENT, &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_1BYTE, &ErrorExt1, MCP251XFD_FIFO29, true); // Send last error on Ext1 module through Ext2
      ErrorExt1 = ERR_OK;
    }

    //--- Send last error on Ext2 through Ext2 ---
    if (ErrorExt2 != ERR_OK)
    {
      TransmitMessageToEXT2(SID_EXT2_ERROR_EVENT, &Ext2SequenceCounter, MCP251XFD_STANDARD_MESSAGE_ID, MCP251XFD_DLC_1BYTE, &ErrorExt1, MCP251XFD_FIFO31, true); // Send last error on Ext2 module through Ext2
      ErrorExt2 = ERR_OK;
    }

    /*CiBDIAG0_Register Diag0;
    CiBDIAG1_Register Diag1;
    ErrorExt1 = MCP251XFD_GetBusDiagnostic(CANEXT1, &Diag0, &Diag1);
    if (ErrorExt1 != ERR_OK) ShowDeviceError(CANEXT1, ErrorExt1); // Show device error
    if (Diag0.CiBDIAG0 == 0xFFFFFFFF) LOGDEBUG("Oops0");
    if (Diag1.CiBDIAG1 == 0xFFFFFFFF) LOGDEBUG("Oops1");*/

    nop();
  }
}