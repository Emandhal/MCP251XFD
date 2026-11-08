/*!*****************************************************************************
 * @file    UnitTestBitratesStandard.cpp
 * @brief   Unit tests of the bitrate calculation with the standard bitrates and all the possible SYSCLK
 * @details The nominal bitrates are 10, 20, 50, 100, 125, 250, 500, 800 kbit/s and 1 Mbit/s,
 *          the data bitrates are 500 kbit/s, 1, 2, 4, 5 and 8 Mbit/s, and the SYSCLK are all the
 *          values the MCP251XFD can have: from 2 to 40 MHz (2 to 40 MHz oscillator, 4 to 40 MHz
 *          crystal, with CLKIN x1, /2, x5 or x10 with the PLL) by steps of 0.5 MHz.
 *          Nothing is compared with the driver itself, the expected values come from
 *            - a table written by hand (ExpectedResults)
 *            - exact divisions (when SYSCLK = BRP x TQ x bitrate exists, the bitrates must be exact)
 *            - a brute force search of the possible configurations (the driver must find one if one exists,
 *              with the lowest bitrate error, and at most MCP251XFD_BITRATE_MAX_ERROR_PERMILLE of error)
 *            - the registers (limits of the component, prescalers, bitrates, sample points and TDC)
 *            - a port of the bit timing calculation of the Linux kernel (can_calc_bittiming): if Linux finds
 *              a configuration then the driver must find one with at most the same error
 *          Works with the 1.x driver (MCP251XFD_BitTimeConfig) and the 2.x driver (CAN_BitTimeConfig)
 ******************************************************************************/
#include "stdafx.h"
#include "CppUnitTest.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <string>
#include <sstream>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#ifndef CAN_NO_CANFD
#  define CAN_NO_CANFD  MCP251XFD_NO_CANFD
#endif

#ifdef CAN_COMMON_H_INC
#  define API_V2 1
#else
#  define API_V2 0
#endif


namespace UnitTest_BitratesStandard
{
  /*! @brief Result of a bit time calculation: one structure for both drivers
   *
   * All the values are real values (register + 1)
   */
  struct Res
  {
    eERRORRESULT err;
    uint32_t nBrp, n1, n2, nSjw;   // Nominal: prescaler, TSEG1, TSEG2, SJW
    uint32_t dBrp, d1, d2, dSjw;   // Data
    int      tdcMode;              // 0 = disabled, 1 = manual, 2 = automatic
    int32_t  tdco, tdcv;
    uint32_t nbr, dbr;             // Actual bitrates of the statistics
  };

  /*! @brief Calculate the bit time configuration with the driver
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] nom Is the wanted nominal bitrate in bit/s
   * @param[in] dat Is the wanted data bitrate in bit/s (CAN_NO_CANFD for CAN 2.0 only)
   * @param[in] defaultSp If 'true' the sample points are not set (0) to get the default sample points of the driver (2.x only, the 1.x driver has no sample point parameter), else they are 80%
   * @return Returns the error and the registers of the configuration
   */
  static Res Calc(uint32_t clk, uint32_t nom, uint32_t dat, bool defaultSp = false)
  {
    Res r; memset(&r, 0, sizeof r);
#if API_V2
    CAN_BitTimeConfig c; CAN_BitTimeStats t; CAN_CANFDbusConfig b;
    memset(&c, 0, sizeof c); memset(&t, 0, sizeof t); memset(&b, 0, sizeof b);
    b.DesiredNominalBitrate = nom; b.DesiredDataBitrate = dat;
    if (!defaultSp) { b.NominalSamplePoint = 80; b.DataSamplePoint = 80; }
    c.Stats = &t;
    r.err = MCP251XFD_CalculateBitTimeConfiguration(clk, b, &c);
    r.tdcMode = (c.TDCmode == CAN_TDC_AUTO_MODE ? 2 : c.TDCmode == CAN_TDC_MANUAL_MODE ? 1 : 0);
#else
    (void)defaultSp;
    MCP251XFD_BitTimeConfig c; MCP251XFD_BitTimeStats t;
    memset(&c, 0, sizeof c); memset(&t, 0, sizeof t);
    c.Stats = &t;
    r.err = MCP251XFD_CalculateBitTimeConfiguration(clk, nom, dat, &c);
    r.tdcMode = (c.TDCMOD == MCP251XFD_AUTO_MODE ? 2 : c.TDCMOD == MCP251XFD_MANUAL_MODE ? 1 : 0);
#endif
    r.nBrp = c.NBRP + 1u; r.n1 = c.NTSEG1 + 1u; r.n2 = c.NTSEG2 + 1u; r.nSjw = c.NSJW + 1u;
    r.dBrp = c.DBRP + 1u; r.d1 = c.DTSEG1 + 1u; r.d2 = c.DTSEG2 + 1u; r.dSjw = c.DSJW + 1u;
    r.tdco = (int32_t)c.TDCO; r.tdcv = (int32_t)c.TDCV;
    r.nbr = t.NominalBitrate; r.dbr = t.DataBitrate;
    return r;
  }

  //=============================================================================
  // Independent references
  //=============================================================================
  static const uint32_t NoErr = 0xFFFFFFFFu;   // No possible TQ count

  static uint32_t Absd(uint32_t a, uint32_t b) { return a > b ? a - b : b - a; }
  static uint32_t BrErr(uint32_t clk, uint32_t brp, uint32_t tq, uint32_t des) { return Absd(clk / (brp * tq), des); }

  /*! @brief Tell if a TQ count is possible: it is inside the limits and its bitrate error is at most MCP251XFD_BITRATE_MAX_ERROR_PERMILLE of the wanted bitrate (floor and ceil)
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] brp Is the prescaler
   * @param[in] tq Is the TQ count of the bit
   * @param[in] des Is the wanted bitrate in bit/s
   * @param[in] tqMin Is the minimum TQ count of the phase
   * @param[in] tqMax Is the maximum TQ count of the phase
   * @return Returns 'true' if the TQ count is possible
   */
  static bool TqAllowed(uint32_t clk, uint32_t brp, uint32_t tq, uint32_t des, uint32_t tqMin, uint32_t tqMax)
  {
    if ((tq < tqMin) || (tq > tqMax)) return false;
    return ((unsigned long long)BrErr(clk, brp, tq, des) * 1000ull) <= ((unsigned long long)des * MCP251XFD_BITRATE_MAX_ERROR_PERMILLE);
  }

  /*! @brief Get the lowest bitrate error of a phase with a prescaler
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] brp Is the prescaler
   * @param[in] des Is the wanted bitrate in bit/s
   * @param[in] tqMin Is the minimum TQ count of the phase
   * @param[in] tqMax Is the maximum TQ count of the phase
   * @return Returns the error in bit/s, or NoErr if no TQ count is possible
   */
  static uint32_t PhaseErr(uint32_t clk, uint32_t brp, uint32_t des, uint32_t tqMin, uint32_t tqMax)
  {
    uint32_t best = NoErr;
    const uint32_t tq = clk / (des * brp);
    for (uint32_t k = 0; k < 2u; k++)
      if (TqAllowed(clk, brp, tq + k, des, tqMin, tqMax)) { const uint32_t e = BrErr(clk, brp, tq + k, des); if (e < best) best = e; }
    return best;
  }

  /*! @brief Get the lowest sum of the bitrate errors with a common prescaler and with a prescaler for each phase
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] nom Is the wanted nominal bitrate in bit/s
   * @param[in] dat Is the wanted data bitrate in bit/s (CAN_NO_CANFD for CAN 2.0 only)
   * @param[out] eCommon Receives the lowest sum of the errors with a common prescaler (NoErr if impossible)
   * @param[out] eIndep Receives the lowest sum of the errors with a prescaler for each phase (NoErr if impossible)
   */
  static void MinErrors(uint32_t clk, uint32_t nom, uint32_t dat, uint32_t& eCommon, uint32_t& eIndep)
  {
    uint32_t minN = NoErr, minD = NoErr; eCommon = NoErr;
    for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
    {
      const uint32_t en = PhaseErr(clk, brp, nom, MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX);
      const uint32_t ed = (dat == CAN_NO_CANFD) ? 0u : PhaseErr(clk, brp, dat, MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX);
      if (en < minN) minN = en;
      if ((dat != CAN_NO_CANFD) && (ed < minD)) minD = ed;
      if ((en != NoErr) && (ed != NoErr) && (en + ed < eCommon)) eCommon = en + ed;
    }
    if (dat == CAN_NO_CANFD) { eIndep = eCommon; return; }
    eIndep = ((minN == NoErr) || (minD == NoErr)) ? NoErr : minN + minD;
  }

  /*! @brief Tell if the driver must find a configuration: a prescaler for the nominal phase and a prescaler for the data phase (a prescaler for each phase is used if no common one is as good)
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] nom Is the wanted nominal bitrate in bit/s
   * @param[in] dat Is the wanted data bitrate in bit/s (CAN_NO_CANFD for CAN 2.0 only)
   * @return Returns 'true' if a configuration exists
   */
  static bool Feasible(uint32_t clk, uint32_t nom, uint32_t dat)
  {
    uint32_t eCommon, eIndep; MinErrors(clk, nom, dat, eCommon, eIndep);
    return (eCommon != NoErr) || (eIndep != NoErr);
  }

  /*! @brief Tell if a bitrate can be exact: clk = BRP x TQ x bitrate
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] bitrate Is the wanted bitrate in bit/s
   * @param[in] tqMin Is the minimum TQ count of the phase
   * @param[in] tqMax Is the maximum TQ count of the phase
   * @return Returns 'true' if a prescaler and a TQ count give exactly the bitrate
   */
  static bool ExactPossible(uint32_t clk, uint32_t bitrate, uint32_t tqMin, uint32_t tqMax)
  {
    for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
    {
      if ((clk % (brp * bitrate)) != 0) continue;
      const uint32_t tq = clk / (brp * bitrate);
      if ((tq >= tqMin) && (tq <= tqMax)) return true;
    }
    return false;
  }

  //=============================================================================
  // Port of the bit timing calculation of the Linux kernel (can_calc_bittiming() and can_update_sample_point() of drivers/net/can/dev/calc_bittiming.c)
  //=============================================================================
  struct LinuxLimits { unsigned t1min, t1max, t2min, t2max, brpmin, brpmax; };
  static const LinuxLimits LinuxNominal = { 2u, 256u, 1u, 128u, 1u, 256u };   // MCP251XFD nominal phase: TSEG1 2..256, TSEG2 1..128, BRP 1..256
  static const LinuxLimits LinuxData    = { 1u,  32u, 1u,  16u, 1u, 256u };   // MCP251XFD data phase: TSEG1 1..32, TSEG2 1..16, BRP 1..256
  struct LinuxRes { bool ok; unsigned brp, bitrate; unsigned long long errPpm; };

  static unsigned LinuxClamp(unsigned v, unsigned lo, unsigned hi) { return v < lo ? lo : (v > hi ? hi : v); }
  static unsigned LinuxRefSamplePoint(unsigned bitrate) { return (bitrate > 800000u) ? 750u : ((bitrate > 500000u) ? 800u : 875u); }

  static unsigned LinuxUpdateSamplePoint(const LinuxLimits& b, unsigned ref, unsigned tseg, unsigned* t1p, unsigned* t2p, unsigned* errp)
  {
    unsigned bestErr = UINT_MAX, bestSp = 0;
    for (int i = 0; i <= 1; i++)
    {
      unsigned tseg2 = tseg + 1u - (ref * (tseg + 1u)) / 1000u - (unsigned)i;
      tseg2 = LinuxClamp(tseg2, b.t2min, b.t2max);
      unsigned tseg1 = tseg - tseg2;
      if (tseg1 > b.t1max) { tseg1 = b.t1max; tseg2 = tseg - tseg1; }
      const unsigned sp = 1000u * (tseg + 1u - tseg2) / (tseg + 1u);
      const unsigned e = (unsigned)abs((int)ref - (int)sp);
      if ((sp <= ref) && (e < bestErr)) { bestSp = sp; bestErr = e; *t1p = tseg1; *t2p = tseg2; }
    }
    if (errp) *errp = bestErr;
    return bestSp;
  }

  static LinuxRes LinuxCalc(unsigned clk, unsigned bitrate, const LinuxLimits& b)
  {
    LinuxRes r; memset(&r, 0, sizeof r);
    const unsigned ref = LinuxRefSamplePoint(bitrate);
    unsigned bestErr = UINT_MAX, bestSpErr = UINT_MAX, bestTseg = 0, bestBrp = 0, t1 = 0, t2 = 0, spErr;
    for (unsigned tseg = (b.t1max + b.t2max) * 2u + 1u; tseg >= (b.t1min + b.t2min) * 2u; tseg--)
    {
      const unsigned tsegall = 1u + tseg / 2u;
      unsigned brp = clk / (tsegall * bitrate) + tseg % 2u;
      if ((brp < b.brpmin) || (brp > b.brpmax)) continue;
      const unsigned br = clk / (brp * tsegall);
      const unsigned be = (unsigned)abs((int)bitrate - (int)br);
      if (be > bestErr) continue;
      if (be < bestErr) bestSpErr = UINT_MAX;
      LinuxUpdateSamplePoint(b, ref, tseg / 2u, &t1, &t2, &spErr);
      if (spErr >= bestSpErr) continue;
      bestSpErr = spErr; bestErr = be; bestTseg = tseg / 2u; bestBrp = brp;
      if ((be == 0) && (spErr == 0)) break;
    }
    if (bestBrp == 0) return r;
    if (bestErr != 0) { const unsigned long long e = (unsigned long long)bestErr * 10000ull / bitrate; if (e > 500ull) return r; }  // 5% maximum
    LinuxUpdateSamplePoint(b, ref, bestTseg, &t1, &t2, NULL);
    r.brp = bestBrp; r.ok = (t1 >= b.t1min) && (t2 >= b.t2min);
    r.bitrate = clk / (r.brp * (1u + t1 + t2));
    r.errPpm = (unsigned long long)Absd(bitrate, r.bitrate) * 1000000ull / bitrate;
    return r;
  }

  //=============================================================================
  // Checks of one case
  //=============================================================================
  static std::wstring Name(uint32_t clk, uint32_t nom, uint32_t dat, const wchar_t* what, uint32_t v1 = 0, uint32_t v2 = 0)
  {
    std::wostringstream os;
    os << L"SYSCLK " << clk / 1000u << L"kHz, nominal " << nom << L"bit/s, data " << dat << L"bit/s: " << what;
    if ((v1 != 0) || (v2 != 0)) os << L" (" << v1 << L", " << v2 << L")";
    return os.str();
  }

  /*! @brief Check that a result is a valid configuration, and that it is coherent with the wanted bitrates
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] nom Is the wanted nominal bitrate in bit/s
   * @param[in] dat Is the wanted data bitrate in bit/s (CAN_NO_CANFD for CAN 2.0 only)
   * @param[in] r Is the result of the driver
   */
  static void CheckOk(uint32_t clk, uint32_t nom, uint32_t dat, const Res& r)
  {
    const bool fd = (dat != CAN_NO_CANFD);
    const uint32_t nTq = MCP251XFD_NSYNC + r.n1 + r.n2, dTq = MCP251XFD_DSYNC + r.d1 + r.d2;

    //--- Registers inside the limits of the component
    Assert::IsTrue((r.nBrp >= MCP251XFD_NBRP_MIN) && (r.nBrp <= MCP251XFD_NBRP_MAX), Name(clk, nom, dat, L"NBRP out of limits").c_str());
    Assert::IsTrue((r.n1 >= MCP251XFD_NTSEG1_MIN) && (r.n1 <= MCP251XFD_NTSEG1_MAX), Name(clk, nom, dat, L"NTSEG1 out of limits").c_str());
    Assert::IsTrue((r.n2 >= MCP251XFD_NTSEG2_MIN) && (r.n2 <= MCP251XFD_NTSEG2_MAX), Name(clk, nom, dat, L"NTSEG2 out of limits").c_str());
    Assert::IsTrue((r.nSjw >= MCP251XFD_NSJW_MIN) && (r.nSjw <= MCP251XFD_NSJW_MAX) && (r.nSjw <= r.n1) && (r.nSjw <= r.n2), Name(clk, nom, dat, L"NSJW out of limits").c_str());
    Assert::IsTrue((nTq >= MCP251XFD_NTQBIT_MIN) && (nTq <= MCP251XFD_NTQBIT_MAX), Name(clk, nom, dat, L"nominal TQ count out of limits").c_str());
    if (fd)
    {
      Assert::IsTrue((r.dBrp >= MCP251XFD_DBRP_MIN) && (r.dBrp <= MCP251XFD_DBRP_MAX), Name(clk, nom, dat, L"DBRP out of limits").c_str());
      Assert::IsTrue((r.d1 >= MCP251XFD_DTSEG1_MIN) && (r.d1 <= MCP251XFD_DTSEG1_MAX), Name(clk, nom, dat, L"DTSEG1 out of limits").c_str());
      Assert::IsTrue((r.d2 >= MCP251XFD_DTSEG2_MIN) && (r.d2 <= MCP251XFD_DTSEG2_MAX), Name(clk, nom, dat, L"DTSEG2 out of limits").c_str());
      Assert::IsTrue((r.dSjw >= MCP251XFD_DSJW_MIN) && (r.dSjw <= MCP251XFD_DSJW_MAX) && (r.dSjw <= r.d1) && (r.dSjw <= r.d2), Name(clk, nom, dat, L"DSJW out of limits").c_str());
      Assert::IsTrue((dTq >= MCP251XFD_DTQBIT_MIN) && (dTq <= MCP251XFD_DTQBIT_MAX), Name(clk, nom, dat, L"data TQ count out of limits").c_str());
    }

    //--- Bitrates of the registers = bitrates of the statistics
    const uint32_t nbr = clk / (r.nBrp * nTq), dbr = fd ? clk / (r.dBrp * dTq) : 0u;
    Assert::IsTrue(r.nbr == nbr, Name(clk, nom, dat, L"nominal bitrate of the statistics differs from the registers").c_str());
    Assert::IsTrue(r.dbr == dbr, Name(clk, nom, dat, L"data bitrate of the statistics differs from the registers").c_str());

    //--- Bitrate error: at most MCP251XFD_BITRATE_MAX_ERROR_PERMILLE (5%, like the Linux kernel) of the wanted bitrate for each phase
    Assert::IsTrue(((unsigned long long)Absd(nbr, nom) * 1000ull) <= ((unsigned long long)nom * MCP251XFD_BITRATE_MAX_ERROR_PERMILLE), Name(clk, nom, dat, L"nominal bitrate error too high", nbr, 0).c_str());
    if (fd) Assert::IsTrue(((unsigned long long)Absd(dbr, dat) * 1000ull) <= ((unsigned long long)dat * MCP251XFD_BITRATE_MAX_ERROR_PERMILLE), Name(clk, nom, dat, L"data bitrate error too high", dbr, 0).c_str());

    //--- The sum of the bitrate errors is the lowest possible, with a common prescaler or with a prescaler for each phase
    uint32_t eCommon, eIndep; MinErrors(clk, nom, dat, eCommon, eIndep);
    const uint32_t eMin = (eIndep < eCommon) ? eIndep : eCommon;
    const uint32_t eDrv = Absd(nbr, nom) + (fd ? Absd(dbr, dat) : 0u);
    Assert::IsTrue(eDrv == eMin, Name(clk, nom, dat, L"the sum of the bitrate errors is not the lowest possible", eDrv, eMin).c_str());

    //--- Same prescaler for both phases (identical TQ avoids the quantization errors at the bit rate switching), except if a prescaler for each phase gives a lower bitrate error
    //    or, with the same bitrate error, a nearer sample point (the sweep checks the sample point choice)
    if (fd) Assert::IsTrue((r.nBrp == r.dBrp) || (eIndep <= eCommon), Name(clk, nom, dat, L"different prescalers but a common prescaler gives a lower bitrate error", r.nBrp, r.dBrp).c_str());

    //--- TDC (ISO 11898-1 section 11.3.3, like the Linux kernel): automatic mode only if the data prescaler is 1 or 2, the offset is the data sample point in SYSCLK periods (max 63), the value is measured by the component
    if (fd)
    {
      const bool automatic = (r.dBrp <= 2u);
      const uint32_t ssp = r.dBrp * (MCP251XFD_DSYNC + r.d1);
      const int32_t tdco = automatic ? (int32_t)(ssp > (uint32_t)MCP251XFD_TDCO_MAX ? (uint32_t)MCP251XFD_TDCO_MAX : ssp) : 0;
      Assert::IsTrue(r.tdcMode == (automatic ? 2 : 0), Name(clk, nom, dat, L"TDC mode (automatic only if DBRP is 1 or 2)", r.dBrp, (uint32_t)r.tdcMode).c_str());
      Assert::IsTrue(r.tdco == tdco, Name(clk, nom, dat, L"TDCO", (uint32_t)r.tdco, (uint32_t)tdco).c_str());
      Assert::IsTrue(r.tdcv == 0, Name(clk, nom, dat, L"TDCV must be 0 (measured by the component)").c_str());
    }

    //--- Sample points: the driver aims at 80% (2.x) or 75% to 87.5% (1.x); a small TQ count limits the choice, and so does a big one:
    //    the nominal TSEG1 is 256 at most and TSEG2 128 at most, then the sample point of 385 TQ (the maximum) is 66.7% (the bitrate error has the priority on the sample point).
    //    The upper bound is the highest target (87.5%) plus one TQ, the resolution of the sample point
    const uint32_t nsp = ((MCP251XFD_NSYNC + r.n1) * 1000u) / nTq;
    Assert::IsTrue((nsp >= 660u) && (nsp <= 875u + 1000u / nTq + 1u), Name(clk, nom, dat, L"nominal sample point out of 66%..87.5% (+1 TQ)", nsp, 0).c_str());
    if (fd)
    {
      const uint32_t dsp = ((MCP251XFD_DSYNC + r.d1) * 1000u) / dTq;
      Assert::IsTrue((dsp >= 660u) && (dsp <= 875u + 1000u / dTq + 1u), Name(clk, nom, dat, L"data sample point out of 66%..87.5% (+1 TQ)", dsp, 0).c_str());
    }

    //--- If the bitrates can be exact then they must be exact (a prescaler for each phase is used if there is no common one)
    if (ExactPossible(clk, nom, MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX) && (!fd || ExactPossible(clk, dat, MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX)))
    {
      Assert::IsTrue(nbr == nom, Name(clk, nom, dat, L"the nominal bitrate can be exact", nbr, 0).c_str());
      if (fd) Assert::IsTrue(dbr == dat, Name(clk, nom, dat, L"the data bitrate can be exact", dbr, 0).c_str());
    }
  }

  /*! @brief Check one case of the grid: the driver must refuse the bitrates outside of its limits, find a configuration if one exists, and give a valid one
   *
   * @param[in] clk Is the SYSCLK in Hz
   * @param[in] nom Is the wanted nominal bitrate in bit/s
   * @param[in] dat Is the wanted data bitrate in bit/s (CAN_NO_CANFD for CAN 2.0 only)
   */
  static void CheckCase(uint32_t clk, uint32_t nom, uint32_t dat)
  {
    const Res r = Calc(clk, nom, dat);
    if ((nom < MCP251XFD_NOMBITRATE_MIN) || (nom > MCP251XFD_NOMBITRATE_MAX))
    {
      Assert::IsTrue(r.err == ERR__BAUDRATE_ERROR, Name(clk, nom, dat, L"a nominal bitrate out of the driver limits must give ERR__BAUDRATE_ERROR").c_str());
      return;
    }
    if ((dat != CAN_NO_CANFD) && ((dat < MCP251XFD_DATABITRATE_MIN) || (dat > MCP251XFD_DATABITRATE_MAX)))
    {
      Assert::IsTrue(r.err == ERR__BAUDRATE_ERROR, Name(clk, nom, dat, L"a data bitrate out of the driver limits must give ERR__BAUDRATE_ERROR").c_str());
      return;
    }
    if (Feasible(clk, nom, dat))
    {
      Assert::IsTrue(r.err == ERR_OK, Name(clk, nom, dat, L"a configuration exists but the driver gave an error").c_str());
      if (r.err == ERR_OK) CheckOk(clk, nom, dat, r);
    }
    else
      Assert::IsTrue(r.err == ERR__BITTIME_ERROR, Name(clk, nom, dat, L"no configuration exists, it should give ERR__BITTIME_ERROR").c_str());
  }

  /*! @brief All the SYSCLK that the component can have: 2 MHz to 40 MHz by steps of 0.5 MHz.
   *
   * CLKIN x1 (2 to 40 MHz oscillator, 4 to 40 MHz crystal), CLKIN /2, CLKIN x10 (PLL) and CLKIN x5 (PLL, then /2) stay inside this range.
   */
  static std::vector<uint32_t> AllClocks()
  {
    std::vector<uint32_t> v;
    for (uint32_t khz = 2000u; khz <= 40000u; khz += 500u) v.push_back(khz * 1000u);
    return v;
  }
  static const uint32_t NominalBr[]   = { 10000u, 20000u, 50000u, 100000u, 125000u, 250000u, 500000u, 800000u, 1000000u };            // Standard nominal bitrates
  static const uint32_t DataBr[]      = { 500000u, 1000000u, 2000000u, 4000000u, 5000000u, 8000000u };                                // Standard data bitrates


  TEST_CLASS(UnitTest_BitratesStandard)
  {
  public:

    /*! @brief Values worked out by hand (the quotient SYSCLK / bitrate is the TQ count x the prescaler) */
    TEST_METHOD(TestMethod_Bitrates_ExpectedResults)
    {
      struct Row { uint32_t clk, nom, dat; bool ok; uint32_t nbr, dbr; };
      static const Row ExpectedResults[] =
      {
        // CAN 2.0 only
        {  2000000u,   10000u, 0u,       true,   10000u, 0u },      // 2MHz / 10k    = 200 TQ
        {  2000000u,   20000u, 0u,       true,   20000u, 0u },      // 2MHz / 20k    = 100 TQ
        {  2000000u,   50000u, 0u,       true,   50000u, 0u },      // 2MHz / 50k    =  40 TQ
        {  2000000u,  100000u, 0u,       true,  100000u, 0u },      // 2MHz / 100k   =  20 TQ
        {  2000000u,  125000u, 0u,       true,  125000u, 0u },      // 2MHz / 125k   =  16 TQ
        {  2000000u,  250000u, 0u,       true,  250000u, 0u },      // 2MHz / 250k   =   8 TQ
        {  2000000u,  450000u, 0u,       false,      0u, 0u },      // 2MHz / 450k   =  4.4 TQ, 4 TQ is 11% too fast and 5 TQ is 11% too slow: more than the 5% maximum
        {  2000000u,  500000u, 0u,       true,  500000u, 0u },      // 2MHz / 500k   =   4 TQ (the minimum)
        {  2000000u,  800000u, 0u,       false,      0u, 0u },      // 2MHz / 800k   =  2.5 TQ, less than the 4 TQ minimum
        {  2000000u, 1000000u, 0u,       false,      0u, 0u },      // 2MHz / 1M     =   2 TQ, less than the 4 TQ minimum
        {  4000000u,   10000u, 0u,       true,   10000u, 0u },      // 4MHz / 10k    = 400 TQ, more than 385: prescaler 2 and 200 TQ
        {  4000000u,  125000u, 0u,       true,  125000u, 0u },      // 4MHz / 125k   =  32 TQ
        {  4000000u,  800000u, 0u,       true,  800000u, 0u },      // 4MHz / 800k   =   5 TQ
        {  4000000u, 1000000u, 0u,       true, 1000000u, 0u },      // 4MHz / 1M     =   4 TQ
        { 10000000u,   10000u, 0u,       true,   10000u, 0u },      // 10MHz / 10k   = 1000 = 4 x 250 TQ
        { 10000000u,  125000u, 0u,       true,  125000u, 0u },      // 10MHz / 125k  =  80 TQ
        { 10000000u,  250000u, 0u,       true,  250000u, 0u },      // 10MHz / 250k  =  40 TQ
        { 10000000u,  500000u, 0u,       true,  500000u, 0u },      // 10MHz / 500k  =  20 TQ
        { 10000000u, 1000000u, 0u,       true, 1000000u, 0u },      // 10MHz / 1M    =  10 TQ
        { 20000000u,   10000u, 0u,       true,   10000u, 0u },      // 20MHz / 10k   = 2000 = 8 x 250 TQ
        { 20000000u,  800000u, 0u,       true,  800000u, 0u },      // 20MHz / 800k  =  25 TQ
        { 20000000u, 1000000u, 0u,       true, 1000000u, 0u },      // 20MHz / 1M    =  20 TQ
        { 40000000u,   10000u, 0u,       true,   10000u, 0u },      // 40MHz / 10k   = 4000 = 16 x 250 TQ
        { 40000000u,   20000u, 0u,       true,   20000u, 0u },      // 40MHz / 20k   = 2000 =  8 x 250 TQ
        { 40000000u,   50000u, 0u,       true,   50000u, 0u },      // 40MHz / 50k   =  800 =  4 x 200 TQ
        { 40000000u,  100000u, 0u,       true,  100000u, 0u },      // 40MHz / 100k  =  400 =  2 x 200 TQ
        { 40000000u,  125000u, 0u,       true,  125000u, 0u },      // 40MHz / 125k  = 320 TQ
        { 40000000u,  800000u, 0u,       true,  800000u, 0u },      // 40MHz / 800k  =  50 TQ
        { 40000000u, 1000000u, 0u,       true, 1000000u, 0u },      // 40MHz / 1M    =  40 TQ
        { 39500000u, 1000000u, 0u,       true,  987500u, 0u },      // 39.5MHz / 1M  = 39.5 TQ, 40 TQ gives 987500 (1.25% too slow) and 39 TQ gives 1012820 (1.28% too fast)
        // CAN FD
        {  2000000u,   10000u,  500000u, true,   10000u,  500000u },  // 2MHz: 200 TQ, 4 TQ
        {  4000000u,  500000u,  500000u, true,  500000u,  500000u },  // 4MHz: 8 TQ for both
        {  4000000u,  500000u, 1000000u, true,  500000u, 1000000u },  // 4MHz: 8 TQ, 4 TQ
        {  4000000u,  500000u, 2000000u, false,      0u,       0u },  // 4MHz / 2M = 2 TQ, less than the 3 TQ minimum of the data phase
        { 10000000u,  500000u, 5000000u, false,      0u,       0u },  // 10MHz / 5M = 2 TQ
        { 20000000u,  500000u, 5000000u, true,  500000u, 5000000u },  // 20MHz: 40 TQ, 4 TQ
        { 20000000u,  100000u, 8000000u, false,      0u,       0u },  // 20MHz / 8M = 2.5 TQ and the closest, 3 TQ, is 17% too slow
        { 20000000u, 1000000u, 8000000u, false,      0u,       0u },  // 20MHz / 8M = 2.5 TQ and the closest, 3 TQ, is 17% too slow
        { 40000000u,   10000u,  500000u, true,   10000u,  500000u },  // 40MHz: 16 x 250 TQ, 16 x 5 TQ
        { 40000000u,  100000u, 2000000u, true,  100000u, 2000000u },  // 40MHz: 2 x 200 TQ, 2 x 10 TQ
        { 40000000u,  100000u, 5000000u, true,  100000u, 5000000u },  // 40MHz: 2 x 200 TQ, 2 x 4 TQ
        { 40000000u,  500000u, 2000000u, true,  500000u, 2000000u },  // 40MHz: 80 TQ, 20 TQ
        { 40000000u,  500000u, 8000000u, true,  500000u, 8000000u },  // 40MHz: 80 TQ, 5 TQ
        { 40000000u, 1000000u, 4000000u, true, 1000000u, 4000000u },  // 40MHz: 40 TQ, 10 TQ
        // CAN FD where the two phases can not share the prescaler: 8 Mbit/s needs a prescaler of 1 (5 TQ) and the nominal phase a bigger prescaler
        { 40000000u,   10000u, 8000000u, true,   10000u, 8000000u },  // 40MHz: 16 x 250 TQ and 1 x 5 TQ
        { 40000000u,   50000u, 8000000u, true,   50000u, 8000000u },  // 40MHz:  4 x 200 TQ and 1 x 5 TQ
        { 40000000u,  100000u, 8000000u, true,  100000u, 8000000u },  // 40MHz:  2 x 200 TQ and 1 x 5 TQ
      };
      for (size_t z = 0; z < sizeof(ExpectedResults) / sizeof(ExpectedResults[0]); z++)
      {
        const Row& e = ExpectedResults[z];
        const Res r = Calc(e.clk, e.nom, e.dat);
        if (!e.ok)
        {
          Assert::IsTrue(r.err == ERR__BITTIME_ERROR, Name(e.clk, e.nom, e.dat, L"should give ERR__BITTIME_ERROR").c_str());
          continue;
        }
        Assert::IsTrue(r.err == ERR_OK, Name(e.clk, e.nom, e.dat, L"should be ERR_OK").c_str());
        if (r.err != ERR_OK) continue;
        Assert::IsTrue(r.nbr == e.nbr, Name(e.clk, e.nom, e.dat, L"wrong actual nominal bitrate", r.nbr, e.nbr).c_str());
        Assert::IsTrue(r.dbr == e.dbr, Name(e.clk, e.nom, e.dat, L"wrong actual data bitrate", r.dbr, e.dbr).c_str());
        CheckOk(e.clk, e.nom, e.dat, r);
      }
    }

    /*! @brief Prescalers: the two phases use the same prescaler if it is possible, else a prescaler for each phase (ex: 8 Mbit/s needs a prescaler of 1 with a 40 MHz SYSCLK, or a nearer sample point) */
    TEST_METHOD(TestMethod_Bitrates_Prescalers)
    {
      // 40MHz / 8M = 5 TQ x prescaler 1 only, the nominal 100k needs a prescaler of at least 2 (400 TQ at prescaler 1 is more than 385)
      Res r = Calc(40000000u, 100000u, 8000000u);
      Assert::IsTrue(r.err == ERR_OK, L"40MHz 100k/8M should be ERR_OK");
      Assert::IsTrue(r.dBrp == 1u, L"40MHz 100k/8M: the data prescaler must be 1");
      Assert::IsTrue(r.nBrp >= 2u, L"40MHz 100k/8M: the nominal prescaler must be 2 or more (400 TQ at prescaler 1 is more than 385)");
      // 40MHz 500k/2M: BRP 1 gives 80 TQ and 20 TQ, both exact, a common prescaler exists then it is used
      r = Calc(40000000u, 500000u, 2000000u);
      Assert::IsTrue(r.err == ERR_OK, L"40MHz 500k/2M should be ERR_OK");
      Assert::IsTrue(r.nBrp == r.dBrp, L"40MHz 500k/2M: a common prescaler exists, it must be used");
      // 4MHz 500k/1M: BRP 1 only (4 TQ for the data phase)
      r = Calc(4000000u, 500000u, 1000000u);
      Assert::IsTrue(r.err == ERR_OK, L"4MHz 500k/1M should be ERR_OK");
      Assert::IsTrue((r.nBrp == 1u) && (r.dBrp == 1u), L"4MHz 500k/1M: prescalers must be 1");
      // 38.5MHz 100k/1M: the prescaler 1 gives 385 TQ (exact) for the nominal phase but the sample point of 385 TQ is 66.7% at best (NTSEG1 is 256 at most),
      // and the data phase needs the prescaler 1 (39 TQ). The prescaler 5 gives 77 TQ (exact, sample point 87%) for the nominal phase: same bitrate errors, nearer sample point, then a prescaler per phase
      r = Calc(38500000u, 100000u, 1000000u, true);
      Assert::IsTrue(r.err == ERR_OK, L"38.5MHz 100k/1M should be ERR_OK");
      Assert::IsTrue(r.nbr == 100000u, L"38.5MHz 100k/1M: the nominal bitrate must be exact");
      Assert::IsTrue(r.nBrp != r.dBrp, L"38.5MHz 100k/1M: a prescaler for each phase gives a nearer nominal sample point");
      Assert::IsTrue((((MCP251XFD_NSYNC + r.n1) * 1000u) / (MCP251XFD_NSYNC + r.n1 + r.n2)) >= 850u, L"38.5MHz 100k/1M: the nominal sample point must be near 87.5% (not 66.7%)");
      // 40MHz 10k: 4000 = prescaler x TQ and 385 TQ at most, so the prescaler is 11 or more
      r = Calc(40000000u, 10000u, CAN_NO_CANFD);
      Assert::IsTrue(r.err == ERR_OK, L"40MHz 10k should be ERR_OK");
      Assert::IsTrue(r.nBrp >= 11u, L"40MHz 10k: the prescaler must be at least 11 (4000 / 385)");
    }

    /*! @brief TDC: automatic only with a data prescaler of 1 or 2 (offset = data sample point in SYSCLK periods, max 63), disabled otherwise (like the Linux kernel) */
    TEST_METHOD(TestMethod_Bitrates_TdcPolicy)
    {
      // Data prescaler 1: automatic
      Res r = Calc(40000000u, 100000u, 8000000u);
      Assert::IsTrue(r.err == ERR_OK, L"40MHz 100k/8M should be ERR_OK");
      Assert::IsTrue((r.dBrp == 1u) && (r.tdcMode == 2), L"DBRP = 1 must give an automatic TDC");
      Assert::IsTrue(r.tdco == (int32_t)(MCP251XFD_DSYNC + r.d1), L"TDCO must be the data sample point in SYSCLK periods");
      Assert::IsTrue(r.tdcv == 0, L"TDCV must be 0");
      // 4MHz 500k/1M: data prescaler 1 (4 TQ)
      r = Calc(4000000u, 500000u, 1000000u);
      Assert::IsTrue(r.err == ERR_OK, L"4MHz 500k/1M should be ERR_OK");
      Assert::IsTrue((r.dBrp == 1u) && (r.tdcMode == 2) && (r.tdco == (int32_t)(MCP251XFD_DSYNC + r.d1)), L"4MHz 500k/1M: automatic TDC with TDCO = 1 x (1 + DTSEG1)");
      // 40MHz 10k/500k: the data prescaler is 16 or 20 (the only common prescalers that give exact bitrates) then the TDC is disabled
      r = Calc(40000000u, 10000u, 500000u);
      Assert::IsTrue(r.err == ERR_OK, L"40MHz 10k/500k should be ERR_OK");
      Assert::IsTrue(r.dBrp > 2u, L"40MHz 10k/500k: the data prescaler must be more than 2");
      Assert::IsTrue((r.tdcMode == 0) && (r.tdco == 0) && (r.tdcv == 0), L"DBRP > 2 must give a disabled TDC");
      // CAN 2.0 only: no data phase
      r = Calc(40000000u, 500000u, CAN_NO_CANFD);
      Assert::IsTrue(r.err == ERR_OK, L"40MHz 500k should be ERR_OK");
    }

    /*! @brief CAN 2.0: all the standard nominal bitrates for all the SYSCLK */
    TEST_METHOD(TestMethod_Bitrates_Standard_CAN20)
    {
      const std::vector<uint32_t> clocks = AllClocks();
      for (size_t c = 0; c < clocks.size(); c++)
        for (size_t n = 0; n < sizeof(NominalBr) / sizeof(NominalBr[0]); n++)
          CheckCase(clocks[c], NominalBr[n], CAN_NO_CANFD);
    }

    /*! @brief CAN FD: all the standard nominal bitrates with all the standard data bitrates (data >= nominal) for all the SYSCLK */
    TEST_METHOD(TestMethod_Bitrates_Standard_CANFD)
    {
      const std::vector<uint32_t> clocks = AllClocks();
      for (size_t c = 0; c < clocks.size(); c++)
        for (size_t n = 0; n < sizeof(NominalBr) / sizeof(NominalBr[0]); n++)
          for (size_t d = 0; d < sizeof(DataBr) / sizeof(DataBr[0]); d++)
          {
            if (DataBr[d] < NominalBr[n]) continue; // The data bitrate is never slower than the nominal one
            CheckCase(clocks[c], NominalBr[n], DataBr[d]);
          }
    }

    /*! @brief Same result at each call */
    TEST_METHOD(TestMethod_Bitrates_Standard_SameResultTwice)
    {
      const std::vector<uint32_t> clocks = AllClocks();
      for (size_t c = 0; c < clocks.size(); c += 3)
        for (size_t n = 0; n < sizeof(NominalBr) / sizeof(NominalBr[0]); n++)
          for (size_t d = 0; d < sizeof(DataBr) / sizeof(DataBr[0]); d++)
          {
            const Res a = Calc(clocks[c], NominalBr[n], DataBr[d]), b = Calc(clocks[c], NominalBr[n], DataBr[d]);
            Assert::IsTrue(memcmp(&a, &b, sizeof a) == 0, Name(clocks[c], NominalBr[n], DataBr[d], L"the result must not change from one call to the next").c_str());
          }
    }

    /*! @brief Comparison with the bit timing calculation of the Linux kernel (each phase has its own prescaler in Linux):
     *
     * if Linux finds a configuration then the driver must find one, with at most the same bitrate errors
     */
    TEST_METHOD(TestMethod_Bitrates_Standard_LinuxComparison)
    {
      const std::vector<uint32_t> clocks = AllClocks();
      for (size_t c = 0; c < clocks.size(); c++)
        for (size_t n = 0; n < sizeof(NominalBr) / sizeof(NominalBr[0]); n++)
          for (size_t d = 0; d < 1u + sizeof(DataBr) / sizeof(DataBr[0]); d++)
          {
            const uint32_t clk = clocks[c], nom = NominalBr[n], dat = (d == 0) ? (uint32_t)CAN_NO_CANFD : DataBr[d - 1u];
            if ((dat != CAN_NO_CANFD) && (dat < nom)) continue;
            const LinuxRes ln = LinuxCalc(clk, nom, LinuxNominal);
            const LinuxRes ld = (dat != CAN_NO_CANFD) ? LinuxCalc(clk, dat, LinuxData) : LinuxRes();
            if (!ln.ok || ((dat != CAN_NO_CANFD) && !ld.ok)) continue;                                       // Linux finds nothing
            const Res r = Calc(clk, nom, dat);
            Assert::IsTrue(r.err == ERR_OK, Name(clk, nom, dat, L"Linux finds a configuration but the driver gave an error", (uint32_t)r.err, 0).c_str());
            if (r.err != ERR_OK) continue;
            const unsigned long long eDrv = (unsigned long long)Absd(r.nbr, nom) * 1000000ull / nom + ((dat != CAN_NO_CANFD) ? (unsigned long long)Absd(r.dbr, dat) * 1000000ull / dat : 0ull);
            const unsigned long long eLnx = ln.errPpm + ld.errPpm;
            Assert::IsTrue(eDrv <= eLnx, Name(clk, nom, dat, L"the bitrate error of the driver is higher than Linux (ppm)", (uint32_t)eDrv, (uint32_t)eLnx).c_str());
          }
    }

#if API_V2
    /*! @brief Sample points not set (0): the driver takes the targets of the CiA 601-3 and of the Linux kernel: 87.5% up to 500 kbit/s, 80% up to 800 kbit/s, 75% above */
    TEST_METHOD(TestMethod_Bitrates_DefaultSamplePoints)
    {
      struct Row { uint32_t clk, nom, dat, nomSp, dataSp; };
      static const Row Rows[] =
      {
        { 40000000u,  125000u,  500000u, 875u, 875u },
        { 40000000u,  250000u, 1000000u, 875u, 750u },
        { 40000000u,  500000u, 2000000u, 875u, 750u },
        { 40000000u,  800000u, 4000000u, 800u, 750u },
        { 40000000u, 1000000u, 5000000u, 750u, 750u },
        { 20000000u,  500000u, 2000000u, 875u, 750u },
        { 20000000u,  800000u, 4000000u, 800u, 750u },
        { 10000000u,  100000u,  500000u, 875u, 875u },
      };
      for (size_t z = 0; z < sizeof(Rows) / sizeof(Rows[0]); z++)
      {
        const Row& e = Rows[z];
        const Res r = Calc(e.clk, e.nom, e.dat, true);
        Assert::IsTrue(r.err == ERR_OK, Name(e.clk, e.nom, e.dat, L"sample points not set should give ERR_OK").c_str());
        if (r.err != ERR_OK) continue;
        const uint32_t nTq = MCP251XFD_NSYNC + r.n1 + r.n2, dTq = MCP251XFD_DSYNC + r.d1 + r.d2;
        const uint32_t nsp = ((MCP251XFD_NSYNC + r.n1) * 1000u) / nTq, dsp = ((MCP251XFD_DSYNC + r.d1) * 1000u) / dTq;
        Assert::IsTrue(Absd(nsp, e.nomSp) <= 1000u / nTq + 2u, Name(e.clk, e.nom, e.dat, L"default nominal sample point", nsp, e.nomSp).c_str());
        Assert::IsTrue(Absd(dsp, e.dataSp) <= 1000u / dTq + 2u, Name(e.clk, e.nom, e.dat, L"default data sample point", dsp, e.dataSp).c_str());
      }
    }
#endif
  };
}
