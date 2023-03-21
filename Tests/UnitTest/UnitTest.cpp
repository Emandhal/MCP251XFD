#include "stdafx.h"
#include "CppUnitTest.h"
#include <cstdint>
#include <windows.h>
#include <WinUser.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {namespace VisualStudio {namespace CppUnitTestFramework
{
  template<> inline std::wstring ToString<eERRORRESULT>(const eERRORRESULT& t) { RETURN_WIDE_STRING(t); }
//  template<> inline std::wstring ToString<unsigned short>(const unsigned short& t) { RETURN_WIDE_STRING(t); }
} } }

namespace UnitTest_Misc
{
	TEST_CLASS(UnitTest_Misc)
	{
	private:

		/*bool DoEvents()
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return false;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return true;
		}*/



	public:

        TEST_METHOD(TestMethod_SubstractWithWrap)
        {
//#define TIME_DIFF(begin,end)  ((end) >= (begin) ? (end) - (begin) : ((UINT32_MAX - (begin)) + ((end) + 1)))
#define TIME_DIFF(begin,end) (((end) >= (begin)) ? ((end) - (begin)) : (UINT32_MAX - ((begin) - (end) - 1)))

            uint32_t begin = 0;
            uint32_t end = begin + 10;
            uint32_t Result = TIME_DIFF(begin, end);
            Assert::AreEqual(10u, Result, L"SUBSTRACT_WITH_WRAP(0, 10): result should be '10'");

            begin = UINT32_MAX - 2;
            end = begin + 13;
            Result = TIME_DIFF(begin, end);
            Assert::AreEqual(13u, Result, L"SUBSTRACT_WITH_WRAP(UINT32_MAX-2, 10): result should be '13'");

            begin = UINT32_MAX - 2;
            end = begin + 3;
            Result = TIME_DIFF(begin, end);
            Assert::AreEqual(3u, Result, L"SUBSTRACT_WITH_WRAP(UINT32_MAX-2, 0): result should be '3'");
        }

		TEST_METHOD(TestMethod_ComputeCRC16USB)
		{
			// https://crccalc.com
			// http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

			const uint8_t TESTVECTOR_EMPTYSTRING[1]    = "";
			const uint8_t TESTVECTOR_123456789[10]     = "123456789";
			const uint8_t TESTVECTOR_ALPHABETIC_LC[27] = "abcdefghijklmnopqrstuvwxyz";
			const uint8_t TESTVECTOR_ALPHABETIC_UC[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			const uint8_t TESTVECTOR_0x0001020304[5]   = { 0x00, 0x01, 0x02, 0x03, 0x04 };
			uint16_t Result = 0;

			Result = ComputeCRC16USB(NULL, 0);
			Assert::AreEqual((uint16_t)0x0000, Result, L"ComputeCRC16USB(NULL, 0): result should be '0x0000'");

			Result = ComputeCRC16USB(TESTVECTOR_EMPTYSTRING, 0);
			Assert::AreEqual((uint16_t)0x0000, Result, L"ComputeCRC16USB('', 0): result should be '0x0000'");

			Result = ComputeCRC16USB(&TESTVECTOR_123456789[0], 9);
			Assert::AreEqual((uint16_t)0xB4C8, Result, L"ComputeCRC16USB('123456789', 9): result should be '0xB4C8'");

			Result = ComputeCRC16USB(&TESTVECTOR_0x0001020304[0],5);
			Assert::AreEqual((uint16_t)0xF07A, Result, L"ComputeCRC16USB(0x0001020304, 9): result should be '0xB4C8'");

			Result = ComputeCRC16USB(&TESTVECTOR_ALPHABETIC_LC[0], 26);
			Assert::AreEqual((uint16_t)0x8580, Result, L"ComputeCRC16USB('abcdefghijklmnopqrstuvwxyz', 26): result should be '0x8580'");

			Result = ComputeCRC16USB(&TESTVECTOR_ALPHABETIC_UC[0], 26);
			Assert::AreEqual((uint16_t)0x017A, Result, L"ComputeCRC16USB('ABCDEFGHIJKLMNOPQRSTUVWXYZ', 26): result should be '0x017A'");
		}

    TEST_METHOD(TestMethod_ComputeCRC15CAN)
    {
      // https://crccalc.com
      // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

      const uint8_t TESTVECTOR_EMPTYSTRING[1]    = "";
      const uint8_t TESTVECTOR_123456789[10]     = "123456789";
      const uint8_t TESTVECTOR_ALPHABETIC_LC[27] = "abcdefghijklmnopqrstuvwxyz";
      const uint8_t TESTVECTOR_ALPHABETIC_UC[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      const uint8_t TESTVECTOR_0x0001020304[5]   = { 0x00, 0x01, 0x02, 0x03, 0x04 };
      uint16_t Result = 0;

      Result = ComputeCRC15CAN(NULL, 0);
      Assert::AreEqual((uint16_t)0x0000, Result, L"ComputeCRC16USB(NULL, 0): result should be '0x0000'");

      Result = ComputeCRC15CAN(TESTVECTOR_EMPTYSTRING, 0);
      Assert::AreEqual((uint16_t)0x0000, Result, L"ComputeCRC16USB('', 0): result should be '0x0000'");

      Result = ComputeCRC15CAN(&TESTVECTOR_123456789[0], 9*8);
      Assert::AreEqual((uint16_t)0x059E, Result, L"ComputeCRC16USB('123456789', 9*8): result should be '0x059E'");

      Result = ComputeCRC15CAN(&TESTVECTOR_0x0001020304[0], 5*8);
      Assert::AreEqual((uint16_t)0x6A75, Result, L"ComputeCRC16USB(0x0001020304, 9*8): result should be '0x6A75'");

      Result = ComputeCRC15CAN(&TESTVECTOR_ALPHABETIC_LC[0], 26*8);
      Assert::AreEqual((uint16_t)0x1155, Result, L"ComputeCRC16USB('abcdefghijklmnopqrstuvwxyz', 26*8): result should be '0x1155'");

      Result = ComputeCRC15CAN(&TESTVECTOR_ALPHABETIC_UC[0], 26*8);
      Assert::AreEqual((uint16_t)0x6BEC, Result, L"ComputeCRC16USB('ABCDEFGHIJKLMNOPQRSTUVWXYZ', 26*8): result should be '0x6BEC'");
    }

    TEST_METHOD(TestMethod_ComputeCRC16CMS)
    {
      // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

      const uint8_t TESTVECTOR_EMPTYSTRING[1]    = "";
      const uint8_t TESTVECTOR_123456789[10]     = "123456789";
      const uint8_t TESTVECTOR_ALPHABETIC_LC[27] = "abcdefghijklmnopqrstuvwxyz";
      const uint8_t TESTVECTOR_ALPHABETIC_UC[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      const uint8_t TESTVECTOR_0x0001020304[5]   = { 0x00, 0x01, 0x02, 0x03, 0x04 };
      uint16_t Result = 0;

      Result = ComputeCRC16CMS(NULL, 0);
      Assert::AreEqual((uint16_t)0x0000, Result, L"ComputeCRC16CMS(NULL, 0): result should be '0x0000'");

      Result = ComputeCRC16CMS(TESTVECTOR_EMPTYSTRING, 0);
      Assert::AreEqual((uint16_t)0xFFFF, Result, L"ComputeCRC16CMS('', 0): result should be '0xFFFF'");

      Result = ComputeCRC16CMS(&TESTVECTOR_123456789[0], 9);
      Assert::AreEqual((uint16_t)0xAEE7, Result, L"ComputeCRC16CMS('123456789', 9): result should be '0xAEE7'");

      Result = ComputeCRC16CMS(&TESTVECTOR_0x0001020304[0], 5);
      Assert::AreEqual((uint16_t)0xBA33, Result, L"ComputeCRC16CMS(0x0001020304, 9): result should be '0xBA33'");

      Result = ComputeCRC16CMS(&TESTVECTOR_ALPHABETIC_LC[0], 26);
      Assert::AreEqual((uint16_t)0x1B42, Result, L"ComputeCRC16CMS('abcdefghijklmnopqrstuvwxyz', 26): result should be '0x1B42'");

      Result = ComputeCRC16CMS(&TESTVECTOR_ALPHABETIC_UC[0], 26);
      Assert::AreEqual((uint16_t)0x6245, Result, L"ComputeCRC16CMS('ABCDEFGHIJKLMNOPQRSTUVWXYZ', 26): result should be '0x6245'");
    }

    TEST_METHOD(TestMethod_AllFunc_pComp_NULL)
    {
        eERRORRESULT Error = ERR__TEST_ERROR;
      uint8_t TmpArrayUint8[10];
      uint16_t TmpUint16 = 0x0000;
      uint32_t TmpUint32 = 0x00000000;
      bool TmpBool = false;

      //=============================================================================
      // Parameters (pComp == NULL)
      //=============================================================================
      MCP251XFD_Config Conf;
      Error = Init_MCP251XFD(NULL, &Conf);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test Init_MCP251XFD(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_InitRAM(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_InitRAM(NULL), should be ERR__PARAMETER_ERROR");

      eMCP251XFD_Devices Devices;
      Error = MCP251XFD_GetDeviceID(NULL, &Devices, NULL, NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetDeviceID(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_TransmitMessageObjectToFIFO(NULL, &TmpArrayUint8[0], 1, MCP251XFD_FIFO1, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_TransmitMessageObjectToFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_TransmitMessageObjectToTXQ(NULL, &TmpArrayUint8[0], 1, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_TransmitMessageObjectToTXQ(NULL, ...), should be ERR__PARAMETER_ERROR");

      MCP251XFD_CANMessage MessageToSend;
      Error = MCP251XFD_TransmitMessageToFIFO(NULL, &MessageToSend, MCP251XFD_FIFO1, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_TransmitMessageToFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_TransmitMessageToTXQ(NULL, &MessageToSend, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_TransmitMessageToTXQ(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ReceiveMessageObjectFromFIFO(NULL, &TmpArrayUint8[0], 1, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ReceiveMessageObjectFromFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ReceiveMessageObjectFromTEF(NULL, &TmpArrayUint8[0], 1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ReceiveMessageObjectFromTEF(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ReceiveMessageFromFIFO(NULL, &MessageToSend, MCP251XFD_PAYLOAD_8BYTE, NULL, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ReceiveMessageFromFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ReceiveMessageFromTEF(NULL, &MessageToSend, NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ReceiveMessageFromTEF(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ConfigureCRC(NULL, MCP251XFD_CRC_ALL_EVENTS);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureCRC(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_CRCEvents CRCEvents;
      Error = MCP251XFD_GetCRCEvents(NULL, &CRCEvents, &TmpUint16);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetCRCEvents(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearCRCEvents(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearCRCEvents(NULL), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ConfigureECC(NULL, true, MCP251XFD_ECC_ALL_EVENTS, 0x00);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureECC(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_ECCEvents ECCEvents;
      Error = MCP251XFD_GetECCEvents(NULL, &ECCEvents, &TmpUint16);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetECCEvents(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearECCEvents(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearECCEvents(NULL), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ConfigurePins(NULL, MCP251XFD_PIN_AS_INT0_TX, MCP251XFD_PIN_AS_INT1_RX, MCP251XFD_PINS_PUSHPULL_OUT, MCP251XFD_PINS_PUSHPULL_OUT, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigurePins(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_SetGPIOPinsDirection(NULL, 0x00, 0x00);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_SetGPIOPinsDirection(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetGPIOPinsInputLevel(NULL, &TmpArrayUint8[0]);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetGPIOPinsInputLevel(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_SetGPIOPinsOutputLevel(NULL, 0x00, 0x00);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_SetGPIOPinsOutputLevel(NULL, ...), should be ERR__PARAMETER_ERROR");

      MCP251XFD_BitTimeConfig BTConf;
      Error = MCP251XFD_SetBitTimeConfiguration(NULL, &BTConf, true);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_SetBitTimeConfiguration(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_AbortAllTransmissions(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_AbortAllTransmissions(NULL), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_OperationMode ActualMode;
      Error = MCP251XFD_GetActualOperationMode(NULL, &ActualMode);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetActualOperationMode(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_RequestOperationMode(NULL, MCP251XFD_RESTRICTED_OPERATION_MODE, true);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_RequestOperationMode(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_WaitOperationModeChange(NULL, MCP251XFD_RESTRICTED_OPERATION_MODE);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_WaitOperationModeChange(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_StartCAN20(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_StartCAN20(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_StartCANFD(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_StartCANFD(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ConfigureCANController(NULL, MCP251XFD_CAN_RESTRICTED_MODE_ON_ERROR, MCP251XFD_NO_DELAY);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_WaitOperationModeChange(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ConfigureSleepMode(NULL, true, MCP251XFD_NO_FILTER, true);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureSleepMode(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_EnterSleepMode(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_EnterSleepMode(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_IsDeviceInSleepMode(NULL, &TmpBool);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_IsDeviceInSleepMode(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_PowerStates FromState;
      Error = MCP251XFD_WakeUp(NULL, &FromState);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_WakeUp(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ConfigureTimeStamp(NULL, true, MCP251XFD_TS_CAN20_SOF, 0x0000, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureTimeStamp(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_SetTimeStamp(NULL, 0x00000000);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_SetTimeStamp(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetTimeStamp(NULL, &TmpUint32);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTimeStamp(NULL, ...), should be ERR__PARAMETER_ERROR");

      MCP251XFD_FIFO ConfFIFO;
      Error = MCP251XFD_ConfigureTEF(NULL, true, &ConfFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureTEF(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ConfigureTXQ(NULL, true, &ConfFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureTXQ(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ConfigureFIFO(NULL, &ConfFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ConfigureFIFOList(NULL, &ConfFIFO, 1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureFIFOList(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ResetFIFO(NULL, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ResetFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ResetTEF(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ResetTEF(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ResetTXQ(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ResetTXQ(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_UpdateFIFO(NULL, MCP251XFD_FIFO1, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_UpdateFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_UpdateTEF(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_UpdateTEF(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_UpdateTXQ(NULL, false);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_UpdateTXQ(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_FlushFIFO(NULL, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_FlushFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_FlushTXQ(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_FlushTXQ(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_FlushAllFIFO(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_FlushAllFIFO(NULL), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_FIFOstatus StatusFlags;
      Error = MCP251XFD_GetFIFOStatus(NULL, MCP251XFD_FIFO1, &StatusFlags);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetFIFOStatus(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_TEFstatus TEFStatusFlags;
      Error = MCP251XFD_GetTEFStatus(NULL, &TEFStatusFlags);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTEFStatus(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_TXQstatus TXQStatusFlags;
      Error = MCP251XFD_GetTXQStatus(NULL, &TXQStatusFlags);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTXQStatus(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetNextMessageAddressFIFO(NULL, MCP251XFD_FIFO1, &TmpUint32, &TmpArrayUint8[0]);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetNextMessageAddressFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetNextMessageAddressTEF(NULL, &TmpUint32);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetNextMessageAddressTEF(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetNextMessageAddressTXQ(NULL, &TmpUint32, &TmpArrayUint8[0]);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetNextMessageAddressTXQ(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearFIFOConfiguration(NULL, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearFIFOConfiguration(NULL, ...), should be ERR__PARAMETER_ERROR");

      eMCP251XFD_InterruptEvents InterruptsFlags;
      Error = MCP251XFD_ConfigureInterrupt(NULL, MCP251XFD_INT_ENABLE_ALL_EVENTS);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureInterrupt(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetInterruptEvents(NULL, &InterruptsFlags);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetInterruptEvents(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_InterruptFlagCode CurrentEvent;
      Error = MCP251XFD_GetCurrentInterruptEvent(NULL, &CurrentEvent);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetCurrentInterruptEvent(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearInterruptEvents(NULL, MCP251XFD_INT_ENABLE_ALL_EVENTS);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearInterruptEvents(NULL, ...), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_FIFO FIFOName;
      eMCP251XFD_FIFOstatus Flags;
      Error = MCP251XFD_GetCurrentReceiveFIFONameAndStatusInterrupt(NULL, &FIFOName, &Flags);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetCurrentReceiveFIFONameAndStatusInterrupt(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetCurrentReceiveFIFONameInterrupt(NULL, &FIFOName);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetCurrentReceiveFIFONameInterrupt(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetCurrentTransmitFIFONameAndStatusInterrupt(NULL, &FIFOName, &Flags);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetCurrentTransmitFIFONameAndStatusInterrupt(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetCurrentTransmitFIFONameInterrupt(NULL, &FIFOName);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetCurrentTransmitFIFONameInterrupt(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearFIFOEvents(NULL, MCP251XFD_FIFO1, MCP251XFD_RX_FIFO_OVERFLOW);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearFIFOEvents(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearTEFOverflowEvent(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearTEFOverflowEvent(NULL), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearFIFOOverflowEvent(NULL, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearFIFOOverflowEvent(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearFIFOAttemptsEvent(NULL, MCP251XFD_FIFO1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearFIFOAttemptsEvent(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearTXQAttemptsEvent(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearTXQAttemptsEvent(NULL), should be ERR__PARAMETER_ERROR");
      eMCP251XFD_InterruptOnFIFO InterruptOnFIFO;
      Error = MCP251XFD_GetReceiveInterruptStatusOfAllFIFO(NULL, &InterruptOnFIFO, &InterruptOnFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetReceiveInterruptStatusOfAllFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetReceivePendingInterruptStatusOfAllFIFO(NULL, &InterruptOnFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetReceivePendingInterruptStatusOfAllFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetReceiveOverflowInterruptStatusOfAllFIFO(NULL, &InterruptOnFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetReceiveOverflowInterruptStatusOfAllFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetTransmitInterruptStatusOfAllFIFO(NULL, &InterruptOnFIFO, &InterruptOnFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTransmitInterruptStatusOfAllFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetTransmitPendingInterruptStatusOfAllFIFO(NULL, &InterruptOnFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTransmitPendingInterruptStatusOfAllFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetTransmitAttemptInterruptStatusOfAllFIFO(NULL, &InterruptOnFIFO);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTransmitAttemptInterruptStatusOfAllFIFO(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ConfigureDeviceNetFilter(NULL, MCP251XFD_D_NET_FILTER_DISABLE);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureDeviceNetFilter(NULL, ...), should be ERR__PARAMETER_ERROR");
      MCP251XFD_Filter ConfFilter;
      Error = MCP251XFD_ConfigureFilter(NULL, &ConfFilter);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureFilter(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ConfigureFilterList(NULL, MCP251XFD_D_NET_FILTER_DISABLE, &ConfFilter, 1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ConfigureFilterList(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_DisableFilter(NULL, MCP251XFD_FILTER1);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_DisableFilter(NULL, ...), should be ERR__PARAMETER_ERROR");

      eMCP251XFD_TXRXErrorStatus TXRXErrorStatus;
      Error = MCP251XFD_GetTransmitReceiveErrorCountAndStatus(NULL, &TmpArrayUint8[0], &TmpArrayUint8[1], &TXRXErrorStatus);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTransmitReceiveErrorCountAndStatus(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetTransmitErrorCount(NULL, &TmpArrayUint8[0]);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTransmitErrorCount(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetReceiveErrorCount(NULL, &TmpArrayUint8[0]);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetReceiveErrorCount(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_GetTransmitReceiveErrorStatus(NULL, &TXRXErrorStatus);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetTransmitReceiveErrorStatus(NULL, ...), should be ERR__PARAMETER_ERROR");
      MCP251XFD_CiBDIAG0_Register CiBDIAG0Reg;
      MCP251XFD_CiBDIAG1_Register CiBDIAG1Reg;
      Error = MCP251XFD_GetBusDiagnostic(NULL, &CiBDIAG0Reg, &CiBDIAG1Reg);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_GetBusDiagnostic(NULL, ...), should be ERR__PARAMETER_ERROR");
      Error = MCP251XFD_ClearBusDiagnostic(NULL, true, true);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ClearBusDiagnostic(NULL, ...), should be ERR__PARAMETER_ERROR");

      Error = MCP251XFD_ResetDevice(NULL);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test MCP251XFD_ResetDevice(NULL), should be ERR__PARAMETER_ERROR");
    }
	};
}

/*#include <CppUnitTest.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

BEGIN_TEST_MODULE_ATTRIBUTE()
TEST_MODULE_ATTRIBUTE(L"Date", L"2010/6/12")
END_TEST_MODULE_ATTRIBUTE()


TEST_MODULE_INITIALIZE(ModuleInitialize)
{
	Logger::WriteMessage("In Module Initialize");
}

TEST_MODULE_CLEANUP(ModuleCleanup)
{
	Logger::WriteMessage("In Module Cleanup");
}



TEST_CLASS(Class1)
{
public:
	Class1()
	{
		Logger::WriteMessage("In Class1");
	}
	~Class1()
	{
		Logger::WriteMessage("In ~Class1");
	}



	TEST_CLASS_INITIALIZE(ClassInitialize)
	{
		Logger::WriteMessage("In Class Initialize");
	}
	TEST_CLASS_CLEANUP(ClassCleanup)
	{
		Logger::WriteMessage("In Class Cleanup");
	}


	BEGIN_TEST_METHOD_ATTRIBUTE(Method1)
		TEST_OWNER(L"OwnerName")
		TEST_PRIORITY(1)
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Method1)
	{
		Logger::WriteMessage("In Method1");
		Assert::AreEqual(0, 0);
	}

	TEST_METHOD(Method2)
	{
		Assert::Fail(L"Fail");
	}
};*/
