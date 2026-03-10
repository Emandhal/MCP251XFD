#include "stdafx.h"
#include "CppUnitTest.h"
#include <cstdint>
#include <windows.h>
#include <WinUser.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
  namespace VisualStudio {
    namespace CppUnitTestFramework
    {
      template<> inline std::wstring ToString<eERRORRESULT>(const eERRORRESULT& t) { RETURN_WIDE_STRING(t); }
      template<> inline std::wstring ToString<eMCP251XFD_TDCMode>(const eMCP251XFD_TDCMode& t) { RETURN_WIDE_STRING(t); }
    }
  }
}


namespace UnitTest_Bitrates
{
  TEST_CLASS(UnitTest_Bitrates)
  {
  public:

    TEST_METHOD(TestMethod_MCP251XFD_ParameterValidation)
    {
      const uint32_t SYSCLK_40MHz = 40000000;
      const uint32_t NBR_500kbps = 500000;
      const uint32_t DBR_2Mbps = 2000000;
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig TestBitTimeConf;
      MCP251XFD_BitTimeStats TestStats;
      TestBitTimeConf.Stats = &TestStats;

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
      //--- Test (desiredDataBitrate > MCP251XFD_DATABITRATE_MAX) ---
      Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, MCP251XFD_DATABITRATE_MAX + 1, &TestBitTimeConf);
      Assert::AreEqual(ERR__BAUDRATE_ERROR, Error, L"Test (desiredDataBitrate > MCP251XFD_DATABITRATE_MAX), should be ERR__BAUDRATE_ERROR");
      //--- Test impossible combination ---
      Error = MCP251XFD_CalculateBitTimeConfiguration(20000000, 1000000, 8000000, &TestBitTimeConf);
      Assert::AreEqual(ERR__BITTIME_ERROR, Error, L"Test (SYSCLK_20MHz, NBR_1Mbps, DBR_8Mbps), should be ERR__BITTIME_ERROR");
      //--- Test (Stats == NULL) does not crash ---
      TestBitTimeConf.Stats = NULL;
      Error = MCP251XFD_CalculateBitTimeConfiguration(SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps, &TestBitTimeConf);
      Assert::AreEqual(ERR_OK, Error, L"Test (Stats == NULL, SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be ERR_OK without crash");
      TestBitTimeConf.Stats = &TestStats;
    }

    //=========================================================================
    // CAN 2.0 only tests (NO_CANFD)
    //=========================================================================
    TEST_METHOD(TestMethod_CAN20_40MHz_125kbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 125000, MCP251XFD_NO_CANFD, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(254u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 254");
      Assert::AreEqual( 63u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 63");
      Assert::AreEqual( 63u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 63");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0 (default)");
      Assert::AreEqual(14u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 14 (default)");
      Assert::AreEqual( 3u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 3 (default)");
      Assert::AreEqual( 3u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 3 (default)");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_TDC_DISABLED, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be TDC_DISABLED");
      Assert::AreEqual(                     0, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(                    0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual(125000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 125kbps");
      Assert::AreEqual(     0u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(   426u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 426m");
      Assert::AreEqual(  8000u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 80.00%");
      Assert::AreEqual(     0u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(   100u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 1.00%");
      Assert::AreEqual(    78u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0.78%");
      Assert::AreEqual(     0u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(     0u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(     0u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(    78u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_125kbps, NO_CANFD), should be 0.78%");
    }

    TEST_METHOD(TestMethod_CAN20_40MHz_250kbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 250000, MCP251XFD_NO_CANFD, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 0");
      Assert::AreEqual(138u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 138");
      Assert::AreEqual( 19u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 19");
      Assert::AreEqual( 19u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 19");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_TDC_DISABLED, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be TDC_DISABLED");
      //--- Result Statistics ---
      Assert::AreEqual(250000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 250kbps");
      Assert::AreEqual(   246u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 246m");
      Assert::AreEqual(  8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 87.00%");
      Assert::AreEqual(    62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 0.62%");
      Assert::AreEqual(    48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 0.48%");
      Assert::AreEqual(    48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_250kbps, NO_CANFD), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CAN20_40MHz_500kbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 500000, MCP251XFD_NO_CANFD, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual( 0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 0");
      Assert::AreEqual(68u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 68");
      Assert::AreEqual( 9u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 9");
      Assert::AreEqual( 9u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 9");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_TDC_DISABLED, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be TDC_DISABLED");
      //--- Result Statistics ---
      Assert::AreEqual(500000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 500kbps");
      Assert::AreEqual(    96u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 96m");
      Assert::AreEqual(  8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 87.00%");
      Assert::AreEqual(    62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 0.62%");
      Assert::AreEqual(    48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 0.48%");
      Assert::AreEqual(    48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_500kbps, NO_CANFD), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CAN20_40MHz_1Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 1000000, MCP251XFD_NO_CANFD, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual( 0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");
      Assert::AreEqual(28u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 28");
      Assert::AreEqual( 9u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 9");
      Assert::AreEqual( 9u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 9");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_TDC_DISABLED, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be TDC_DISABLED");
      Assert::AreEqual(                     0, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");
      Assert::AreEqual(                    0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual(1000000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 1Mbps");
      Assert::AreEqual(      0u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0");
      Assert::AreEqual(   7500u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 75.00%");
      Assert::AreEqual(    125u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 1.25%");
      Assert::AreEqual(     98u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0.98%");
      Assert::AreEqual(     98u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_1Mbps, NO_CANFD), should be 0.98%");
    }

    TEST_METHOD(TestMethod_CAN20_20MHz_125kbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(20000000, 125000, MCP251XFD_NO_CANFD, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 0");
      Assert::AreEqual(138u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 138");
      Assert::AreEqual( 19u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 19");
      Assert::AreEqual( 19u, Conf.NSJW, L"Test NSJW (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 19");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_TDC_DISABLED, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be TDC_DISABLED");
      //--- Result Statistics ---
      Assert::AreEqual(125000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 125kbps");
      Assert::AreEqual(   544u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 544m");
      Assert::AreEqual(  8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 87.00%");
      Assert::AreEqual(    48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_20MHz, NBR_125kbps, NO_CANFD), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CAN20_8MHz_1Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(8000000, 1000000, MCP251XFD_NO_CANFD, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(0u, Conf.NBRP, L"Test NBRP (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 0");
      Assert::AreEqual(4u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 4");
      Assert::AreEqual(1u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 1");
      Assert::AreEqual(1u, Conf.NSJW, L"Test NSJW (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 1");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_TDC_DISABLED, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be TDC_DISABLED");
      //--- Result Statistics ---
      Assert::AreEqual(1000000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 1Mbps");
      Assert::AreEqual(      0u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 0");
      Assert::AreEqual(   7500u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_8MHz, NBR_1Mbps, NO_CANFD), should be 75.00%");
    }

    //=========================================================================
    // CAN-FD tests
    //=========================================================================
    TEST_METHOD(TestMethod_CANFD_40MHz_125kbps_500kbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 125000, 500000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  1u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 1");
      Assert::AreEqual(138u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 138");
      Assert::AreEqual( 19u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 19");
      Assert::AreEqual( 19u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 19");
      //--- Data Bit Times ---
      Assert::AreEqual( 1u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 1");
      Assert::AreEqual(30u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 30");
      Assert::AreEqual( 7u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 7");
      Assert::AreEqual( 7u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 7");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be AUTO_MODE");
      Assert::AreEqual(                 63, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 63");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0");
      Assert::AreEqual(true, Conf.EDGE_FILTER, L"Test EDGE_FILTER (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be true");
      //--- Result Statistics ---
      Assert::AreEqual(125000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 125kbps");
      Assert::AreEqual(500000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 500kbps");
      Assert::AreEqual(   544u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 544m");
      Assert::AreEqual(  8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 87.00%");
      Assert::AreEqual(  8000u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 80.00%");
      Assert::AreEqual(    62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.62%");
      Assert::AreEqual(    48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.48%");
      Assert::AreEqual(   100u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 1.00%");
      Assert::AreEqual(    73u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.73%");
      Assert::AreEqual(    85u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.85%");
      Assert::AreEqual(    48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_125kbps, DBR_500kbps), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_250kbps_1Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 250000, 1000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0");
      Assert::AreEqual(138u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 138");
      Assert::AreEqual( 19u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 19");
      Assert::AreEqual( 19u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 19");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0");
      Assert::AreEqual(28u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 28");
      Assert::AreEqual( 9u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 9");
      Assert::AreEqual( 9u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 9");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be AUTO_MODE");
      Assert::AreEqual(                 30, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 30");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual( 250000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 250kbps");
      Assert::AreEqual(1000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1Mbps");
      Assert::AreEqual(    246u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 246m");
      Assert::AreEqual(   8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 87.00%");
      Assert::AreEqual(   7500u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 75.00%");
      Assert::AreEqual(     62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.62%");
      Assert::AreEqual(     48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.48%");
      Assert::AreEqual(    125u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1.25%");
      Assert::AreEqual(     74u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.74%");
      Assert::AreEqual(    106u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 1.06%");
      Assert::AreEqual(     48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_250kbps, DBR_1Mbps), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_500kbps_2Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 500000, 2000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual( 0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(68u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 68");
      Assert::AreEqual( 9u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 9");
      Assert::AreEqual( 9u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 9");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(13u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 13");
      Assert::AreEqual( 4u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 4");
      Assert::AreEqual( 4u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 4");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be AUTO_MODE");
      Assert::AreEqual(                 15, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 15");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual( 500000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 500kbps");
      Assert::AreEqual(2000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 2Mbps");
      Assert::AreEqual(     96u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 96m");
      Assert::AreEqual(   8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 87.00%");
      Assert::AreEqual(   7500u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 75.00%");
      Assert::AreEqual(     62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.62%");
      Assert::AreEqual(     48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.48%");
      Assert::AreEqual(    125u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 1.25%");
      Assert::AreEqual(     74u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.74%");
      Assert::AreEqual(    106u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 1.06%");
      Assert::AreEqual(     48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_500kbps, DBR_2Mbps), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_500kbps_5Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 500000, 5000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual( 0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0");
      Assert::AreEqual(68u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 68");
      Assert::AreEqual( 9u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 9");
      Assert::AreEqual( 9u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 9");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0");
      Assert::AreEqual( 4u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 4");
      Assert::AreEqual( 1u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1");
      Assert::AreEqual( 1u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be AUTO_MODE");
      Assert::AreEqual(                  6, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 6");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual( 500000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 500kbps");
      Assert::AreEqual(5000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 5Mbps");
      Assert::AreEqual(     96u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 96m");
      Assert::AreEqual(   8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 87.00%");
      Assert::AreEqual(   7500u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 75.00%");
      Assert::AreEqual(     62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.62%");
      Assert::AreEqual(     48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.48%");
      Assert::AreEqual(    125u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 1.25%");
      Assert::AreEqual(     82u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.82%");
      Assert::AreEqual(     54u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.54%");
      Assert::AreEqual(     48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_500kbps, DBR_5Mbps), should be 0.48%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_1Mbps_8Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 1000000, 8000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual( 0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");
      Assert::AreEqual(28u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 28");
      Assert::AreEqual( 9u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 9");
      Assert::AreEqual( 9u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 9");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");
      Assert::AreEqual( 2u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 2");
      Assert::AreEqual( 0u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");
      Assert::AreEqual( 0u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be AUTO_MODE");
      Assert::AreEqual(                  4, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 4");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual(1000000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1Mbps");
      Assert::AreEqual(8000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 8Mbps");
      Assert::AreEqual(      0u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0");
      Assert::AreEqual(   7500u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 75.00%");
      Assert::AreEqual(   8000u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 80.00%");
      Assert::AreEqual(    125u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1.25%");
      Assert::AreEqual(     98u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0.98%");
      Assert::AreEqual(    100u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1.00%");
      Assert::AreEqual(    161u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 1.61%");
      Assert::AreEqual(     54u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0.54%");
      Assert::AreEqual(     54u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_1Mbps, DBR_8Mbps), should be 0.54%");
    }

    TEST_METHOD(TestMethod_CANFD_20MHz_500kbps_2Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(20000000, 500000, 2000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual( 0u, Conf.NBRP, L"Test NBRP (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(33u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 33");
      Assert::AreEqual( 4u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 4");
      Assert::AreEqual( 4u, Conf.NSJW, L"Test NSJW (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 4");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0");
      Assert::AreEqual( 5u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 5");
      Assert::AreEqual( 2u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 2");
      Assert::AreEqual( 2u, Conf.DSJW, L"Test DSJW (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 2");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be AUTO_MODE");
      Assert::AreEqual(                  7, Conf.TDCO, L"Test TDCO (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 7");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual( 500000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 500kbps");
      Assert::AreEqual(2000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 2Mbps");
      Assert::AreEqual(     94u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 94m");
      Assert::AreEqual(   8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 87.00%");
      Assert::AreEqual(   7000u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 70.00%");
      Assert::AreEqual(     62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0.62%");
      Assert::AreEqual(     48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0.48%");
      Assert::AreEqual(    150u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 1.50%");
      Assert::AreEqual(     74u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0.74%");
      Assert::AreEqual(    127u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 1.27%");
      Assert::AreEqual(     48u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_20MHz, NBR_500kbps, DBR_2Mbps), should be 0.48%");
    }

    //=========================================================================
    // "Grand ecart" tests: large ratio between nominal and data bitrates
    //=========================================================================
    TEST_METHOD(TestMethod_CANFD_40MHz_125kbps_8Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 125000, 8000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual(254u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 254");
      Assert::AreEqual( 63u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 63");
      Assert::AreEqual( 63u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 63");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual( 2u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 2");
      Assert::AreEqual( 0u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual( 0u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be AUTO_MODE");
      Assert::AreEqual(                  4, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 4");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual(true, Conf.EDGE_FILTER, L"Test EDGE_FILTER (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be true");
      //--- Result Statistics ---
      Assert::AreEqual( 125000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 125kbps");
      Assert::AreEqual(8000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 8Mbps");
      Assert::AreEqual(    426u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 426m");
      Assert::AreEqual(   8000u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 80.00%");
      Assert::AreEqual(   8000u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 80.00%");
      Assert::AreEqual(    100u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 1.00%");
      Assert::AreEqual(     78u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0.78%");
      Assert::AreEqual(    100u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 1.00%");
      Assert::AreEqual(    141u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 1.41%");
      Assert::AreEqual(      8u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0.08%");
      Assert::AreEqual(      8u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_125kbps, DBR_8Mbps), should be 0.08%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_125kbps_2Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 125000, 2000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(254u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 254");
      Assert::AreEqual( 63u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 63");
      Assert::AreEqual( 63u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 63");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(13u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 13");
      Assert::AreEqual( 4u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 4");
      Assert::AreEqual( 4u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 4");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be AUTO_MODE");
      Assert::AreEqual(                 15, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 15");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual( 125000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 125kbps");
      Assert::AreEqual(2000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 2Mbps");
      Assert::AreEqual(    426u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 426m");
      Assert::AreEqual(   8000u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 80.00%");
      Assert::AreEqual(   7500u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 75.00%");
      Assert::AreEqual(    100u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 1.00%");
      Assert::AreEqual(     78u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 0.78%");
      Assert::AreEqual(    125u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 1.25%");
      Assert::AreEqual(    135u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 1.35%");
      Assert::AreEqual(     37u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 0.37%");
      Assert::AreEqual(     37u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_125kbps, DBR_2Mbps), should be 0.37%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_250kbps_8Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 250000, 8000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual(138u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 138");
      Assert::AreEqual( 19u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 19");
      Assert::AreEqual( 19u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 19");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual( 2u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 2");
      Assert::AreEqual( 0u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0");
      Assert::AreEqual( 0u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be AUTO_MODE");
      Assert::AreEqual(                  4, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 4");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual( 250000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 250kbps");
      Assert::AreEqual(8000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 8Mbps");
      Assert::AreEqual(    246u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 246m");
      Assert::AreEqual(   8700u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 87.00%");
      Assert::AreEqual(   8000u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 80.00%");
      Assert::AreEqual(     62u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0.62%");
      Assert::AreEqual(     48u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0.48%");
      Assert::AreEqual(    100u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 1.00%");
      Assert::AreEqual(     87u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0.87%");
      Assert::AreEqual(     15u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0.15%");
      Assert::AreEqual(     15u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_250kbps, DBR_8Mbps), should be 0.15%");
    }

    TEST_METHOD(TestMethod_CANFD_40MHz_1Mbps_2Mbps)
    {
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD_BitTimeConfig Conf;
      MCP251XFD_BitTimeStats Stats;
      Conf.Stats = &Stats;

      Error = MCP251XFD_CalculateBitTimeConfiguration(40000000, 1000000, 2000000, &Conf);
      Assert::AreEqual(ERR_OK, Error, L"Test (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be ERR_OK");
      //--- Nominal Bit Times ---
      Assert::AreEqual(  0u, Conf.NBRP, L"Test NBRP (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 0");
      Assert::AreEqual( 28u, Conf.NTSEG1, L"Test NTSEG1 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 28");
      Assert::AreEqual(  9u, Conf.NTSEG2, L"Test NTSEG2 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 9");
      Assert::AreEqual(  9u, Conf.NSJW, L"Test NSJW (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 9");
      //--- Data Bit Times ---
      Assert::AreEqual( 0u, Conf.DBRP, L"Test DBRP (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(13u, Conf.DTSEG1, L"Test DTSEG1 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 13");
      Assert::AreEqual( 4u, Conf.DTSEG2, L"Test DTSEG2 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 4");
      Assert::AreEqual( 4u, Conf.DSJW, L"Test DSJW (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 4");
      //--- Transmitter Delay Compensation ---
      Assert::AreEqual(MCP251XFD_AUTO_MODE, Conf.TDCMOD, L"Test TDCMOD (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be AUTO_MODE");
      Assert::AreEqual(                 15, Conf.TDCO, L"Test TDCO (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 15");
      Assert::AreEqual(                 0u, Conf.TDCV, L"Test TDCV (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 0");
      //--- Result Statistics ---
      Assert::AreEqual(1000000u, Stats.NominalBitrate, L"Test NominalBitrate (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 1Mbps");
      Assert::AreEqual(2000000u, Stats.DataBitrate, L"Test DataBitrate (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 2Mbps");
      Assert::AreEqual(      0u, Stats.MaxBusLength, L"Test MaxBusLength (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 0");
      Assert::AreEqual(   7500u, Stats.NSamplePoint, L"Test NSamplePoint (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 75.00%");
      Assert::AreEqual(   7500u, Stats.DSamplePoint, L"Test DSamplePoint (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 75.00%");
      Assert::AreEqual(    125u, Stats.OscTolC1, L"Test OscTolC1 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 1.25%");
      Assert::AreEqual(     98u, Stats.OscTolC2, L"Test OscTolC2 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 0.98%");
      Assert::AreEqual(    125u, Stats.OscTolC3, L"Test OscTolC3 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 1.25%");
      Assert::AreEqual(    126u, Stats.OscTolC4, L"Test OscTolC4 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 1.26%");
      Assert::AreEqual(    161u, Stats.OscTolC5, L"Test OscTolC5 (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 1.61%");
      Assert::AreEqual(     98u, Stats.OscTolerance, L"Test OscTolerance (SYSCLK_40MHz, NBR_1Mbps, DBR_2Mbps), should be 0.98%");
    }

  };
}