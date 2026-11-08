/*!*****************************************************************************
 * @file    BitrateSweep.cpp
 * @brief   Exhaustive test of MCP251XFD_CalculateBitTimeConfiguration() and of the bitrate statistics
 * @details Every result is checked against values computed independently from the registers
 *          and against a brute force search, not against the driver itself:
 *           1. registers inside the limits of the component, SJW <= min(SEG1, SEG2)
 *           2. same prescaler for the nominal and the data phase
 *           3. the bitrates computed from the registers equal the ones of the statistics
 *           4. the sum of the bitrate errors is the lowest possible (brute force over all prescalers and TQ counts)
 *           5. [v2] the sample points are the nearest possible for the chosen TQ counts, and among the best
 *              bitrate errors the sample points are the nearest of the wanted ones
 *           6. [v2] sample points statistics, TDC (SSP = DBRP x (DSYNC + DTSEG1)), oscillator tolerance
 *           7. same result twice, with and without statistics, CAN2.0 only defaults, rejected parameters
 *
 *          Works with the 1.x driver (MCP251XFD_BitTimeConfig) and the 2.x driver (CAN_BitTimeConfig)
 *          Usage:  bitrate_sweep [--full] [--clock MHz] [--no-sweep] [--random N] [--seed S] [-v]
 *            (default)   quick sweep: SYSCLK 20 and 40 MHz, nominal every 25 kbit/s, data every 250 kbit/s
 *            --full      all SYSCLK from 2 to 40 MHz, nominal every 5 kbit/s, data every 25 kbit/s
 *            --clock N   only the SYSCLK of N MHz
 *            --no-sweep  skip the sweep (only the random cases)
 *            --random N  N more random cases (any SYSCLK, nominal, data and sample points)
 *          Build (MSVC):  cl /TP /std:c++17 /EHsc /O2 /I..\.. /I..\..\CRC BitrateSweep.cpp ..\..\MCP251XFD.c ..\..\CRC\CRC_CAN.c
 *          Build (GCC):   g++ -x c++ -std=c++17 -O2 -I../.. -I../../CRC BitrateSweep.cpp ../../MCP251XFD.c ../../CRC/CRC_CAN.c
 *          Exit code: 0 if all the checks passed
 ******************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <initializer_list>
#include <string>
#include "MCP251XFD.h"

#ifndef CAN_NO_CANFD
#  define CAN_NO_CANFD  MCP251XFD_NO_CANFD
#endif

// Older drivers: a TQ count is possible only if the floor of the ratio is inside the limits (no ceil alone)
#ifndef MCP251XFD_BITRATE_MAX_ERROR_PERMILLE
#  define MCP251XFD_BITRATE_MAX_ERROR_PERMILLE  0u
#endif

#ifdef CAN_COMMON_H_INC
#  define API_V2 1
#else
#  define API_V2 0
#endif

/*! @brief Adapter : one normalized result for both drivers (all values are real values: register + 1) */
struct Sp { bool permille; unsigned value; };   // Wanted sample point (percent or per mille). Only used by the v2 driver

struct Res
{
  int      err;
  uint32_t nBrp, n1, n2, nSjw;     // Nominal: prescaler, TSEG1, TSEG2, SJW
  uint32_t dBrp, d1, d2, dSjw;     // Data
  int      tdcMode;                // 0 = disabled, 1 = manual, 2 = auto
  int32_t  tdco, tdcv;
  uint32_t nbr, dbr, len, nsp, dsp, c[5], tol; // Statistics
  uint8_t  raw[256];               // Raw configuration without the statistics pointer, to compare two runs
  size_t   rawSize;
};

#if API_V2
static Res Calc(uint32_t clk, uint32_t nom, uint32_t dat, Sp spN, Sp spD, bool withStats)
{
  Res r; memset(&r, 0, sizeof r);
  CAN_BitTimeConfig c; CAN_BitTimeStats t; CAN_CANFDbusConfig b;
  memset(&c, 0, sizeof c); memset(&t, 0, sizeof t); memset(&b, 0, sizeof b);
  b.DesiredNominalBitrate = nom; b.DesiredDataBitrate = dat;
  if (spN.permille) b.NominalSamplePointPermille = (uint16_t)spN.value; else b.NominalSamplePoint = (uint8_t)spN.value;
  if (spD.permille) b.DataSamplePointPermille    = (uint16_t)spD.value; else b.DataSamplePoint    = (uint8_t)spD.value;
  c.Stats = withStats ? &t : NULL;
  r.err = (int)MCP251XFD_CalculateBitTimeConfiguration(clk, b, &c);
  r.nBrp = c.NBRP + 1u; r.n1 = c.NTSEG1 + 1u; r.n2 = c.NTSEG2 + 1u; r.nSjw = c.NSJW + 1u;
  r.dBrp = c.DBRP + 1u; r.d1 = c.DTSEG1 + 1u; r.d2 = c.DTSEG2 + 1u; r.dSjw = c.DSJW + 1u;
  r.tdcMode = (c.TDCmode == CAN_TDC_AUTO_MODE ? 2 : c.TDCmode == CAN_TDC_MANUAL_MODE ? 1 : 0);
  r.tdco = c.TDCO; r.tdcv = c.TDCV;
  r.nbr = t.NominalBitrate; r.dbr = t.DataBitrate; r.len = t.MaxBusLength; r.nsp = t.NSamplePoint; r.dsp = t.DSamplePoint;
  r.c[0] = t.OscTolC1; r.c[1] = t.OscTolC2; r.c[2] = t.OscTolC3; r.c[3] = t.OscTolC4; r.c[4] = t.OscTolC5; r.tol = t.OscTolerance;
  c.Stats = NULL; r.rawSize = sizeof c < sizeof r.raw ? sizeof c : sizeof r.raw; memcpy(r.raw, &c, r.rawSize);
  return r;
}
#else
static Res Calc(uint32_t clk, uint32_t nom, uint32_t dat, Sp, Sp, bool withStats)
{
  Res r; memset(&r, 0, sizeof r);
  MCP251XFD_BitTimeConfig c; MCP251XFD_BitTimeStats t;
  memset(&c, 0, sizeof c); memset(&t, 0, sizeof t);
  c.Stats = withStats ? &t : NULL;
  r.err = (int)MCP251XFD_CalculateBitTimeConfiguration(clk, nom, dat, &c);
  r.nBrp = c.NBRP + 1u; r.n1 = c.NTSEG1 + 1u; r.n2 = c.NTSEG2 + 1u; r.nSjw = c.NSJW + 1u;
  r.dBrp = c.DBRP + 1u; r.d1 = c.DTSEG1 + 1u; r.d2 = c.DTSEG2 + 1u; r.dSjw = c.DSJW + 1u;
  r.tdcMode = (c.TDCMOD == MCP251XFD_AUTO_MODE ? 2 : c.TDCMOD == MCP251XFD_MANUAL_MODE ? 1 : 0);
  r.tdco = c.TDCO; r.tdcv = (int32_t)c.TDCV;
  r.nbr = t.NominalBitrate; r.dbr = t.DataBitrate; r.len = t.MaxBusLength; r.nsp = t.NSamplePoint; r.dsp = t.DSamplePoint;
  r.c[0] = t.OscTolC1; r.c[1] = t.OscTolC2; r.c[2] = t.OscTolC3; r.c[3] = t.OscTolC4; r.c[4] = t.OscTolC5; r.tol = t.OscTolerance;
  c.Stats = NULL; r.rawSize = sizeof c < sizeof r.raw ? sizeof c : sizeof r.raw; memcpy(r.raw, &c, r.rawSize);
  return r;
}
#endif

static bool SameRes(const Res& a, const Res& b)
{
  if ((a.err != b.err) || (a.nBrp != b.nBrp) || (a.n1 != b.n1) || (a.n2 != b.n2) || (a.nSjw != b.nSjw) || (a.dBrp != b.dBrp) || (a.d1 != b.d1) || (a.d2 != b.d2) || (a.dSjw != b.dSjw)) return false;
  if ((a.tdcMode != b.tdcMode) || (a.tdco != b.tdco) || (a.tdcv != b.tdcv) || (a.nbr != b.nbr) || (a.dbr != b.dbr) || (a.len != b.len) || (a.nsp != b.nsp) || (a.dsp != b.dsp) || (a.tol != b.tol)) return false;
  for (int i = 0; i < 5; i++) if (a.c[i] != b.c[i]) return false;
  return (a.rawSize == b.rawSize) && (memcmp(a.raw, b.raw, a.rawSize) == 0);
}

//=============================================================================
// Checks bookkeeping
//=============================================================================
struct Check { const char* name; unsigned long long runs, fails; };
static Check Checks[] = {
  { "ranges    registers inside the component limits, SJW <= min(SEG1,SEG2)", 0, 0 },
  { "same BRP  same prescaler in both phases, except if one per phase is better",0, 0 },
  { "bitrates  statistics equal the bitrates computed from the registers",    0, 0 },
  { "buslen    maximum bus length is never negative (no unsigned wrap)",           0, 0 },
  { "optimum   sum of the bitrate errors is the lowest possible (brute force)", 0, 0 },
  { "sp-split  sample points are the nearest possible for the chosen TQ",     0, 0 },
  { "sp-best   best sample points among the best bitrate errors",             0, 0 },
  { "sp-stats  sample points of the statistics are exact",                    0, 0 },
  { "tdc       TDC automatic only with a data prescaler of 1 or 2, offset, value",0, 0 },
  { "osc-tol   oscillator tolerance is the minimum of the conditions",        0, 0 },
  { "repeat    same result twice, with and without statistics",               0, 0 },
  { "can20     CAN2.0 only configuration",                                    0, 0 },
  { "rejected  out of range parameters are refused",                          0, 0 },
};
enum { C_RANGES, C_SAMEBRP, C_BITRATES, C_BUSLEN, C_OPTIMUM, C_SPSPLIT, C_SPBEST, C_SPSTATS, C_TDC, C_OSC, C_REPEAT, C_CAN20, C_REJECT, C_COUNT };
static unsigned Printed = 0; static bool Verbose = false;

#include <cstdarg>
static void Report(int idx, bool ok, uint32_t clk, uint32_t nom, uint32_t dat, Sp sp, const char* fmt, ...)
{
  Checks[idx].runs++;
  if (ok) return;
  Checks[idx].fails++;
  if (Verbose || (Checks[idx].fails <= 5))
  {
    char msg[300]; va_list ap; va_start(ap, fmt); vsnprintf(msg, sizeof msg, fmt, ap); va_end(ap);
    std::printf("  FAIL %-8.8s clk=%u nom=%u dat=%u sp=%u%s : %s\n", Checks[idx].name, clk, nom, dat, sp.value, sp.permille ? "pm" : "%", msg);
  }
  Printed++;
}

//=============================================================================
// Independent references
//=============================================================================
static uint32_t Absd(uint32_t a, uint32_t b) { return a > b ? a - b : b - a; }
static uint32_t BrErr(uint32_t clk, uint32_t brp, uint32_t tq, uint32_t des) { return Absd(clk / (brp * tq), des); }

/*! @brief Policy of the driver: a TQ count is possible if it is inside the limits and if its bitrate error is at most MCP251XFD_BITRATE_MAX_ERROR_PERMILLE of the wanted bitrate (floor and ceil) */
static bool TqAllowed(uint32_t clk, uint32_t brp, uint32_t tq, uint32_t des, uint32_t tqMin, uint32_t tqMax)
{
  if ((tq < tqMin) || (tq > tqMax)) return false;
  return ((unsigned long long)BrErr(clk, brp, tq, des) * 1000ull) <= ((unsigned long long)des * MCP251XFD_BITRATE_MAX_ERROR_PERMILLE);
}

/*! @brief Best (lowest) error over all TQ counts for each prescaler : tab[brp] */
static void BestTable(uint32_t clk, uint32_t des, uint32_t tqMin, uint32_t tqMax, std::vector<uint32_t>& tab)
{
  tab.assign(MCP251XFD_NBRP_MAX + 1, 0xFFFFFFFFu);
  for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
    for (uint32_t tq = tqMin; tq <= tqMax; tq++) { if (!TqAllowed(clk, brp, tq, des, tqMin, tqMax)) continue; uint32_t e = BrErr(clk, brp, tq, des); if (e < tab[brp]) tab[brp] = e; }
}

/*! @brief Deviation (per mille) of the nearest sample point possible for a TQ count : brute force over all the splits */
static uint32_t SpRefDev(uint32_t tq, uint32_t wanted, bool isData)
{
  const uint32_t sync = isData ? MCP251XFD_DSYNC : MCP251XFD_NSYNC;
  const uint32_t s1Min = isData ? MCP251XFD_DTSEG1_MIN : MCP251XFD_NTSEG1_MIN, s1Max = isData ? MCP251XFD_DTSEG1_MAX : MCP251XFD_NTSEG1_MAX;
  const uint32_t s2Min = isData ? MCP251XFD_DTSEG2_MIN : MCP251XFD_NTSEG2_MIN, s2Max = isData ? MCP251XFD_DTSEG2_MAX : MCP251XFD_NTSEG2_MAX;
  uint32_t best = 0xFFFFFFFFu;
  for (uint32_t s2 = s2Min; s2 <= s2Max; s2++)
  {
    if (tq < sync + s2) break;
    const uint32_t s1 = tq - sync - s2;
    if ((s1 < s1Min) || (s1 > s1Max)) continue;
    const uint32_t d = Absd(((sync + s1) * 1000u) / tq, wanted);
    if (d < best) best = d;
  }
  return best;
}
static uint32_t SpActDev(uint32_t seg1, uint32_t seg2, uint32_t wanted, bool isData)
{
  const uint32_t sync = isData ? MCP251XFD_DSYNC : MCP251XFD_NSYNC;
  return Absd(((sync + seg1) * 1000u) / (sync + seg1 + seg2), wanted);
}

//=============================================================================
// One case
//=============================================================================
/*! @brief True if the wanted bitrates are possible: at least one prescaler for the nominal phase and at least one prescaler for the data phase (the driver uses a prescaler per phase if no common one is as good) */
static bool Feasible(uint32_t clk, uint32_t nom, uint32_t dat)
{
  bool nomOk = false, dataOk = (dat == CAN_NO_CANFD);
  for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
  {
    const uint32_t ntq = clk / (nom * brp);
    if (TqAllowed(clk, brp, ntq, nom, MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX) || TqAllowed(clk, brp, ntq + 1, nom, MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX)) nomOk = true;
    if (dat == CAN_NO_CANFD) continue;
    const uint32_t dtq = clk / (dat * brp);
    if (TqAllowed(clk, brp, dtq, dat, MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX) || TqAllowed(clk, brp, dtq + 1, dat, MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX)) dataOk = true;
  }
  return nomOk && dataOk;
}
#if !API_V2
/*! @brief Wanted sample point of the 1.x driver (it has no sample point parameter): 87.5% up to 500 kbit/s, 80% up to 800 kbit/s, 75% above */
static uint32_t TierSp(uint32_t bitrate) { return (bitrate > 800000u) ? 750u : ((bitrate > 500000u) ? 800u : 875u); }
#endif
struct BestCache { uint32_t clk = 0, nom = 0, dat = 0; std::vector<uint32_t> n, d; bool ok = false; };

static void RunCase(uint32_t clk, uint32_t nom, uint32_t dat, Sp spN, Sp spD, const std::vector<uint32_t>& bestN, const std::vector<uint32_t>& bestD)
{
  const bool fd = (dat != CAN_NO_CANFD);
  Res r = Calc(clk, nom, dat, spN, spD, true);
  if (r.err != 0) { Report(C_REJECT, !Feasible(clk, nom, dat), clk, nom, dat, spN, "refused (error %d) but a valid configuration exists", r.err); return; }

  const uint32_t nTq = MCP251XFD_NSYNC + r.n1 + r.n2, dTq = MCP251XFD_DSYNC + r.d1 + r.d2;
  //--- 1. ranges
  bool ok = (r.nBrp >= MCP251XFD_NBRP_MIN) && (r.nBrp <= MCP251XFD_NBRP_MAX) && (r.n1 >= MCP251XFD_NTSEG1_MIN) && (r.n1 <= MCP251XFD_NTSEG1_MAX)
         && (r.n2 >= MCP251XFD_NTSEG2_MIN) && (r.n2 <= MCP251XFD_NTSEG2_MAX) && (r.nSjw >= MCP251XFD_NSJW_MIN) && (r.nSjw <= MCP251XFD_NSJW_MAX)
         && (r.nSjw <= r.n1) && (r.nSjw <= r.n2) && (nTq >= MCP251XFD_NTQBIT_MIN) && (nTq <= MCP251XFD_NTQBIT_MAX);
  if (fd) ok = ok && (r.dBrp >= MCP251XFD_DBRP_MIN) && (r.dBrp <= MCP251XFD_DBRP_MAX) && (r.d1 >= MCP251XFD_DTSEG1_MIN) && (r.d1 <= MCP251XFD_DTSEG1_MAX)
         && (r.d2 >= MCP251XFD_DTSEG2_MIN) && (r.d2 <= MCP251XFD_DTSEG2_MAX) && (r.dSjw >= MCP251XFD_DSJW_MIN) && (r.dSjw <= MCP251XFD_DSJW_MAX)
         && (r.dSjw <= r.d1) && (r.dSjw <= r.d2) && (dTq >= MCP251XFD_DTQBIT_MIN) && (dTq <= MCP251XFD_DTQBIT_MAX);
  Report(C_RANGES, ok, clk, nom, dat, spN, "NBRP=%u N1=%u N2=%u NSJW=%u | DBRP=%u D1=%u D2=%u DSJW=%u", r.nBrp, r.n1, r.n2, r.nSjw, r.dBrp, r.d1, r.d2, r.dSjw);
  if (!ok) return; // The next checks need valid registers

  //--- 3. bitrates from the registers = statistics
  const uint32_t nbr = clk / (r.nBrp * nTq), dbr = fd ? clk / (r.dBrp * dTq) : 0u;
  Report(C_BITRATES, (r.nbr == nbr) && (r.dbr == dbr), clk, nom, dat, spN, "stats %u/%u registers %u/%u", r.nbr, r.dbr, nbr, dbr);

  //--- 3b. maximum bus length computed with signed values (0 if the propagation budget is lower than the transceiver delay)
  {
    const long long ntqNs  = (long long)(((unsigned long long)r.nBrp * 1000000ull) / (clk / 1000u));
    const long long budget = ntqNs * ((long long)r.n1 - (long long)r.n2) - 2ll * MCP251XFD_tTXDtRXD_MAX;
    const uint32_t expected = (budget > 0) ? (uint32_t)(budget / (2 * MCP251XFD_tBUS_CONV)) : 0u;
    Report(C_BUSLEN, r.len == expected, clk, nom, dat, spN, "max bus length %u expected %u (NTQ=%lld ns, SEG1-SEG2=%lld)", r.len, expected, ntqNs, (long long)r.n1 - (long long)r.n2);
  }

  //--- 4. lowest possible sum of the bitrate errors, with a common prescaler (eCommon) and with a prescaler per phase (eIndep)
  uint32_t eCommon = 0xFFFFFFFFu, minN = 0xFFFFFFFFu, minD = 0xFFFFFFFFu;
  for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
  {
    if (bestN[brp] < minN) minN = bestN[brp];
    if (fd && (bestD[brp] < minD)) minD = bestD[brp];
    if ((bestN[brp] == 0xFFFFFFFFu) || (fd && (bestD[brp] == 0xFFFFFFFFu))) continue; // No TQ count allowed with this prescaler
    const uint32_t e = bestN[brp] + (fd ? bestD[brp] : 0u); if (e < eCommon) eCommon = e;
  }
  const uint32_t eIndep = fd ? ((minN == 0xFFFFFFFFu || minD == 0xFFFFFFFFu) ? 0xFFFFFFFFu : minN + minD) : eCommon;
  const uint32_t eMin   = (eIndep < eCommon) ? eIndep : eCommon;
  const uint32_t eDrv = Absd(nbr, nom) + (fd ? Absd(dbr, dat) : 0u);
  Report(C_OPTIMUM, eDrv == eMin, clk, nom, dat, spN, "driver error %u (BRP=%u/%u NTQ=%u DTQ=%u) best possible %u (common prescaler %u, one prescaler per phase %u)", eDrv, r.nBrp, r.dBrp, nTq, dTq, eMin, eCommon, eIndep);
  //--- 5. sample points (the 1.x driver has no sample point parameter: it aims at 87.5% up to 500 kbit/s, 80% up to 800 kbit/s, 75% above)
#if API_V2
  const uint32_t wN = spN.permille ? spN.value : spN.value * 10u, wD = spD.permille ? spD.value : spD.value * 10u;
#else
  const uint32_t wN = TierSp(nom), wD = fd ? TierSp(dat) : 0u;
#endif
  const uint32_t devN = SpActDev(r.n1, r.n2, wN, false), devD = fd ? SpActDev(r.d1, r.d2, wD, true) : 0u;
  Report(C_SPSPLIT, (devN <= SpRefDev(nTq, wN, false) + 1u) && (!fd || (devD <= SpRefDev(dTq, wD, true) + 1u)), clk, nom, dat, spN,
         "nominal dev %u (best %u) data dev %u (best %u)", devN, SpRefDev(nTq, wN, false), devD, fd ? SpRefDev(dTq, wD, true) : 0u);
  // Lowest sample point deviation of a phase for a prescaler, among the TQ counts that give the lowest bitrate error of this prescaler
  auto phaseDev = [&](uint32_t brp, uint32_t br, uint32_t best, uint32_t tqMin, uint32_t tqMax, uint32_t wanted, bool isData) -> uint32_t
  {
    uint32_t dev = 0xFFFFFFFFu;
    for (uint32_t tq = tqMin; tq <= tqMax; tq++) if (TqAllowed(clk, brp, tq, br, tqMin, tqMax) && (BrErr(clk, brp, tq, br) == best)) { const uint32_t d = SpRefDev(tq, wanted, isData); if (d < dev) dev = d; }
    return dev;
  };
  // Lowest sample point deviation with a common prescaler (among the prescalers that give eCommon) and with a prescaler for each phase
  uint32_t devCommon = 0xFFFFFFFFu, devIndep = 0xFFFFFFFFu;
  for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
  {
    if ((bestN[brp] == 0xFFFFFFFFu) || (fd && (bestD[brp] == 0xFFFFFFFFu))) continue;
    if (bestN[brp] + (fd ? bestD[brp] : 0u) != eCommon) continue;
    const uint32_t dn = phaseDev(brp, nom, bestN[brp], MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX, wN, false);
    const uint32_t dd = fd ? phaseDev(brp, dat, bestD[brp], MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX, wD, true) : 0u;
    if ((dn != 0xFFFFFFFFu) && (dd != 0xFFFFFFFFu) && (dn + dd < devCommon)) devCommon = dn + dd;
  }
  if (fd && (eIndep != 0xFFFFFFFFu))
  {
    uint32_t dn = 0xFFFFFFFFu, dd = 0xFFFFFFFFu;
    for (uint32_t brp = MCP251XFD_NBRP_MIN; brp <= MCP251XFD_NBRP_MAX; brp++)
    {
      if (bestN[brp] == minN) { const uint32_t d = phaseDev(brp, nom, bestN[brp], MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX, wN, false); if (d < dn) dn = d; }
      if (bestD[brp] == minD) { const uint32_t d = phaseDev(brp, dat, bestD[brp], MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX, wD, true);  if (d < dd) dd = d; }
    }
    if ((dn != 0xFFFFFFFFu) && (dd != 0xFFFFFFFFu)) devIndep = dn + dd;
  }
  // A prescaler for each phase is better if its bitrate error is lower, or with the same bitrate error if its sample point is nearer
  // (the sample point deviations are compared with a tolerance of 2 per mille, the driver and this reference do not round the same way)
  const bool indepBetter = fd && (eIndep != 0xFFFFFFFFu) && ((eIndep < eCommon) || ((eIndep == eCommon) && (devIndep + 2u < devCommon)));
  const bool indepAllowed = fd && (eIndep != 0xFFFFFFFFu) && ((eIndep < eCommon) || ((eIndep == eCommon) && (devIndep <= devCommon + 2u)));
  //--- 4b. the two phases use the same prescaler (identical TQ prevents the quantization errors during the bit rate switching), unless a prescaler per phase gives a lower bitrate error or a nearer sample point
  if (fd) Report(C_SAMEBRP, (r.nBrp == r.dBrp) ? !indepBetter : indepAllowed, clk, nom, dat, spN, "NBRP=%u DBRP=%u (error / sample point deviation with a common prescaler %u / %u, with one prescaler per phase %u / %u)", r.nBrp, r.dBrp, eCommon, devCommon, eIndep, devIndep);
  if (eDrv == eMin)
  {
    uint32_t bestDev = 0xFFFFFFFFu;
    if ((eCommon == eMin) && (devCommon < bestDev)) bestDev = devCommon;
    if ((eIndep  == eMin) && (devIndep  < bestDev)) bestDev = devIndep;
    Report(C_SPBEST, devN + devD <= bestDev + 2u, clk, nom, dat, spN, "driver deviation %u best possible %u | driver BRP=%u/%u NTQ=%u DTQ=%u | wanted nominal %u data %u per mille", devN + devD, bestDev, r.nBrp, r.dBrp, nTq, dTq, wN, wD);
  }

  //--- 6. statistics of the sample points
  Report(C_SPSTATS, (r.nsp == ((MCP251XFD_NSYNC + r.n1) * 10000u) / nTq) && (r.dsp == (fd ? ((MCP251XFD_DSYNC + r.d1) * 10000u) / dTq : 0u)), clk, nom, dat, spN,
         "stats %u/%u expected %u/%u", r.nsp, r.dsp, ((MCP251XFD_NSYNC + r.n1) * 10000u) / nTq, fd ? ((MCP251XFD_DSYNC + r.d1) * 10000u) / dTq : 0u);
  //--- 6b. TDC: automatic only with a data prescaler of 1 or 2 (ISO 11898-1 section 11.3.3, like the Linux kernel), TDCO is the data sample point in SYSCLK periods, TDCV is measured by the component
  if (fd)
  {
    const uint32_t ssp = r.dBrp * (MCP251XFD_DSYNC + r.d1);
    const bool automatic = (r.dBrp <= 2u);
    const int32_t tdco = automatic ? (int32_t)(ssp > (uint32_t)MCP251XFD_TDCO_MAX ? (uint32_t)MCP251XFD_TDCO_MAX : ssp) : 0;
    Report(C_TDC, (r.tdcMode == (automatic ? 2 : 0)) && (r.tdco == tdco) && (r.tdcv == 0), clk, nom, dat, spN, "mode %d tdco %d tdcv %d expected mode %d tdco %d tdcv 0", r.tdcMode, r.tdco, r.tdcv, automatic ? 2 : 0, tdco);
  }

  //--- 7. oscillator tolerance
  uint32_t tol = r.c[0] < r.c[1] ? r.c[0] : r.c[1];
  if (fd) for (int i = 2; i < 5; i++) if (r.c[i] < tol) tol = r.c[i];
  Report(C_OSC, r.tol == tol, clk, nom, dat, spN, "tolerance %u expected %u (C1..C5 = %u %u %u %u %u)", r.tol, tol, r.c[0], r.c[1], r.c[2], r.c[3], r.c[4]);

  //--- 8. repeatable, same configuration without statistics
  Res r2 = Calc(clk, nom, dat, spN, spD, false), r3 = Calc(clk, nom, dat, spN, spD, true);
  Report(C_REPEAT, (r2.err == 0) && (r2.rawSize == r.rawSize) && (memcmp(r2.raw, r.raw, r.rawSize) == 0) && SameRes(r3, r), clk, nom, dat, spN, "results differ between two runs");

  //--- 9. CAN2.0 only
  if (!fd)
  {
#if API_V2
    Report(C_CAN20, (r.dBrp == 1) && (r.d1 == 15) && (r.d2 == 4) && (r.dSjw == 4) && (r.dbr == 0) && (r.dsp == 0), clk, nom, dat, spN, "data defaults: DBRP=%u D1=%u D2=%u DSJW=%u dbr=%u", r.dBrp, r.d1, r.d2, r.dSjw, r.dbr);
#else
    Report(C_CAN20, (r.dBrp == 1) && (r.d1 == 15) && (r.d2 == 4) && (r.dSjw == 4), clk, nom, dat, spN, "data defaults: DBRP=%u D1=%u D2=%u DSJW=%u", r.dBrp, r.d1, r.d2, r.dSjw);
#endif
  }
}

//=============================================================================
// Sweeps
//=============================================================================
static uint64_t Rng = 0x9E3779B97F4A7C15ull;
static uint32_t Rand32() { Rng ^= Rng << 13; Rng ^= Rng >> 7; Rng ^= Rng << 17; return (uint32_t)(Rng >> 16); }
static uint32_t RandRange(uint32_t lo, uint32_t hi) { return lo + Rand32() % (hi - lo + 1u); }

static void CheckRejected()
{
  const Sp sp = { false, 80 };
  struct { uint32_t clk, nom, dat; } bad[] = {
    { MCP251XFD_SYSCLK_MIN - 1, 500000, 2000000 }, { MCP251XFD_SYSCLK_MAX + 1, 500000, 2000000 },
    { 40000000, MCP251XFD_NOMBITRATE_MIN - 1, 2000000 }, { 40000000, MCP251XFD_NOMBITRATE_MAX + 1, 2000000 },
    { 40000000, 500000, MCP251XFD_DATABITRATE_MIN - 1 }, { 40000000, 500000, MCP251XFD_DATABITRATE_MAX + 1 }, { 40000000, 500000, 1 },
  };
  for (size_t i = 0; i < sizeof bad / sizeof bad[0]; i++) { Res r = Calc(bad[i].clk, bad[i].nom, bad[i].dat, sp, sp, true); Report(C_REJECT, r.err != 0, bad[i].clk, bad[i].nom, bad[i].dat, sp, "accepted"); }
  struct { uint32_t clk, nom, dat; } good[] = {
    { MCP251XFD_SYSCLK_MIN, MCP251XFD_NOMBITRATE_MIN, 0 }, { MCP251XFD_SYSCLK_MAX, MCP251XFD_NOMBITRATE_MAX, MCP251XFD_DATABITRATE_MAX },
    { MCP251XFD_SYSCLK_MAX, MCP251XFD_NOMBITRATE_MIN, MCP251XFD_DATABITRATE_MIN }, { MCP251XFD_SYSCLK_MIN, MCP251XFD_NOMBITRATE_MIN, MCP251XFD_DATABITRATE_MIN },
  };
  for (size_t i = 0; i < sizeof good / sizeof good[0]; i++) { Res r = Calc(good[i].clk, good[i].nom, good[i].dat, sp, sp, true); Report(C_REJECT, r.err == 0 || (good[i].clk < 4000000), good[i].clk, good[i].nom, good[i].dat, sp, "limit value refused (error %d)", r.err); }
}

int main(int argc, char** argv)
{
  uint32_t onlyClock = 0;
  bool full = false, noSweep = false; unsigned long long randomCount = 0;
  for (int i = 1; i < argc; i++)
  {
    if (!std::strcmp(argv[i], "--full")) full = true;
    else if (!std::strcmp(argv[i], "-v")) Verbose = true;
    else if (!std::strcmp(argv[i], "--no-sweep")) noSweep = true;
    else if (!std::strcmp(argv[i], "--clock") && (i + 1 < argc)) onlyClock = (uint32_t)std::strtoul(argv[++i], NULL, 10) * 1000000u;
    else if (!std::strcmp(argv[i], "--random") && (i + 1 < argc)) randomCount = std::strtoull(argv[++i], NULL, 10);
    else if (!std::strcmp(argv[i], "--seed") && (i + 1 < argc)) Rng ^= std::strtoull(argv[++i], NULL, 10) * 0x2545F4914F6CDD1Dull;
  }

  std::vector<uint32_t> clocks, noms, dats; std::vector<Sp> sps;
  if (full) { for (uint32_t m = MCP251XFD_SYSCLK_MIN / 1000000u; m <= MCP251XFD_SYSCLK_MAX / 1000000u; m++) clocks.push_back(m * 1000000u); }
  else { clocks.push_back(20000000u); clocks.push_back(40000000u); }
  if (onlyClock != 0) { clocks.clear(); clocks.push_back(onlyClock); }
  const uint32_t nStep = full ? 5000u : 25000u, dStep = full ? 25000u : 250000u;
  for (uint32_t n = MCP251XFD_NOMBITRATE_MIN; n <= MCP251XFD_NOMBITRATE_MAX; n += nStep) noms.push_back(n);
  for (uint32_t n : std::initializer_list<uint32_t>{ 10000u, 20000u, 50000u, 100000u, 125000u, 250000u, 333333u, 800000u, (uint32_t)MCP251XFD_NOMBITRATE_MAX }) noms.push_back(n);
  dats.push_back(CAN_NO_CANFD);
  for (uint32_t d = MCP251XFD_DATABITRATE_MIN; d <= MCP251XFD_DATABITRATE_MAX; d += dStep) dats.push_back(d);
  for (uint32_t d : std::initializer_list<uint32_t>{ 833333u, 2500000u, 5000000u, 6666666u, (uint32_t)MCP251XFD_DATABITRATE_MAX }) dats.push_back(d);
  sps.push_back({ false, 80 }); sps.push_back({ false, 70 });
#if API_V2
  sps.push_back({ true, 875 });
  if (full) { sps.push_back({ false, 87 }); sps.push_back({ true, 825 }); }
#endif

  std::printf("MCP251XFD bitrate sweep (%s driver, %s): %zu clocks x %zu nominal x %zu data x %zu sample points\n", API_V2 ? "2.x" : "1.x", full ? "full" : "quick", clocks.size(), noms.size(), dats.size(), sps.size());
  unsigned long long cases = 0;
  for (uint32_t clk : clocks)
  {
    if (noSweep) break;
    std::vector<std::vector<uint32_t>> bestN(noms.size()), bestD(dats.size());
    for (size_t i = 0; i < noms.size(); i++) BestTable(clk, noms[i], MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX, bestN[i]);
    for (size_t i = 0; i < dats.size(); i++) if (dats[i] != CAN_NO_CANFD) BestTable(clk, dats[i], MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX, bestD[i]);
    for (size_t i = 0; i < noms.size(); i++)
      for (size_t j = 0; j < dats.size(); j++)
        for (size_t k = 0; k < sps.size(); k++)
        {
          if ((dats[j] != CAN_NO_CANFD) && (dats[j] < noms[i])) continue; // Data bitrate lower than the nominal one is not a CAN-FD case
          Sp spD = sps[k]; if (k == 1) { spD.value = 75; }               // Different nominal and data sample points
          RunCase(clk, noms[i], dats[j], sps[k], spD, bestN[i], bestD[j]); cases++;
        }
    std::fflush(stdout);
  }
  for (unsigned long long q = 0; q < randomCount; q++)
  {
    const uint32_t clk = RandRange(MCP251XFD_SYSCLK_MIN, MCP251XFD_SYSCLK_MAX), nom = RandRange(MCP251XFD_NOMBITRATE_MIN, MCP251XFD_NOMBITRATE_MAX);
    const uint32_t dat = (Rand32() % 8 == 0) ? CAN_NO_CANFD : RandRange(nom > MCP251XFD_DATABITRATE_MIN ? nom : MCP251XFD_DATABITRATE_MIN, MCP251XFD_DATABITRATE_MAX);
    Sp spN = { false, RandRange(50, 95) }, spD = { false, RandRange(50, 95) };
#if API_V2
    if (Rand32() & 1) { spN.permille = true; spN.value = RandRange(500, 950); }
    if (Rand32() & 1) { spD.permille = true; spD.value = RandRange(500, 950); }
#endif
    std::vector<uint32_t> bn, bd; BestTable(clk, nom, MCP251XFD_NTQBIT_MIN, MCP251XFD_NTQBIT_MAX, bn); if (dat != CAN_NO_CANFD) BestTable(clk, dat, MCP251XFD_DTQBIT_MIN, MCP251XFD_DTQBIT_MAX, bd);
    RunCase(clk, nom, dat, spN, spD, bn, bd); cases++;
  }
  CheckRejected();

  std::printf("\n%llu cases\n", cases);
  unsigned long long totalFails = 0;
  for (int i = 0; i < C_COUNT; i++)
  {
    std::printf("  [%s] %-80s %12llu runs, %llu failed\n", Checks[i].fails == 0 ? " OK " : "FAIL", Checks[i].name, Checks[i].runs, Checks[i].fails);
    totalFails += Checks[i].fails;
  }
  std::printf("\n%s\n", totalFails == 0 ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED");
  return totalFails == 0 ? 0 : 1;
}
