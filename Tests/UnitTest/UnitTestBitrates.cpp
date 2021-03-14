#include "stdafx.h"
#include "CppUnitTest.h"
#include <cstdint>
#include <windows.h>
#include <WinUser.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework
{
  template<> inline std::wstring ToString<eERRORRESULT>(const eERRORRESULT& t) { RETURN_WIDE_STRING(t); }
  template<> inline std::wstring ToString<unsigned short>(const unsigned short& t) { RETURN_WIDE_STRING(t); }
  template<> inline std::wstring ToString<eMCP251XFD_TDCMode>(const eMCP251XFD_TDCMode& t) { RETURN_WIDE_STRING(t); }
} } }


namespace UnitTest_Bitrates
{
	TEST_CLASS(UnitTest_Bitrates)
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

		TEST_METHOD(TestMethod_MCP251XFD_CalculateBitTimeConfigurationCANFD)
		{
          const uint32_t SYSCLK_20MHz = 20000000; // SYSCLK at 20MHz
          const uint32_t SYSCLK_40MHz = 40000000; // SYSCLK at 40MHz
          const uint32_t NBR_125kbps  =   125000; // Normal Bitrate at 125kbps
          const uint32_t NBR_250kbps  =   250000; // Normal Bitrate at 250kbps
          const uint32_t NBR_500kbps  =   500000; // Normal Bitrate at 500kbps
          const uint32_t NBR_1Mbps    =  1000000; // Normal Bitrate at 1Mbps
          const uint32_t DBR_500kbps  =   500000; // Data Bitrate at 500kbps
          const uint32_t DBR_1Mbps    =  1000000; // Data Bitrate at 1Mbps
          const uint32_t DBR_2Mbps    =  2000000; // Data Bitrate at 2Mbps
          const uint32_t DBR_5Mbps    =  5000000; // Data Bitrate at 5Mbps
          const uint32_t DBR_8Mbps    =  8000000; // Data Bitrate at 8Mbps
          eERRORRESULT Error = ERR__TEST_ERROR;
          MCP251XFD_BitTimeConfig TestBitTimeConf;
          MCP251XFD_BitTimeStats TestStats;
          TestBitTimeConf.Stats = &TestStats;

          //=============================================================================
          // Parameters check
          //=============================================================================
          //--- Test (pConf == NULL) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps, NULL);
          Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (pConf == NULL), should be ERR__PARAMETER_ERROR");
          //--- Test (fsysclk < MCP251XFD_SYSCLK_MIN) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(MCP251XFD_SYSCLK_MIN - 1, NBR_500kbps, DBR_2Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fsysclk < MCP251XFD_SYSCLK_MIN), should be ERR__PARAMETER_ERROR");
          //--- Test (fsysclk > MCP251XFD_SYSCLK_MAX) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(MCP251XFD_SYSCLK_MAX + 1, NBR_500kbps, DBR_2Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fsysclk > MCP251XFD_SYSCLK_MAX), should be ERR__PARAMETER_ERROR");
          //--- Test (desiredNominalBitrate < MCP251XFD_NOMBITRATE_MIN) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, MCP251XFD_NOMBITRATE_MIN - 1, DBR_2Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR__BAUDRATE_ERROR, Error, L"Test (desiredNominalBitrate < MCP251XFD_NOMBITRATE_MIN), should be ERR__BAUDRATE_ERROR");
          //--- Test (desiredNominalBitrate > MCP251XFD_NOMBITRATE_MAX) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, MCP251XFD_NOMBITRATE_MAX + 1, DBR_2Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR__BAUDRATE_ERROR, Error, L"Test (desiredNominalBitrate > MCP251XFD_NOMBITRATE_MAX), should be ERR__BAUDRATE_ERROR");
          //--- Test (desiredDataBitrate < MCP251XFD_DATABITRATE_MIN) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, MCP251XFD_DATABITRATE_MIN - 1, &TestBitTimeConf);
          Assert::AreEqual(ERR__BAUDRATE_ERROR, Error, L"Test (desiredDataBitrate < MCP251XFD_DATABITRATE_MIN), should be ERR__BAUDRATE_ERROR");
          //--- Test (desiredNominalBitrate > MCP251XFD_DATABITRATE_MAX) ---
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, MCP251XFD_DATABITRATE_MAX + 1, &TestBitTimeConf);
          Assert::AreEqual(ERR__BAUDRATE_ERROR, Error, L"Test (desiredNominalBitrate > MCP251XFD_DATABITRATE_MAX), should be ERR__BAUDRATE_ERROR");



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_125kbps, NO_CANFD
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_125kbps, MCP251XFD_NO_CANFD, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual(0u, TestBitTimeConf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");         // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(254u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 254"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual(63u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 63");   // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual(63u, TestBitTimeConf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 63");       // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual( 0x0u, TestBitTimeConf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0x0");      // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(0x0Eu, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0x0E"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 0x3u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0x3");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 0x3u, TestBitTimeConf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 15");       // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_AUTO_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be AUTO_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(               0x10, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0x10");        // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(              0x00u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0x00");        // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_125kbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 125kbps"); // This is the actual nominal bitrate
          Assert::AreEqual(         0u, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");          // This is the actual data bitrate
          Assert::AreEqual(       426u, TestStats.MaxBusLength  , L"Test MaxBusLength (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 426m");      // This is the maximum bus length according to parameters
          Assert::AreEqual(      8000u, TestStats.NSamplePoint  , L"Test NSamplePoint (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 80.00%");    // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(         0u, TestStats.DSamplePoint  , L"Test DSamplePoint (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");         // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(       100u, TestStats.OscTolC1      , L"Test OscTolC1 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 1.00%");         // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolC2      , L"Test OscTolC2 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0.78%");         // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(         0u, TestStats.OscTolC3      , L"Test OscTolC3 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");             // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(         0u, TestStats.OscTolC4      , L"Test OscTolC4 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");             // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(         0u, TestStats.OscTolC5      , L"Test OscTolC5 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");             // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolerance  , L"Test OscTolerance (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 078%");      // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_1Mbps, NO_CANFD
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_1Mbps, MCP251XFD_NO_CANFD, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.NBRP  , L"Test NBRP (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");     // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(30u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 254"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 7u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 63");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 7u, TestBitTimeConf.NSJW  , L"Test NSJW (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 63");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual( 0x0u, TestBitTimeConf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0x0");      // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(0x0Eu, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0x0E"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 0x3u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0x3");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 0x3u, TestBitTimeConf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 15");       // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_AUTO_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be AUTO_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(               0x10, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0x10");        // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(              0x00u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0x00");        // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_1Mbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 1Mbps"); // This is the actual nominal bitrate
          Assert::AreEqual(       0u, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");        // This is the actual data bitrate
          Assert::AreEqual(       6u, TestStats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 6m");        // This is the maximum bus length according to parameters
          Assert::AreEqual(    8000u, TestStats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 80.00%");    // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(       0u, TestStats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");         // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(     100u, TestStats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 1.00%");             // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(      78u, TestStats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0.78%");             // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       0u, TestStats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");                 // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       0u, TestStats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");                 // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       0u, TestStats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");                 // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(      78u, TestStats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0.78%");     // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_125kbps, DBR_500kbps
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_125kbps, DBR_500kbps, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual(  0u, TestBitTimeConf.NBRP  , L"Test NBRP (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0");     // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(254u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 254"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 63u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 63");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 63u, TestBitTimeConf.NSJW  , L"Test NSJW (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 63");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.DBRP  , L"Test DBRP (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(62u, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 62"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual(15u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 15"); // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual(15u, TestBitTimeConf.DSJW  , L"Test DSJW (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 15");   // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_MANUAL_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be MANUAL_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(                   63, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 63");            // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(                   0u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0");             // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_125kbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 125kbps"); // This is the actual nominal bitrate
          Assert::AreEqual(DBR_500kbps, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 500kbps");    // This is the actual data bitrate
          Assert::AreEqual(       426u, TestStats.MaxBusLength  , L"Test MaxBusLength (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 426m");      // This is the maximum bus length according to parameters
          Assert::AreEqual(      8000u, TestStats.NSamplePoint  , L"Test NSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 80.00%");    // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(      8000u, TestStats.DSamplePoint  , L"Test DSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 80.00%");    // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(       100u, TestStats.OscTolC1      , L"Test OscTolC1 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 1.00%");         // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolC2      , L"Test OscTolC2 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.78%");         // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       100u, TestStats.OscTolC3      , L"Test OscTolC3 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 1.00%");         // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       118u, TestStats.OscTolC4      , L"Test OscTolC4 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 1.18%");         // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        87u, TestStats.OscTolC5      , L"Test OscTolC5 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.87%");         // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolerance  , L"Test OscTolerance (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.78%");     // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual(  0u, TestBitTimeConf.NBRP  , L"Test NBRP (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0");     // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(126u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 126"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 31u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 31");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 31u, TestBitTimeConf.NSJW  , L"Test NSJW (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 31");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.DBRP  , L"Test DBRP (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(30u, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 30"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 7u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 7");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 7u, TestBitTimeConf.DSJW  , L"Test DSJW (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 7");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_AUTO_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be AUTO_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(                 31, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 31");          // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(                 0u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0");           // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_250kbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 250kbps"); // This is the actual nominal bitrate
          Assert::AreEqual(  DBR_1Mbps, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1Mbps");      // This is the actual data bitrate
          Assert::AreEqual(       186u, TestStats.MaxBusLength  , L"Test MaxBusLength (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 186m");      // This is the maximum bus length according to parameters
          Assert::AreEqual(      8000u, TestStats.NSamplePoint  , L"Test NSamplePoint (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 80.00%");    // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(      8000u, TestStats.DSamplePoint  , L"Test DSamplePoint (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 80.00%");    // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(       100u, TestStats.OscTolC1      , L"Test OscTolC1 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1.00%");         // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual        (78u, TestStats.OscTolC2      , L"Test OscTolC2 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.78%");         // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       100u, TestStats.OscTolC3      , L"Test OscTolC3 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1.00%");         // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       118u, TestStats.OscTolC4      , L"Test OscTolC4 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1.18%");         // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        87u, TestStats.OscTolC5      , L"Test OscTolC5 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.87%");         // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolerance  , L"Test OscTolerance (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.78%");     // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.NBRP  , L"Test NBRP (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(62u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 62"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual(15u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 15"); // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual(15u, TestBitTimeConf.NSJW  , L"Test NSJW (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 15");   // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.DBRP  , L"Test DBRP (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(14u, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 14"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 3u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 3");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 3u, TestBitTimeConf.DSJW  , L"Test DSJW (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 3");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_AUTO_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be AUTO_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(                 15, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 15");          // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(                 0u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0");           // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_500kbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 500kbps"); // This is the actual nominal bitrate
          Assert::AreEqual(  DBR_2Mbps, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 2Mbps");      // This is the actual data bitrate
          Assert::AreEqual(        66u, TestStats.MaxBusLength  , L"Test MaxBusLength (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 66m");       // This is the maximum bus length according to parameters
          Assert::AreEqual(      8000u, TestStats.NSamplePoint  , L"Test NSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 80.00%");    // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(      8000u, TestStats.DSamplePoint  , L"Test DSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 80.00%");    // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(       100u, TestStats.OscTolC1      , L"Test OscTolC1 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 1.00%");         // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolC2      , L"Test OscTolC2 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.78%");         // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       100u, TestStats.OscTolC3      , L"Test OscTolC3 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 1.00%");         // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       118u, TestStats.OscTolC4      , L"Test OscTolC4 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 1.18%");         // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        87u, TestStats.OscTolC5      , L"Test OscTolC5 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.87%");         // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolerance  , L"Test OscTolerance (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.78%");     // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.NBRP  , L"Test NBRP (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(62u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 62"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual(15u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 15"); // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual(15u, TestBitTimeConf.NSJW  , L"Test NSJW (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 15");   // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual(0u, TestBitTimeConf.DBRP  , L"Test DBRP (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(4u, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 4");  // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual(1u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual(1u, TestBitTimeConf.DSJW  , L"Test DSJW (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_AUTO_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be AUTO_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(                  5, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 5");           // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(                 0u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0");           // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_500kbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 500kbps"); // This is the actual nominal bitrate
          Assert::AreEqual(  DBR_5Mbps, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 5Mbps");      // This is the actual data bitrate
          Assert::AreEqual(        66u, TestStats.MaxBusLength  , L"Test MaxBusLength (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 66m");       // This is the maximum bus length according to parameters
          Assert::AreEqual(      8000u, TestStats.NSamplePoint  , L"Test NSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 80.00%");    // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(      7500u, TestStats.DSamplePoint  , L"Test DSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 75.00%");    // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(       100u, TestStats.OscTolC1      , L"Test OscTolC1 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1.00%");         // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        78u, TestStats.OscTolC2      , L"Test OscTolC2 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.78%");         // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       125u, TestStats.OscTolC3      , L"Test OscTolC3 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1.25%");         // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(       132u, TestStats.OscTolC4      , L"Test OscTolC4 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1.32%");         // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        56u, TestStats.OscTolC5      , L"Test OscTolC5 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.56%");         // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(        56u, TestStats.OscTolerance  , L"Test OscTolerance (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.56%");     // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)



          //=============================================================================
          // Test SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps
          //=============================================================================
          Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps, &TestBitTimeConf);
          Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be ERR_OK");
          //--- Nominal Bit Times ---
          Assert::AreEqual( 0u, TestBitTimeConf.NBRP  , L"Test NBRP (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");     // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(30u, TestBitTimeConf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 254"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual( 7u, TestBitTimeConf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 63");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual( 7u, TestBitTimeConf.NSJW  , L"Test NSJW (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 63");    // Synchronization Jump Width bits; Length is value x TQ
          //--- Data Bit Times ---
          Assert::AreEqual(0u, TestBitTimeConf.DBRP  , L"Test DBRP (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0x0");    // Baud Rate Prescaler bits; TQ = value/Fsys
          Assert::AreEqual(2u, TestBitTimeConf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0x0E"); // Time Segment 1 bits (Propagation Segment + Phase Segment 1); Length is value x TQ
          Assert::AreEqual(0u, TestBitTimeConf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0x3");  // Time Segment 2 bits (Phase Segment 2); Length is value x TQ
          Assert::AreEqual(0u, TestBitTimeConf.DSJW  , L"Test DSJW (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 15");     // Synchronization Jump Width bits; Length is value x TQ
          //--- Transmitter Delay Compensation ---
          Assert::AreEqual(MCP251XFD_AUTO_MODE, TestBitTimeConf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be AUTO_MODE"); // Transmitter Delay Compensation Mode; Secondary Sample Point (SSP)
          Assert::AreEqual(                  3, TestBitTimeConf.TDCO  , L"Test TDCO (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0x10");        // Transmitter Delay Compensation Offset; Secondary Sample Point (SSP). Two’s complement; offset can be positive, zero, or negative (used as positive only here)
          Assert::AreEqual(                 0u, TestBitTimeConf.TDCV  , L"Test TDCV (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0x00");        // Transmitter Delay Compensation Value; Secondary Sample Point (SSP)
          //--- Result Statistics ---
          Assert::AreEqual(NBR_1Mbps, TestStats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1Mbps"); // This is the actual nominal bitrate
          Assert::AreEqual(DBR_8Mbps, TestStats.DataBitrate   , L"Test DataBitrate (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 8Mbps");    // This is the actual data bitrate
          Assert::AreEqual(       6u, TestStats.MaxBusLength  , L"Test MaxBusLength (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 6m");      // This is the maximum bus length according to parameters
          Assert::AreEqual(    8000u, TestStats.NSamplePoint  , L"Test NSamplePoint (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 80.00%");  // Nominal Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(    8000u, TestStats.DSamplePoint  , L"Test DSamplePoint (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 80.00%");  // Data Sample Point. Should be as close as possible to 80%
          Assert::AreEqual(     100u, TestStats.OscTolC1      , L"Test OscTolC1 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1.00%");       // Condition 1 for the maximum tolerance of the oscillator (Equation 3-12 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(      78u, TestStats.OscTolC2      , L"Test OscTolC2 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0.78%");       // Condition 2 for the maximum tolerance of the oscillator (Equation 3-13 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(     100u, TestStats.OscTolC3      , L"Test OscTolC3 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1.00%");       // Condition 3 for the maximum tolerance of the oscillator (Equation 3-14 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(     129u, TestStats.OscTolC4      , L"Test OscTolC4 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1.29%");       // Condition 4 for the maximum tolerance of the oscillator (Equation 3-15 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(      53u, TestStats.OscTolC5      , L"Test OscTolC5 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0.54%");       // Condition 5 for the maximum tolerance of the oscillator (Equation 3-16 of MCP25XXFD Family Reference Manual)
          Assert::AreEqual(      53u, TestStats.OscTolerance  , L"Test OscTolerance (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0.54%");   // Oscillator Tolerance, minimum of conditions 1-5 (Equation 3-11 of MCP25XXFD Family Reference Manual)
        }
	};
}