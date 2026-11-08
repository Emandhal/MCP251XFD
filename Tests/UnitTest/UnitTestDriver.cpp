/*!*****************************************************************************
 * @file    UnitTestDriver.cpp
 * @brief   Unit tests of the driver functions with a fake MCP251XFD component
 * @details The component is simulated by a memory of 4KB (SFR, CAN registers and RAM) that answers
 *          to the SPI instructions RESET, READ and WRITE, and that has the few behaviours the driver
 *          waits for (operation mode change, oscillator ready, self-cleared FIFO reset).
 *          The expected values are written by hand from the datasheet (register layout of the CAN
 *          controller), they are not computed with the driver.
 *          Works with the 1.x driver (MCP251XFD_CANMessage) and the 2.x driver (CAN_CANMessage)
 ******************************************************************************/
#include "stdafx.h"
#include "CppUnitTest.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#ifdef CAN_COMMON_H_INC
#  define API_V2 1
#else
#  define API_V2 0
#endif

namespace UnitTest_Driver
{
  //=============================================================================
  // Names that are different between the two drivers
  //=============================================================================
#if API_V2
  typedef CAN_CANMessage     TMessage;
  typedef CAN_BitTimeStats   TStats;
  typedef setCAN_MessageCtrlFlags TFlags;
  typedef eCAN_DataLength    TDLC;
#  define MSG_SEQ            MessageMarker
#  define FLAG_FD            CAN_CANFD_FRAME
#  define FLAG_BRS           CAN_SWITCH_BITRATE
#  define FLAG_RTR           CAN_REMOTE_TRANSMISSION_REQUEST
#  define FLAG_EXT           CAN_EXTENDED_MESSAGE_ID
#  define FLAG_ESI           CAN_TRANSMIT_ERROR_PASSIVE
#else
  typedef MCP251XFD_CANMessage    TMessage;
  typedef MCP251XFD_BitTimeStats  TStats;
  typedef setMCP251XFD_MessageCtrlFlags TFlags;
  typedef eMCP251XFD_DataLength   TDLC;
#  define MSG_SEQ            MessageSEQ
#  define FLAG_FD            MCP251XFD_CANFD_FRAME
#  define FLAG_BRS           MCP251XFD_SWITCH_BITRATE
#  define FLAG_RTR           MCP251XFD_REMOTE_TRANSMISSION_REQUEST
#  define FLAG_EXT           MCP251XFD_EXTENDED_MESSAGE_ID
#  define FLAG_ESI           MCP251XFD_TRANSMIT_ERROR_PASSIVE
#endif

  /*! @brief Fake component: 4KB of memory (0x000..0x2EF CAN registers, 0x400..0xBFF RAM, 0xE00..0xE17 SFR) */
  struct FakeDevice
  {
    uint8_t  Mem[4096];
    unsigned Transfers;            // Count of SPI transfers
    uint32_t Ms;                   // Fake time, +1ms at each call of the driver
    bool     Is2518;               // The LPMEN bit of the oscillator register stays set only on the MCP2518FD
    bool     FreezeMode;           // The operation mode never changes (to test the timeouts)
    bool     OscNeverReady;        // The oscillator never gets ready
    bool     Dead;                 // No component: reads give 0 and writes are ignored
    uint8_t  Mode;                 // Actual operation mode of the CAN controller (CiCON.OPMOD is read-only)
    uint8_t  DevId;                // Value of the DEVID register after a reset
    unsigned SpiInits;             // Count of calls of the SPI initialization
    uint32_t LastSpiSpeed;         // Last speed asked at the SPI initialization
  };
  static FakeDevice Dev;
  static std::vector<uint16_t> Writes; // Address of the first byte of each WRITE instruction since Setup()

  static void Put8(uint16_t a, uint8_t v)   { Dev.Mem[a] = v; }
  static void Put16(uint16_t a, uint16_t v) { Dev.Mem[a] = (uint8_t)v; Dev.Mem[a + 1] = (uint8_t)(v >> 8); }
  static void Put32(uint16_t a, uint32_t v) { Put16(a, (uint16_t)v); Put16(a + 2, (uint16_t)(v >> 16)); }
  static uint8_t  Get8(uint16_t a)  { return Dev.Mem[a]; }
  static uint16_t Get16(uint16_t a) { return (uint16_t)(Dev.Mem[a] | (Dev.Mem[a + 1] << 8)); }
  static uint32_t Get32(uint16_t a) { return (uint32_t)Get16(a) | ((uint32_t)Get16(a + 2) << 16); }

  /*! @brief Power-on state of the component */
  static void DeviceReset(void)
  {
    bool is2518 = Dev.Is2518; bool freeze = Dev.FreezeMode; bool osc = Dev.OscNeverReady; bool dead = Dev.Dead; uint8_t devId = Dev.DevId; unsigned inits = Dev.SpiInits; uint32_t speed = Dev.LastSpiSpeed;
    memset(&Dev, 0, sizeof Dev);
    Dev.Is2518 = is2518; Dev.FreezeMode = freeze; Dev.OscNeverReady = osc; Dev.Dead = dead; Dev.DevId = devId; Dev.SpiInits = inits; Dev.LastSpiSpeed = speed;
    Put32(0x000, 0x04980760u); // CiCON: Configuration mode (OPMOD = REQOP = 0b100), ISOCRCEN, PXEDIS, WAKFIL, WFT = 0b11
    Dev.Mode = MCP251XFD_CONFIGURATION_MODE;
    Put32(0xE00, 0x00000460u); // OSC
    Put32(0xE14, Dev.DevId);   // DEVID
  }

  /*! @brief Behaviours of the component after a write of 'size' bytes at 'address' */
  static void AfterWrite(uint16_t address, size_t size)
  {
    const uint16_t last = (uint16_t)(address + size - 1u);
    //--- CiCON: the operation mode follows the request (OPMOD = REQOP) and OPMOD is read-only
    if ((address <= 0x003u) && (last >= 0x002u))
    {
      if ((last >= 0x003u) && !Dev.FreezeMode) Dev.Mode = (uint8_t)(Dev.Mem[0x003] & 0x07u);
      Dev.Mem[0x002] = (uint8_t)((Dev.Mem[0x002] & 0x1Fu) | (Dev.Mode << 5));
    }
    //--- OSC: ready flags
    if ((address <= 0xE00u) && (last >= 0xE00u))
    {
      const uint8_t cfg = Dev.Mem[0xE00];
      uint8_t ready = 0;
      if (!Dev.OscNeverReady) ready = (uint8_t)(0x04u | ((cfg & 0x01u) ? 0x01u : 0u) | ((cfg & 0x10u) ? 0x10u : 0u)); // OSCRDY, PLLRDY if PLLEN, SCLKRDY if SCLKDIV
      Dev.Mem[0xE01] = ready;
      if (!Dev.Is2518) Dev.Mem[0xE00] = (uint8_t)(Dev.Mem[0xE00] & ~0x08u); // LPMEN exists only on the MCP2518FD
    }
    //--- FRESET of the TEF, TXQ and FIFO1..31 is self-cleared (the second byte of the control registers)
    for (uint16_t reg = 0x041u; reg <= 0x1CDu; reg++)
    {
      const bool isCtrl = (reg == 0x041u) || (reg == 0x051u) || ((reg >= 0x05Du) && (((reg - 0x05Du) % 12u) == 0u));
      if (isCtrl && (reg >= address) && (reg <= last)) Dev.Mem[reg] = (uint8_t)(Dev.Mem[reg] & ~0x04u);
    }
  }

  static eERRORRESULT Process(const uint8_t* tx, uint8_t* rx, size_t size)
  {
    Dev.Transfers++;
    if (size < 2u) return ERR__PARAMETER_ERROR;
    const uint8_t cmd = (uint8_t)(tx[0] >> 4);
    const uint16_t addr = (uint16_t)(((tx[0] & 0x0Fu) << 8) | tx[1]);
    if (cmd == MCP251XFD_SPI_INSTRUCTION_RESET) { DeviceReset(); return ERR_OK; }
    if (Dev.Dead) { if (rx != NULL) for (size_t i = 2; i < size; i++) rx[i] = 0; return ERR_OK; }
    if ((size_t)addr + (size - 2u) > sizeof Dev.Mem) return ERR__OUT_OF_RANGE;
    if (cmd == MCP251XFD_SPI_INSTRUCTION_READ)
    {
      if (rx == NULL) return ERR__PARAMETER_ERROR;
      for (size_t i = 2; i < size; i++) rx[i] = Dev.Mem[addr + i - 2u];
      return ERR_OK;
    }
    if (cmd == MCP251XFD_SPI_INSTRUCTION_WRITE)
    {
      for (size_t i = 2; i < size; i++) Dev.Mem[addr + i - 2u] = tx[i];
      Writes.push_back(addr);
      AfterWrite(addr, size - 2u);
      return ERR_OK;
    }
    return ERR__NOT_SUPPORTED; // The CRC instructions are not simulated
  }

#if API_V2
  static eERRORRESULT FakeInit(SPI_Interface*, uint8_t, eSPIInterface_Mode, const uint32_t speed) { Dev.SpiInits++; Dev.LastSpiSpeed = speed; return ERR_OK; }
  static eERRORRESULT FakeTransfer(SPI_Interface*, SPIInterface_Packet* const p) { return Process(p->TxData, p->RxData, p->DataSize); }
#else
  static eERRORRESULT FakeInit(void*, uint8_t, const uint32_t speed) { Dev.SpiInits++; Dev.LastSpiSpeed = speed; return ERR_OK; }
  static eERRORRESULT FakeTransfer(void*, uint8_t, uint8_t* tx, uint8_t* rx, size_t size) { return Process(tx, rx, size); }
#endif
  static uint32_t FakeMs(void) { return Dev.Ms++; }
  static uint16_t FakeCrc(const uint8_t* d, size_t n) { return ComputeCRC16CMS(d, n); }

  static MCP251XFD Comp;

  /*! @brief New component in Configuration mode, normal driver mode (no CRC, no safe write) */
  static void Setup(void)
  {
    Dev.Is2518 = true; Dev.FreezeMode = false; Dev.OscNeverReady = false; Dev.Dead = false; Dev.DevId = 0x2F; Dev.SpiInits = 0; Dev.LastSpiSpeed = 0;
    DeviceReset(); Writes.clear();
    memset(&Comp, 0, sizeof Comp);
#if API_V2
    Comp.SPI.fnSPI_Init = FakeInit; Comp.SPI.fnSPI_Transfer = FakeTransfer;
#else
    Comp.fnSPI_Init = FakeInit; Comp.fnSPI_Transfer = FakeTransfer;
#endif
    Comp.fnGetCurrentms = FakeMs; Comp.fnComputeCRC16 = FakeCrc;
  }
  static void SetMode(eMCP251XFD_OperationMode mode) { Dev.Mode = (uint8_t)mode; Dev.Mem[0x002] = (uint8_t)((Dev.Mem[0x002] & 0x1Fu) | ((unsigned)mode << 5)); Dev.Mem[0x003] = (uint8_t)((Dev.Mem[0x003] & ~0x07u) | (unsigned)mode); }

  //=============================================================================
  // Checks
  //=============================================================================
  static std::wstring MsgText(const wchar_t* what, unsigned long long expected, unsigned long long actual)
  {
    std::wostringstream os;
    os << what << L": expected 0x" << std::hex << expected << L", actual 0x" << actual;
    return os.str();
  }
#define CHECK_EQ(expected, actual, what)  do { const unsigned long long e_ = (unsigned long long)(expected), a_ = (unsigned long long)(actual); Assert::IsTrue(e_ == a_, MsgText(what, e_, a_).c_str()); } while (0)
#define CHECK_OK(call, what)              do { const eERRORRESULT r_ = (call); Assert::IsTrue(r_ == ERR_OK, MsgText(what L" (error)", ERR_OK, (unsigned long long)r_).c_str()); } while (0)
#define CHECK_ERR(expected, call, what)   do { const eERRORRESULT r_ = (call); Assert::IsTrue(r_ == (expected), MsgText(what L" (error)", (unsigned long long)(expected), (unsigned long long)r_).c_str()); } while (0)


  TEST_CLASS(UnitTest_Driver)
  {
  public:

    /*! @brief Addresses of the registers (datasheet MCP2518FD, tables of the memory map) */
    TEST_METHOD(TestMethod_Driver_AddressMap)
    {
      CHECK_EQ(0x000, RegMCP251XFD_CiCON, L"CiCON");
      CHECK_EQ(0x004, RegMCP251XFD_CiNBTCFG, L"CiNBTCFG");
      CHECK_EQ(0x008, RegMCP251XFD_CiDBTCFG, L"CiDBTCFG");
      CHECK_EQ(0x00C, RegMCP251XFD_CiTDC, L"CiTDC");
      CHECK_EQ(0x010, RegMCP251XFD_CiTBC, L"CiTBC");
      CHECK_EQ(0x014, RegMCP251XFD_CiTSCON, L"CiTSCON");
      CHECK_EQ(0x018, RegMCP251XFD_CiVEC, L"CiVEC");
      CHECK_EQ(0x01C, RegMCP251XFD_CiINT, L"CiINT");
      CHECK_EQ(0x020, RegMCP251XFD_CiRXIF, L"CiRXIF");
      CHECK_EQ(0x024, RegMCP251XFD_CiTXIF, L"CiTXIF");
      CHECK_EQ(0x028, RegMCP251XFD_CiRXOVIF, L"CiRXOVIF");
      CHECK_EQ(0x02C, RegMCP251XFD_CiTXATIF, L"CiTXATIF");
      CHECK_EQ(0x030, RegMCP251XFD_CiTXREQ, L"CiTXREQ");
      CHECK_EQ(0x034, RegMCP251XFD_CiTREC, L"CiTREC");
      CHECK_EQ(0x038, RegMCP251XFD_CiBDIAG0, L"CiBDIAG0");
      CHECK_EQ(0x03C, RegMCP251XFD_CiBDIAG1, L"CiBDIAG1");
      CHECK_EQ(0x040, RegMCP251XFD_CiTEFCON, L"CiTEFCON");
      CHECK_EQ(0x044, RegMCP251XFD_CiTEFSTA, L"CiTEFSTA");
      CHECK_EQ(0x048, RegMCP251XFD_CiTEFUA, L"CiTEFUA");
      CHECK_EQ(0x050, RegMCP251XFD_CiTXQCON, L"CiTXQCON");
      CHECK_EQ(0x054, RegMCP251XFD_CiTXQSTA, L"CiTXQSTA");
      CHECK_EQ(0x058, RegMCP251XFD_CiTXQUA, L"CiTXQUA");
      CHECK_EQ(0x05C, RegMCP251XFD_CiFIFOCON1, L"CiFIFOCON1");
      CHECK_EQ(0x060, RegMCP251XFD_CiFIFOSTA1, L"CiFIFOSTA1");
      CHECK_EQ(0x064, RegMCP251XFD_CiFIFOUA1, L"CiFIFOUA1");
      CHECK_EQ(0x068, RegMCP251XFD_CiFIFOCON2, L"CiFIFOCON2");
      CHECK_EQ(0x1C4, RegMCP251XFD_CiFIFOCON31, L"CiFIFOCON31");
      CHECK_EQ(0x1CC, RegMCP251XFD_CiFIFOUA31, L"CiFIFOUA31");
      CHECK_EQ(0x1D0, RegMCP251XFD_CiFLTCON0, L"CiFLTCON0");
      CHECK_EQ(0x1EC, RegMCP251XFD_CiFLTCON7, L"CiFLTCON7");
      CHECK_EQ(0x1F0, RegMCP251XFD_CiFLTOBJ0, L"CiFLTOBJ0");
      CHECK_EQ(0x1F4, RegMCP251XFD_CiMASK0, L"CiMASK0");
      CHECK_EQ(0x2E8, RegMCP251XFD_CiFLTOBJ31, L"CiFLTOBJ31");
      CHECK_EQ(0x2EC, RegMCP251XFD_CiMASK31, L"CiMASK31");
      CHECK_EQ(0xE00, RegMCP251XFD_OSC, L"OSC");
      CHECK_EQ(0xE04, RegMCP251XFD_IOCON_DIRECTION, L"IOCON");
      CHECK_EQ(0xE08, RegMCP251XFD_CRC, L"CRC");
      CHECK_EQ(0xE0C, RegMCP251XFD_ECCCON, L"ECCCON");
      CHECK_EQ(0xE10, RegMCP251XFD_ECCSTAT, L"ECCSTAT");
      CHECK_EQ(0xE14, RegMCP251XFD_DEVID, L"DEVID");
      CHECK_EQ(0x400, MCP251XFD_RAM_ADDR, L"RAM start");
      CHECK_EQ(2048, MCP251XFD_RAM_SIZE, L"RAM size");
    }

    /*! @brief Conversions: payload size, DLC and message identifiers */
    TEST_METHOD(TestMethod_Driver_Conversions)
    {
      //--- Payload size of a FIFO
      const uint8_t Payloads[8] = { 8, 12, 16, 20, 24, 32, 48, 64 };
      for (unsigned z = 0; z < 8; z++) CHECK_EQ(Payloads[z], MCP251XFD_PayloadToByte((eMCP251XFD_PayloadSize)z), L"PayloadToByte");
      CHECK_EQ(0, MCP251XFD_PayloadToByte(MCP251XFD_PAYLOAD_COUNT), L"PayloadToByte(out of range)");

      //--- DLC to byte count, CAN 2.0 and CAN FD
      const uint8_t Can20[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8,  8,  8,  8,  8,  8,  8,  8 };
      const uint8_t CanFd[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };
      for (unsigned z = 0; z < 16; z++)
      {
        CHECK_EQ(Can20[z], MCP251XFD_DLCToByte((TDLC)z, false), L"DLCToByte(CAN2.0)");
        CHECK_EQ(CanFd[z], MCP251XFD_DLCToByte((TDLC)z, true), L"DLCToByte(CAN FD)");
      }
      CHECK_EQ(0, MCP251XFD_DLCToByte((TDLC)16, true), L"DLCToByte(16)");
      CHECK_EQ(0, MCP251XFD_DLCToByte((TDLC)255, false), L"DLCToByte(255)");

      //--- Identifier of a message to identifier of an object (T0/R0): SID<10:0> = bits 10..0, EID<17:0> = bits 28..11, SID11 = bit 29
      CHECK_EQ(0x00000123u, MCP251XFD_MessageIDtoObjectMessageIdentifier(0x123u, false, false), L"standard ID");
      CHECK_EQ(0x000007FFu, MCP251XFD_MessageIDtoObjectMessageIdentifier(0x7FFu, false, false), L"standard ID max");
      CHECK_EQ(0x000007FFu, MCP251XFD_MessageIDtoObjectMessageIdentifier(0xFFFFF7FFu, false, false), L"standard ID, upper bits are ignored");
      // Extended ID 0x1ABCDEF0: SID = ID >> 18 = 0x6AF, EID = ID & 0x3FFFF = 0x0DEF0 => T0 = (0x1DEF0 << 11) | 0x6AF
      CHECK_EQ(((0x0DEF0u << 11) | 0x6AFu), MCP251XFD_MessageIDtoObjectMessageIdentifier(0x1ABCDEF0u, true, false), L"extended ID");
      CHECK_EQ(0x1FFFFFFFu, MCP251XFD_MessageIDtoObjectMessageIdentifier(0x1FFFFFFFu, true, false), L"extended ID max");
      CHECK_EQ(0x20000555u, MCP251XFD_MessageIDtoObjectMessageIdentifier(0x555u | 0x800u, false, true), L"standard ID with SID11");
      CHECK_EQ(0x00000555u, MCP251XFD_MessageIDtoObjectMessageIdentifier(0x555u, false, true), L"standard ID with SID11 at 0");
      CHECK_EQ((0x20000000u | (0x0DEF0u << 11) | 0x6AFu), MCP251XFD_MessageIDtoObjectMessageIdentifier(0x1ABCDEF0u | 0x20000000u, true, true), L"extended ID with SID11");

      //--- Back, the reverse operation
      CHECK_EQ(0x123u, MCP251XFD_ObjectMessageIdentifierToMessageID(0x00000123u, false, false), L"standard ID back");
      CHECK_EQ(0x1ABCDEF0u, MCP251XFD_ObjectMessageIdentifierToMessageID(((0x0DEF0u << 11) | 0x6AFu), true, false), L"extended ID back");
      CHECK_EQ(0x555u | 0x800u, MCP251XFD_ObjectMessageIdentifierToMessageID(0x20000555u, false, true), L"standard ID with SID11 back");
      const uint32_t Ids[] = { 0u, 1u, 0x7FFu, 0x800u, 0x12345u, 0x1FFFFFFFu, 0x15555555u, 0x0AAAAAAAu };
      for (size_t z = 0; z < sizeof(Ids) / sizeof(Ids[0]); z++)
      {
        const uint32_t ext = Ids[z] & 0x1FFFFFFFu, std11 = Ids[z] & 0x7FFu;
        CHECK_EQ(ext, MCP251XFD_ObjectMessageIdentifierToMessageID(MCP251XFD_MessageIDtoObjectMessageIdentifier(ext, true, false), true, false), L"extended ID round trip");
        CHECK_EQ(std11, MCP251XFD_ObjectMessageIdentifierToMessageID(MCP251XFD_MessageIDtoObjectMessageIdentifier(std11, false, false), false, false), L"standard ID round trip");
        const uint32_t std12 = Ids[z] & 0xFFFu;
        CHECK_EQ(std12, MCP251XFD_ObjectMessageIdentifierToMessageID(MCP251XFD_MessageIDtoObjectMessageIdentifier(std12, false, true), false, true), L"standard ID with SID11 round trip");
        const uint32_t ext30 = Ids[z] & 0x3FFFFFFFu;
        CHECK_EQ(ext30, MCP251XFD_ObjectMessageIdentifierToMessageID(MCP251XFD_MessageIDtoObjectMessageIdentifier(ext30, true, true), true, true), L"extended ID with SID11 round trip");
      }
    }

    /*! @brief Access to the registers: little endian, 8/16/32 bits, SFR and RAM */
    TEST_METHOD(TestMethod_Driver_RegisterAccess)
    {
      Setup();
      uint8_t v8 = 0; uint16_t v16 = 0; uint32_t v32 = 0;

      CHECK_OK(MCP251XFD_WriteSFR32(&Comp, RegMCP251XFD_CiTBC, 0x11223344u), L"WriteSFR32");
      CHECK_EQ(0x44, Get8(0x010), L"WriteSFR32 byte 0"); CHECK_EQ(0x33, Get8(0x011), L"WriteSFR32 byte 1"); CHECK_EQ(0x22, Get8(0x012), L"WriteSFR32 byte 2"); CHECK_EQ(0x11, Get8(0x013), L"WriteSFR32 byte 3");
      CHECK_OK(MCP251XFD_ReadSFR32(&Comp, RegMCP251XFD_CiTBC, &v32), L"ReadSFR32");  CHECK_EQ(0x11223344u, v32, L"ReadSFR32");
      CHECK_OK(MCP251XFD_ReadSFR16(&Comp, RegMCP251XFD_CiTBC, &v16), L"ReadSFR16");  CHECK_EQ(0x3344u, v16, L"ReadSFR16 low");
      CHECK_OK(MCP251XFD_ReadSFR16(&Comp, (uint16_t)(RegMCP251XFD_CiTBC + 2), &v16), L"ReadSFR16"); CHECK_EQ(0x1122u, v16, L"ReadSFR16 high");
      CHECK_OK(MCP251XFD_ReadSFR8(&Comp, (uint16_t)(RegMCP251XFD_CiTBC + 3), &v8), L"ReadSFR8");     CHECK_EQ(0x11, v8, L"ReadSFR8");
      CHECK_OK(MCP251XFD_WriteSFR16(&Comp, (uint16_t)(RegMCP251XFD_CiTBC + 1), 0xAABBu), L"WriteSFR16");
      CHECK_EQ(0x11AABB44u, Get32(0x010), L"WriteSFR16 in the middle of a register");
      CHECK_OK(MCP251XFD_WriteSFR8(&Comp, (uint16_t)(RegMCP251XFD_CiTBC + 3), 0x5Au), L"WriteSFR8");
      CHECK_EQ(0x5AAABB44u, Get32(0x010), L"WriteSFR8");

      CHECK_OK(MCP251XFD_WriteRAM32(&Comp, 0x7FC, 0xCAFEBABEu), L"WriteRAM32");
      CHECK_EQ(0xCAFEBABEu, Get32(0x7FC), L"WriteRAM32");
      CHECK_OK(MCP251XFD_ReadRAM32(&Comp, 0x7FC, &v32), L"ReadRAM32"); CHECK_EQ(0xCAFEBABEu, v32, L"ReadRAM32");

      //--- Block access longer than the transfer buffer (the driver cuts it)
      uint8_t In[300], Out[300];
      for (unsigned z = 0; z < sizeof In; z++) In[z] = (uint8_t)(z * 7u + 1u);
      CHECK_OK(MCP251XFD_WriteData(&Comp, 0x500, In, 256), L"WriteData 256 bytes");
      CHECK_EQ(0, memcmp(&Dev.Mem[0x500], In, 256), L"WriteData 256 bytes content");
      CHECK_EQ(0, Get8(0x600), L"WriteData does not write after the end");
      memset(Out, 0, sizeof Out);
      CHECK_OK(MCP251XFD_ReadData(&Comp, 0x500, Out, 256), L"ReadData 256 bytes");
      CHECK_EQ(0, memcmp(Out, In, 256), L"ReadData 256 bytes content");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ReadData(&Comp, MCP251XFD_END_ADDR + 1, Out, 4), L"ReadData after the end of the memory");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_WriteData(&Comp, MCP251XFD_END_ADDR + 1, In, 4), L"WriteData after the end of the memory");
    }

    /*! @brief CAN controller configuration: CiCON and the SID11 bit of CiTDC */
    TEST_METHOD(TestMethod_Driver_ConfigureCANController)
    {
      // CiCON bits: ISOCRCEN=5, PXEDIS=6, BRSDIS=12, RTXAT=16, ESIGM=17, SERR2LOM=18, OPMOD=23..21, REQOP=26..24, ABAT=27, TXBWS=31..28
      const uint32_t Kept = 0x04980760u & ~((1u << 5) | (1u << 6) | (1u << 12) | (1u << 16) | (1u << 17) | (1u << 18) | (0xFu << 28) | (1u << 27)); // The bits the function does not touch
      struct Row { unsigned flags; uint32_t bits; } Rows[] =
      {
        { 0x00, 0 },
        { 0x01, 1u << 18 },  // Listen only mode on system error => SERR2LOM
        { 0x02, 1u << 17 },  // Gateway mode ESI recessive => ESIGM
        { 0x04, 1u << 16 },  // Restricted retransmission attempts => RTXAT
        { 0x08, 1u << 12 },  // Bit rate switching disabled => BRSDIS
        { 0x10, 1u << 6  },  // Protocol exception as form error => PXEDIS
        { 0x20, 1u << 5  },  // ISO CRC => ISOCRCEN
        { 0x3F, (1u << 18) | (1u << 17) | (1u << 16) | (1u << 12) | (1u << 6) | (1u << 5) },
      };
      for (size_t z = 0; z < sizeof(Rows) / sizeof(Rows[0]); z++)
      {
        Setup();
        CHECK_OK(MCP251XFD_ConfigureCANController(&Comp, (setMCP251XFD_CANCtrlFlags)Rows[z].flags, MCP251XFD_DELAY_16BIT_TIMES), L"ConfigureCANController");
        const uint32_t Expected = Kept | Rows[z].bits | (4u << 24) | (4u << 21) | (0x4u << 28); // REQOP = OPMOD = configuration mode, TXBWS = 0b0100
        CHECK_EQ(Expected, Get32(0x000), L"CiCON");
        CHECK_EQ(0, Get8(0x00F) & 0x01, L"SID11 not asked");
        CHECK_EQ(0, Comp.InternalConfig & MCP251XFD_CANFD_USE_RRS_BIT_AS_SID11, L"InternalConfig without SID11");
      }
      //--- The bandwidth sharing
      for (unsigned bw = 0; bw <= 12; bw++)
      {
        Setup();
        CHECK_OK(MCP251XFD_ConfigureCANController(&Comp, (setMCP251XFD_CANCtrlFlags)0, (eMCP251XFD_Bandwidth)bw), L"ConfigureCANController bandwidth");
        CHECK_EQ(bw, Get8(0x003) >> 4, L"TXBWS");
      }
      //--- SID11: bit 24 of CiTDC
      Setup();
      CHECK_OK(MCP251XFD_ConfigureCANController(&Comp, MCP251XFD_CANFD_USE_RRS_BIT_AS_SID11, MCP251XFD_NO_DELAY), L"ConfigureCANController SID11");
      CHECK_EQ(0x01, Get8(0x00F) & 0x01, L"SID11EN");
      CHECK_EQ(MCP251XFD_CANFD_USE_RRS_BIT_AS_SID11, Comp.InternalConfig & MCP251XFD_CANFD_USE_RRS_BIT_AS_SID11, L"InternalConfig with SID11");
      CHECK_OK(MCP251XFD_ConfigureCANController(&Comp, (setMCP251XFD_CANCtrlFlags)0, MCP251XFD_NO_DELAY), L"ConfigureCANController without SID11");
      CHECK_EQ(0x00, Get8(0x00F) & 0x01, L"SID11EN cleared");
      CHECK_EQ(0, Comp.InternalConfig & MCP251XFD_CANFD_USE_RRS_BIT_AS_SID11, L"InternalConfig SID11 cleared");
      //--- The device must be in Configuration mode
      Setup(); SetMode(MCP251XFD_NORMAL_CANFD_MODE);
      CHECK_ERR(ERR__NEED_CONFIG_MODE, MCP251XFD_ConfigureCANController(&Comp, (setMCP251XFD_CANCtrlFlags)0, MCP251XFD_NO_DELAY), L"ConfigureCANController in normal mode");
    }

    /*! @brief Operation mode */
    TEST_METHOD(TestMethod_Driver_OperationMode)
    {
      Setup();
      eMCP251XFD_OperationMode Mode = MCP251XFD_NORMAL_CANFD_MODE;
      CHECK_OK(MCP251XFD_GetActualOperationMode(&Comp, &Mode), L"GetActualOperationMode");
      CHECK_EQ(MCP251XFD_CONFIGURATION_MODE, Mode, L"mode after the reset");

      //--- CAN FD mode is refused if the bit time is not configured for CAN FD
      CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_RequestOperationMode(&Comp, MCP251XFD_NORMAL_CANFD_MODE, true), L"normal CAN FD mode without CAN FD configuration");
      //--- All the other modes
      const eMCP251XFD_OperationMode Modes[] = { MCP251XFD_NORMAL_CAN20_MODE, MCP251XFD_LISTEN_ONLY_MODE, MCP251XFD_INTERNAL_LOOPBACK_MODE, MCP251XFD_EXTERNAL_LOOPBACK_MODE, MCP251XFD_RESTRICTED_OPERATION_MODE, MCP251XFD_CONFIGURATION_MODE };
      for (size_t z = 0; z < sizeof(Modes) / sizeof(Modes[0]); z++)
      {
        CHECK_OK(MCP251XFD_RequestOperationMode(&Comp, Modes[z], true), L"RequestOperationMode");
        CHECK_EQ(Modes[z], Get8(0x003) & 0x07, L"REQOP");
        CHECK_OK(MCP251XFD_GetActualOperationMode(&Comp, &Mode), L"GetActualOperationMode");
        CHECK_EQ(Modes[z], Mode, L"OPMOD");
      }
      //--- With the CAN FD configuration
      Comp.InternalConfig |= MCP251XFD_CANFD_ENABLED;
      CHECK_OK(MCP251XFD_StartCANFD(&Comp), L"StartCANFD");
      CHECK_EQ(MCP251XFD_NORMAL_CANFD_MODE, Get8(0x003) & 0x07, L"REQOP of StartCANFD");
      CHECK_OK(MCP251XFD_StartCAN20(&Comp), L"StartCAN20");
      CHECK_EQ(MCP251XFD_NORMAL_CAN20_MODE, Get8(0x003) & 0x07, L"REQOP of StartCAN20");
      CHECK_OK(MCP251XFD_StartCANListenOnly(&Comp), L"StartCANListenOnly");
      CHECK_EQ(MCP251XFD_LISTEN_ONLY_MODE, Get8(0x003) & 0x07, L"REQOP of StartCANListenOnly");
      //--- Abort all transmissions
      CHECK_OK(MCP251XFD_AbortAllTransmissions(&Comp), L"AbortAllTransmissions");
      CHECK_EQ(0x08, Get8(0x003) & 0x08, L"ABAT");
      //--- The mode does not change: timeout of a few ms
      Dev.FreezeMode = true; Dev.Ms = 0;
      CHECK_ERR(ERR__DEVICE_TIMEOUT, MCP251XFD_RequestOperationMode(&Comp, MCP251XFD_NORMAL_CAN20_MODE, true), L"mode that never changes");
      CHECK_EQ(1, Dev.Ms > 5 && Dev.Ms < 100, L"the timeout is a few ms");
      CHECK_OK(MCP251XFD_RequestOperationMode(&Comp, MCP251XFD_NORMAL_CAN20_MODE, false), L"RequestOperationMode without waiting");
    }

    /*! @brief Bit time registers */
    TEST_METHOD(TestMethod_Driver_SetBitTimeConfiguration)
    {
#if API_V2
      CAN_BitTimeConfig Conf; memset(&Conf, 0, sizeof Conf);
      Conf.NBRP = 1; Conf.NTSEG1 = 62; Conf.NTSEG2 = 15; Conf.NSJW = 15;    // Values of the registers (real value - 1)
      Conf.DBRP = 1; Conf.DTSEG1 = 14; Conf.DTSEG2 = 3;  Conf.DSJW = 3;
      Conf.TDCmode = CAN_TDC_AUTO_MODE; Conf.TDCO = 0x10; Conf.TDCV = 0; Conf.EdgeFilter = true; Conf.CAN20only = false;
#  define SET_BITTIME(conf, can20only)  (conf.CAN20only = (can20only), MCP251XFD_SetBitTimeConfiguration(&Comp, &conf))
#  define SET_TDC_MANUAL(conf, tdcv, edge)  do { conf.TDCmode = CAN_TDC_MANUAL_MODE; conf.TDCV = (tdcv); conf.EdgeFilter = (edge); } while (0)
#else
      MCP251XFD_BitTimeConfig Conf; memset(&Conf, 0, sizeof Conf);
      Conf.NBRP = 1; Conf.NTSEG1 = 62; Conf.NTSEG2 = 15; Conf.NSJW = 15;
      Conf.DBRP = 1; Conf.DTSEG1 = 14; Conf.DTSEG2 = 3;  Conf.DSJW = 3;
      Conf.TDCMOD = MCP251XFD_AUTO_MODE; Conf.TDCO = 0x10; Conf.TDCV = 0; Conf.EDGE_FILTER = true;
#  define SET_BITTIME(conf, can20only)  MCP251XFD_SetBitTimeConfiguration(&Comp, &conf, (can20only))
#  define SET_TDC_MANUAL(conf, tdcv, edge)  do { conf.TDCMOD = MCP251XFD_MANUAL_MODE; conf.TDCV = (tdcv); conf.EDGE_FILTER = (edge); } while (0)
#endif
      Setup();
      CHECK_OK(SET_BITTIME(Conf, false), L"SetBitTimeConfiguration");
      // CiNBTCFG: BRP<7:0> = bits 31..24, TSEG1<7:0> = bits 23..16, TSEG2<6:0> = bits 14..8, SJW<6:0> = bits 6..0
      CHECK_EQ((1u << 24) | (62u << 16) | (15u << 8) | 15u, Get32(0x004), L"CiNBTCFG");
      // CiDBTCFG: BRP<7:0> = bits 31..24, TSEG1<4:0> = bits 20..16, TSEG2<3:0> = bits 11..8, SJW<3:0> = bits 3..0
      CHECK_EQ((1u << 24) | (14u << 16) | (3u << 8) | 3u, Get32(0x008), L"CiDBTCFG");
      // CiTDC: TDCV<5:0> = bits 5..0, TDCO<6:0> = bits 14..8, TDCMOD<1:0> = bits 17..16, SID11EN = bit 24, EDGFLTEN = bit 25
      CHECK_EQ((0u << 0) | (0x10u << 8) | (2u << 16) | (1u << 25), Get32(0x00C), L"CiTDC");
      CHECK_EQ(MCP251XFD_CANFD_ENABLED, Comp.InternalConfig & MCP251XFD_CANFD_ENABLED, L"CAN FD enabled in the driver");

      //--- Manual mode, without edge filter, TDCV
      SET_TDC_MANUAL(Conf, 0x2A, false);
      Setup();
      CHECK_OK(SET_BITTIME(Conf, false), L"SetBitTimeConfiguration manual TDC");
      CHECK_EQ((0x2Au << 0) | (0x10u << 8) | (1u << 16), Get32(0x00C), L"CiTDC manual");

      //--- CAN 2.0 only: the data registers are not written and CAN FD is disabled
      Setup(); Put32(0x008, 0x12345678u); Put32(0x00C, 0x9ABCDEF0u); Comp.InternalConfig |= MCP251XFD_CANFD_ENABLED;
      CHECK_OK(SET_BITTIME(Conf, true), L"SetBitTimeConfiguration CAN 2.0 only");
      CHECK_EQ((1u << 24) | (62u << 16) | (15u << 8) | 15u, Get32(0x004), L"CiNBTCFG CAN 2.0 only");
      CHECK_EQ(0x12345678u, Get32(0x008), L"CiDBTCFG untouched");
      CHECK_EQ(0x9ABCDEF0u, Get32(0x00C), L"CiTDC untouched");
      CHECK_EQ(0, Comp.InternalConfig & MCP251XFD_CANFD_ENABLED, L"CAN FD disabled in the driver");
#undef SET_BITTIME
#undef SET_TDC_MANUAL
    }

    /*! @brief TEF, TXQ and FIFO configuration (CiTEFCON, CiTXQCON and CiFIFOCONm) */
    TEST_METHOD(TestMethod_Driver_ConfigureTEF_TXQ_FIFO)
    {
      MCP251XFD_RAMInfos Ram;
      //--- TEF: FSIZE<4:0> = bits 28..24, TEFTSEN = bit 5, TEFOVIE = 3, TEFFIE = 2, TEFHIE = 1, TEFNEIE = 0 ; CiCON.STEF = bit 19
      MCP251XFD_FIFO Tef; memset(&Tef, 0, sizeof Tef); memset(&Ram, 0, sizeof Ram);
      Tef.Name = MCP251XFD_TEF; Tef.Size = MCP251XFD_FIFO_16_MESSAGE_DEEP; Tef.ControlFlags = MCP251XFD_FIFO_ADD_TIMESTAMP_ON_OBJ;
      Tef.InterruptFlags = (eMCP251XFD_FIFOIntFlags)(MCP251XFD_FIFO_OVERFLOW_INT | MCP251XFD_FIFO_EVENT_FIFO_FULL_INT | MCP251XFD_FIFO_EVENT_FIFO_HALF_FULL_INT | MCP251XFD_FIFO_EVENT_FIFO_NOT_EMPTY_INT);
      Tef.RAMInfos = &Ram;
      Setup();
      CHECK_OK(MCP251XFD_ConfigureTEF(&Comp, true, &Tef), L"ConfigureTEF");
      CHECK_EQ((15u << 24) | (1u << 5) | (1u << 3) | (1u << 2) | (1u << 1) | (1u << 0), Get32(0x040), L"CiTEFCON");
      CHECK_EQ(1u << 19, Get32(0x000) & (1u << 19), L"STEF enabled");
      CHECK_EQ(12, Ram.ByteInObject, L"TEF object size (2 words + time stamp)");
      CHECK_EQ(12 * 16, Ram.ByteInFIFO, L"TEF size in RAM");
      CHECK_EQ(MCP251XFD_RAM_ADDR, Ram.RAMStartAddress, L"TEF is the first in RAM");
      Tef.ControlFlags = MCP251XFD_FIFO_NO_CONTROL_FLAGS; Tef.InterruptFlags = MCP251XFD_FIFO_NO_INTERRUPT_FLAGS; Tef.Size = MCP251XFD_FIFO_1_MESSAGE_DEEP;
      CHECK_OK(MCP251XFD_ConfigureTEF(&Comp, true, &Tef), L"ConfigureTEF minimal");
      CHECK_EQ(0u, Get32(0x040), L"CiTEFCON minimal");
      CHECK_EQ(8, Ram.ByteInObject, L"TEF object size without time stamp");
      CHECK_OK(MCP251XFD_ConfigureTEF(&Comp, false, NULL), L"ConfigureTEF disable");
      CHECK_EQ(0u, Get32(0x000) & (1u << 19), L"STEF disabled");
      Tef.Name = MCP251XFD_FIFO1;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureTEF(&Comp, true, &Tef), L"ConfigureTEF with another name");
      Setup(); SetMode(MCP251XFD_NORMAL_CANFD_MODE); Tef.Name = MCP251XFD_TEF;
      CHECK_ERR(ERR__NEED_CONFIG_MODE, MCP251XFD_ConfigureTEF(&Comp, true, &Tef), L"ConfigureTEF in normal mode");

      //--- TXQ: PLSIZE<2:0> = bits 31..29, FSIZE<4:0> = 28..24, TXAT<1:0> = 22..21, TXPRI<4:0> = 20..16, TXATIE = 4, TXQEIE = 2, TXQNIE = 0 ; CiCON.TXQEN = bit 20
      MCP251XFD_FIFO Txq; memset(&Txq, 0, sizeof Txq); memset(&Ram, 0, sizeof Ram);
      Txq.Name = MCP251XFD_TXQ; Txq.Size = MCP251XFD_FIFO_8_MESSAGE_DEEP; Txq.Payload = MCP251XFD_PAYLOAD_32BYTE; Txq.Attempts = MCP251XFD_THREE_ATTEMPTS; Txq.Priority = MCP251XFD_MESSAGE_TX_PRIORITY6;
      Txq.InterruptFlags = (eMCP251XFD_FIFOIntFlags)(MCP251XFD_FIFO_TX_ATTEMPTS_EXHAUSTED_INT | MCP251XFD_FIFO_TRANSMIT_FIFO_EMPTY_INT | MCP251XFD_FIFO_TRANSMIT_FIFO_NOT_FULL_INT);
      Txq.RAMInfos = &Ram;
      Setup();
      CHECK_OK(MCP251XFD_ConfigureTXQ(&Comp, true, &Txq), L"ConfigureTXQ");
      CHECK_EQ((5u << 29) | (7u << 24) | (1u << 21) | (5u << 16) | (1u << 4) | (1u << 2) | (1u << 0), Get32(0x050), L"CiTXQCON");
      CHECK_EQ(1u << 20, Get32(0x000) & (1u << 20), L"TXQEN enabled");
      CHECK_EQ(8 + 32, Ram.ByteInObject, L"TXQ object size");
      CHECK_EQ((8 + 32) * 8, Ram.ByteInFIFO, L"TXQ size in RAM");
      CHECK_OK(MCP251XFD_ConfigureTXQ(&Comp, false, NULL), L"ConfigureTXQ disable");
      CHECK_EQ(0u, Get32(0x000) & (1u << 20), L"TXQEN disabled");
      Txq.Name = MCP251XFD_FIFO2;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureTXQ(&Comp, true, &Txq), L"ConfigureTXQ with another name");
      Txq.Name = MCP251XFD_TXQ; Txq.Size = MCP251XFD_FIFO_32_MESSAGE_DEEP; Txq.Payload = MCP251XFD_PAYLOAD_64BYTE;
      CHECK_ERR(ERR__OUT_OF_MEMORY, MCP251XFD_ConfigureTXQ(&Comp, true, &Txq), L"TXQ larger than the RAM");

      //--- Transmit FIFO: PLSIZE = 29, FSIZE = 24, TXAT = 21, TXPRI = 16, TXEN = 7, RTREN = 6, TXATIE = 4, TFERFFIE = 2, TFHRFHIE = 1, TFNRFNIE = 0
      MCP251XFD_FIFO Fifo; memset(&Fifo, 0, sizeof Fifo); memset(&Ram, 0, sizeof Ram);
      Fifo.Name = MCP251XFD_FIFO3; Fifo.Size = MCP251XFD_FIFO_8_MESSAGE_DEEP; Fifo.Payload = MCP251XFD_PAYLOAD_12BYTE; Fifo.Direction = MCP251XFD_TRANSMIT_FIFO; Fifo.Attempts = MCP251XFD_THREE_ATTEMPTS;
      Fifo.Priority = MCP251XFD_MESSAGE_TX_PRIORITY6; Fifo.ControlFlags = MCP251XFD_FIFO_AUTO_RTR_RESPONSE;
      Fifo.InterruptFlags = (eMCP251XFD_FIFOIntFlags)(MCP251XFD_FIFO_TX_ATTEMPTS_EXHAUSTED_INT | MCP251XFD_FIFO_TRANSMIT_FIFO_EMPTY_INT | MCP251XFD_FIFO_TRANSMIT_FIFO_HALF_EMPTY_INT | MCP251XFD_FIFO_TRANSMIT_FIFO_NOT_FULL_INT);
      Fifo.RAMInfos = &Ram;
      Setup();
      CHECK_OK(MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"ConfigureFIFO transmit");
      CHECK_EQ((1u << 29) | (7u << 24) | (1u << 21) | (5u << 16) | (1u << 7) | (1u << 6) | (1u << 4) | (1u << 2) | (1u << 1) | (1u << 0), Get32(0x074), L"CiFIFOCON3 transmit");
      CHECK_EQ(8 + 12, Ram.ByteInObject, L"transmit FIFO object size");
      CHECK_EQ((8 + 12) * 8, Ram.ByteInFIFO, L"transmit FIFO size in RAM");
      //--- Receive FIFO: RXTSEN = 5, RXOVIE = 3, and the interrupts of the receive FIFO are on the bits 2..0
      memset(&Fifo, 0, sizeof Fifo); memset(&Ram, 0, sizeof Ram);
      Fifo.Name = MCP251XFD_FIFO31; Fifo.Size = MCP251XFD_FIFO_4_MESSAGE_DEEP; Fifo.Payload = MCP251XFD_PAYLOAD_64BYTE; Fifo.Direction = MCP251XFD_RECEIVE_FIFO;
      Fifo.ControlFlags = MCP251XFD_FIFO_ADD_TIMESTAMP_ON_RX;
      Fifo.InterruptFlags = (eMCP251XFD_FIFOIntFlags)(MCP251XFD_FIFO_OVERFLOW_INT | MCP251XFD_FIFO_RECEIVE_FIFO_FULL_INT | MCP251XFD_FIFO_RECEIVE_FIFO_HALF_FULL_INT | MCP251XFD_FIFO_RECEIVE_FIFO_NOT_EMPTY_INT);
      Fifo.RAMInfos = &Ram;
      CHECK_OK(MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"ConfigureFIFO receive");
      CHECK_EQ((7u << 29) | (3u << 24) | (1u << 5) | (1u << 3) | (1u << 2) | (1u << 1) | (1u << 0), Get32(0x1C4), L"CiFIFOCON31 receive");
      CHECK_EQ(8 + 64 + 4, Ram.ByteInObject, L"receive FIFO object size with time stamp");
      CHECK_EQ((8 + 64 + 4) * 4, Ram.ByteInFIFO, L"receive FIFO size in RAM");
      //--- Errors
      Fifo.Name = MCP251XFD_TEF;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"ConfigureFIFO as TEF");
      Fifo.Name = MCP251XFD_TXQ;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"ConfigureFIFO as TXQ");
      Fifo.Name = MCP251XFD_FIFO_COUNT;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"ConfigureFIFO out of range");
      Fifo.Name = MCP251XFD_FIFO1; Fifo.Size = MCP251XFD_FIFO_32_MESSAGE_DEEP;
      CHECK_ERR(ERR__OUT_OF_MEMORY, MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"FIFO larger than the RAM");
      SetMode(MCP251XFD_NORMAL_CANFD_MODE);
      CHECK_ERR(ERR__NEED_CONFIG_MODE, MCP251XFD_ConfigureFIFO(&Comp, &Fifo), L"ConfigureFIFO in normal mode");
    }

    /*! @brief FIFO list: RAM layout (TEF, then TXQ, then FIFO1..31 in order), not listed FIFO are cleared */
    TEST_METHOD(TestMethod_Driver_ConfigureFIFOList)
    {
      MCP251XFD_RAMInfos RamTxq, RamFifo4;
      MCP251XFD_FIFO List[4]; memset(List, 0, sizeof List);
      // Not in the order of the RAM, to check the sort: FIFO4 (receive), TXQ, TEF, FIFO1 (transmit)
      List[0].Name = MCP251XFD_FIFO4; List[0].Size = MCP251XFD_FIFO_8_MESSAGE_DEEP; List[0].Payload = MCP251XFD_PAYLOAD_16BYTE; List[0].Direction = MCP251XFD_RECEIVE_FIFO; List[0].RAMInfos = &RamFifo4;
      List[1].Name = MCP251XFD_TXQ;  List[1].Size = MCP251XFD_FIFO_4_MESSAGE_DEEP; List[1].Payload = MCP251XFD_PAYLOAD_8BYTE;  List[1].RAMInfos = &RamTxq;
      List[2].Name = MCP251XFD_TEF;  List[2].Size = MCP251XFD_FIFO_4_MESSAGE_DEEP; List[2].RAMInfos = NULL;
      List[3].Name = MCP251XFD_FIFO1; List[3].Size = MCP251XFD_FIFO_2_MESSAGE_DEEP; List[3].Payload = MCP251XFD_PAYLOAD_8BYTE; List[3].Direction = MCP251XFD_TRANSMIT_FIFO; List[3].RAMInfos = NULL;
      Setup();
      Put32(0x068, 0x1F1F1F1Fu); Put32(0x074, 0x1F1F1F1Fu); // FIFO2 and FIFO3 are not in the list, they hold an old configuration
      CHECK_OK(MCP251XFD_ConfigureFIFOList(&Comp, List, 4), L"ConfigureFIFOList");
      // RAM: TEF 4 x 8 = 32 bytes at 0x400 ; TXQ 4 x 16 = 64 bytes at 0x420 ; FIFO1 2 x 16 = 32 bytes at 0x460 ; FIFO2 and FIFO3 are 16 bytes each (the minimum) ; FIFO4 at 0x4A0
      CHECK_EQ(MCP251XFD_RAM_ADDR + 0x20u + 0x00u, RamTxq.RAMStartAddress, L"TXQ start address");
      CHECK_EQ(64, RamTxq.ByteInFIFO, L"TXQ size");
      CHECK_EQ(MCP251XFD_RAM_ADDR + 0xA0u, RamFifo4.RAMStartAddress, L"FIFO4 start address");
      CHECK_EQ(24 * 8, RamFifo4.ByteInFIFO, L"FIFO4 size");
      CHECK_EQ(1, List[2].RAMInfos == NULL && List[3].RAMInfos == NULL, L"the temporary RAM information is not left in the list");
      //--- Registers
      CHECK_EQ((3u << 24), Get32(0x040), L"CiTEFCON");
      CHECK_EQ(1u << 19, Get32(0x000) & (1u << 19), L"STEF");
      CHECK_EQ((3u << 24), Get32(0x050) & 0x1F000000u, L"CiTXQCON size");
      CHECK_EQ(1u << 20, Get32(0x000) & (1u << 20), L"TXQEN");
      CHECK_EQ((1u << 24) | (1u << 7), Get32(0x05C) & 0x1F0000FFu, L"CiFIFOCON1");
      CHECK_EQ((7u << 24) | (2u << 29), Get32(0x080) & 0xFF000000u, L"CiFIFOCON4");
      // FIFO2 and FIFO3 are cleared: 1 message deep, 8 bytes payload, receive
      CHECK_EQ(0, (Get32(0x068) >> 24) & 0x1F, L"CiFIFOCON2 size cleared"); CHECK_EQ(0, Get32(0x068) >> 29, L"CiFIFOCON2 payload cleared"); CHECK_EQ(0, Get32(0x068) & 0x80, L"CiFIFOCON2 direction cleared");
      CHECK_EQ(0, (Get32(0x074) >> 24) & 0x1F, L"CiFIFOCON3 size cleared"); CHECK_EQ(0, Get32(0x074) >> 29, L"CiFIFOCON3 payload cleared"); CHECK_EQ(0, Get32(0x074) & 0x80, L"CiFIFOCON3 direction cleared");

      //--- Without TEF and TXQ in the list they are disabled
      Setup(); Put32(0x000, Get32(0x000) | (1u << 19) | (1u << 20));
      MCP251XFD_FIFO One; memset(&One, 0, sizeof One); One.Name = MCP251XFD_FIFO1; One.RAMInfos = NULL;
      CHECK_OK(MCP251XFD_ConfigureFIFOList(&Comp, &One, 1), L"ConfigureFIFOList with one FIFO");
      CHECK_EQ(0, Get32(0x000) & ((1u << 19) | (1u << 20)), L"TEF and TXQ disabled");
      CHECK_EQ(1, One.RAMInfos == NULL, L"RAM information not left");
      //--- Errors
      Setup();
      CHECK_OK(MCP251XFD_ConfigureFIFOList(&Comp, &One, 0), L"ConfigureFIFOList with no FIFO");
      CHECK_EQ(0, Writes.size(), L"nothing written with no FIFO");
      CHECK_ERR(ERR__OUT_OF_RANGE, MCP251XFD_ConfigureFIFOList(&Comp, &One, MCP251XFD_FIFO_CONF_MAX + 1u), L"too many FIFO");
      MCP251XFD_FIFO TwoTef[2]; memset(TwoTef, 0, sizeof TwoTef); TwoTef[0].Name = MCP251XFD_TEF; TwoTef[1].Name = MCP251XFD_TEF;
      CHECK_ERR(ERR__TOO_MANY_TEF, MCP251XFD_ConfigureFIFOList(&Comp, TwoTef, 2), L"two TEF");
      CHECK_EQ(1, TwoTef[0].RAMInfos == NULL, L"RAM information not left after the error");
      MCP251XFD_FIFO TwoTxq[2]; memset(TwoTxq, 0, sizeof TwoTxq); TwoTxq[0].Name = MCP251XFD_TXQ; TwoTxq[1].Name = MCP251XFD_TXQ;
      CHECK_ERR(ERR__TOO_MANY_TXQ, MCP251XFD_ConfigureFIFOList(&Comp, TwoTxq, 2), L"two TXQ");
      MCP251XFD_FIFO Big[3]; memset(Big, 0, sizeof Big);
      for (unsigned z = 0; z < 3; z++) { Big[z].Name = (eMCP251XFD_FIFO)(z + 1); Big[z].Size = MCP251XFD_FIFO_16_MESSAGE_DEEP; Big[z].Payload = MCP251XFD_PAYLOAD_64BYTE; } // 3 x 16 x 72 = 3456 bytes
      CHECK_ERR(ERR__OUT_OF_MEMORY, MCP251XFD_ConfigureFIFOList(&Comp, Big, 3), L"FIFO list larger than the RAM");
      CHECK_EQ(1, Big[0].RAMInfos == NULL && Big[1].RAMInfos == NULL && Big[2].RAMInfos == NULL, L"RAM information not left after the memory error");
    }

    /*! @brief Filters: CiFLTCONm (enable + FIFO pointer), CiFLTOBJm and CiMASKm */
    TEST_METHOD(TestMethod_Driver_ConfigureFilter)
    {
      MCP251XFD_Filter Flt; memset(&Flt, 0, sizeof Flt);
      //--- Standard ID filter 0: FLTOBJ = SID, MASK = SID mask + MIDE (bit 30, the message type is matched), FLTCON = ENABLE (bit 7) | FBP<4:0>
      Flt.Filter = MCP251XFD_FILTER0; Flt.EnableFilter = true; Flt.Match = MCP251XFD_MATCH_ONLY_SID; Flt.PointTo = MCP251XFD_FIFO1; Flt.AcceptanceID = 0x123u; Flt.AcceptanceMask = 0x7FFu; Flt.ExtendedID = false;
      Setup();
      CHECK_OK(MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ConfigureFilter standard");
      CHECK_EQ(0x00000123u, Get32(0x1F0), L"CiFLTOBJ0");
      CHECK_EQ(0x400007FFu, Get32(0x1F4), L"CiMASK0");
      CHECK_EQ(0x81, Get8(0x1D0), L"CiFLTCON0 filter 0");
      //--- Extended ID filter 5 (EXIDE = bit 30 of FLTOBJ), pointing to FIFO3: filter 5 is the second byte of CiFLTCON1
      Flt.Filter = MCP251XFD_FILTER5; Flt.Match = MCP251XFD_MATCH_ONLY_EID; Flt.PointTo = MCP251XFD_FIFO3; Flt.AcceptanceID = 0x1ABCDEF0u; Flt.AcceptanceMask = 0x1FFFFFFFu; Flt.ExtendedID = true;
      CHECK_OK(MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ConfigureFilter extended");
      CHECK_EQ(0x40000000u | 0x06F786AFu, Get32(0x218), L"CiFLTOBJ5");
      CHECK_EQ(0x40000000u | 0x1FFFFFFFu, Get32(0x21C), L"CiMASK5");
      CHECK_EQ(0x83, Get8(0x1D5), L"CiFLTCON1 filter 5");
      //--- Standard and extended: no MIDE, no EXIDE, the ID is a 29-bit one
      Flt.Filter = MCP251XFD_FILTER31; Flt.Match = MCP251XFD_MATCH_SID_EID; Flt.PointTo = MCP251XFD_FIFO31; Flt.AcceptanceID = 0x1ABCDEF0u; Flt.AcceptanceMask = 0x1F0F0F0Fu | 0x1ABCDEF0u;
      CHECK_OK(MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ConfigureFilter standard and extended");
      CHECK_EQ(0x06F786AFu, Get32(0x2E8), L"CiFLTOBJ31");
      CHECK_EQ(MCP251XFD_MessageIDtoObjectMessageIdentifier(0x1F0F0F0Fu | 0x1ABCDEF0u, true, false), Get32(0x2EC), L"CiMASK31");
      CHECK_EQ(0x9F, Get8(0x1EF), L"CiFLTCON7 filter 31");
      //--- A filter already enabled is reconfigured
      Flt.Filter = MCP251XFD_FILTER0; Flt.Match = MCP251XFD_MATCH_ONLY_SID; Flt.PointTo = MCP251XFD_FIFO2; Flt.AcceptanceID = 0x055u; Flt.AcceptanceMask = 0x0FFu;
      CHECK_OK(MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ConfigureFilter again");
      CHECK_EQ(0x00000055u, Get32(0x1F0), L"CiFLTOBJ0 again");
      CHECK_EQ(0x400000FFu, Get32(0x1F4), L"CiMASK0 again");
      CHECK_EQ(0x82, Get8(0x1D0), L"CiFLTCON0 again");
      //--- Disabled filter: only disabled, nothing else changes
      Flt.EnableFilter = false; Flt.AcceptanceID = 0x77u;
      CHECK_OK(MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ConfigureFilter disabled");
      CHECK_EQ(0x00, Get8(0x1D0) & 0x80, L"filter disabled");
      CHECK_EQ(0x00000055u, Get32(0x1F0), L"CiFLTOBJ0 not changed by a disabled filter");

      //--- SID11 (CAN FD base frames use bit 29 of the identifier): the driver accepts a 12-bit standard ID only when it is configured
      Setup(); Flt.EnableFilter = true; Flt.Filter = MCP251XFD_FILTER1; Flt.Match = MCP251XFD_MATCH_ONLY_SID; Flt.PointTo = MCP251XFD_FIFO1; Flt.AcceptanceID = 0x855u; Flt.AcceptanceMask = 0xFFFu;
      CHECK_ERR(ERR__FILTER_TOO_LARGE, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"12-bit ID without SID11");
      Comp.InternalConfig |= MCP251XFD_CANFD_USE_RRS_BIT_AS_SID11 | MCP251XFD_CANFD_ENABLED;
      CHECK_OK(MCP251XFD_ConfigureFilter(&Comp, &Flt), L"12-bit ID with SID11");
      CHECK_EQ(0x20000055u, Get32(0x1F8), L"CiFLTOBJ1 with SID11");
      //--- Errors
      Setup(); Flt.AcceptanceID = 0x123u; Flt.AcceptanceMask = 0x0F0u;
      CHECK_ERR(ERR__FILTER_CONSISTENCY, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ID bits outside of the mask");
      Flt.AcceptanceID = 0x123u; Flt.AcceptanceMask = 0xFFFFu;
      CHECK_ERR(ERR__FILTER_TOO_LARGE, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"mask larger than a standard ID");
      Flt.AcceptanceID = 0x823u; Flt.AcceptanceMask = 0x823u;
      CHECK_ERR(ERR__FILTER_TOO_LARGE, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"ID larger than a standard ID");
      Flt.AcceptanceID = 0x123u; Flt.AcceptanceMask = 0x7FFu;
      Flt.PointTo = MCP251XFD_TXQ;  CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"filter to the TXQ");
      Flt.PointTo = MCP251XFD_TEF;  CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"filter to the TEF");
      Flt.PointTo = MCP251XFD_FIFO_COUNT; CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_ConfigureFilter(&Comp, &Flt), L"filter to a FIFO that does not exist");
#ifdef CHECK_NULL_PARAM
      Flt.PointTo = MCP251XFD_FIFO1;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureFilter(NULL, &Flt), L"ConfigureFilter without component");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureFilter(&Comp, NULL), L"ConfigureFilter without configuration");
#endif
    }

    TEST_METHOD(TestMethod_Driver_FilterList)
    {
      //--- Filters of the list are configured, the other ones are disabled, and the DeviceNet filter is set
      Setup();
      for (unsigned z = 0; z < 32; z++) Put8((uint16_t)(0x1D0 + z), 0x81); // All the filters are enabled, on the FIFO1
      MCP251XFD_Filter List[2]; memset(List, 0, sizeof List);
      List[0].Filter = MCP251XFD_FILTER2; List[0].EnableFilter = true; List[0].Match = MCP251XFD_MATCH_ONLY_SID; List[0].PointTo = MCP251XFD_FIFO4; List[0].AcceptanceID = 0x100u; List[0].AcceptanceMask = 0x700u;
      List[1].Filter = MCP251XFD_FILTER30; List[1].EnableFilter = true; List[1].Match = MCP251XFD_MATCH_ONLY_EID; List[1].PointTo = MCP251XFD_FIFO9; List[1].AcceptanceID = 0x12345u; List[1].AcceptanceMask = 0x1FFFFu;
      CHECK_OK(MCP251XFD_ConfigureFilterList(&Comp, MCP251XFD_D_NET_FILTER_3Bits, List, 2), L"ConfigureFilterList");
      CHECK_EQ(3, Get8(0x000) & 0x1F, L"DNCNT");
      for (unsigned z = 0; z < 32; z++)
      {
        const uint8_t Expected = (z == 2) ? 0x84 : (z == 30) ? 0x89 : 0x01; // The disabled ones keep their FIFO pointer
        CHECK_EQ(Expected, Get8((uint16_t)(0x1D0 + z)), L"CiFLTCONm after the list");
      }
      CHECK_EQ(0x00000100u, Get32(0x200), L"CiFLTOBJ2");
      CHECK_EQ(0x40000700u, Get32(0x204), L"CiMASK2");
      //--- Special cases
      Setup();
      CHECK_OK(MCP251XFD_ConfigureFilterList(&Comp, MCP251XFD_D_NET_FILTER_DISABLE, List, 0), L"empty list");
      CHECK_EQ(0, Writes.size(), L"nothing written for an empty list");
      CHECK_ERR(ERR__OUT_OF_RANGE, MCP251XFD_ConfigureFilterList(&Comp, MCP251XFD_D_NET_FILTER_DISABLE, List, MCP251XFD_FILTER_COUNT + 1u), L"too many filters");
      //--- DisableFilter
      Setup(); Put8(0x1D7, 0x85);
      CHECK_OK(MCP251XFD_DisableFilter(&Comp, MCP251XFD_FILTER7), L"DisableFilter");
      CHECK_EQ(0x05, Get8(0x1D7), L"filter 7 disabled, FIFO pointer kept");
      Writes.clear();
      CHECK_OK(MCP251XFD_DisableFilter(&Comp, MCP251XFD_FILTER7), L"DisableFilter already disabled");
      CHECK_EQ(0, Writes.size(), L"no write to disable a disabled filter");
      CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_DisableFilter(&Comp, MCP251XFD_FILTER_COUNT), L"DisableFilter out of range");
      //--- DeviceNet filter alone
      Setup();
      CHECK_OK(MCP251XFD_ConfigureDeviceNetFilter(&Comp, MCP251XFD_D_NET_FILTER_18Bits), L"ConfigureDeviceNetFilter");
      CHECK_EQ(18, Get8(0x000) & 0x1F, L"DNCNT 18 bits");
      CHECK_EQ(0x60, Get8(0x000) & 0xE0, L"the other bits of CiCON byte 0 are kept");
    }

    /*! @brief Transmit a message: the object is written in the RAM at the address of the FIFO and the FIFO is updated (and flushed) */
    TEST_METHOD(TestMethod_Driver_TransmitMessage)
    {
      // T0 = identifier ; T1 = DLC<3:0>, IDE = bit 4, RTR = 5, BRS = 6, FDF = 7, ESI = 8, SEQ<22:0> = bits 31..9
      uint8_t Data[64]; for (unsigned z = 0; z < 64; z++) Data[z] = (uint8_t)(0x10 + z);
      TMessage Msg; memset(&Msg, 0, sizeof Msg);
      Setup(); memset(&Dev.Mem[0x480], 0xEE, 0x80); Put32(0x064, 0x080u);            // The next message of the FIFO1 is at the offset 0x080 of the RAM
      //--- Standard CAN 2.0 frame of 3 bytes: the payload is padded with 0 to a multiple of 4 bytes
      Msg.MessageID = 0x123u; Msg.MSG_SEQ = 5u; Msg.ControlFlags = (TFlags)0; Msg.DLC = (TDLC)3; Msg.PayloadData = Data;
      CHECK_OK(MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_FIFO1, false), L"TransmitMessageToFIFO standard");
      CHECK_EQ(0x00000123u, Get32(0x480), L"T0");
      CHECK_EQ((5u << 9) | 3u, Get32(0x484), L"T1");
      CHECK_EQ(0x00121110u, Get32(0x488), L"payload 3 bytes + 1 byte of padding");
      CHECK_EQ(0xEE, Get8(0x48C), L"nothing written after the message");
      CHECK_EQ(0x01, Get8(0x05D), L"update only (UINC)");
      //--- With flush (UINC + TXREQ)
      CHECK_OK(MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_FIFO1, true), L"TransmitMessageToFIFO with flush");
      CHECK_EQ(0x03, Get8(0x05D), L"update and flush (UINC + TXREQ)");
      //--- Extended CAN FD frame with bitrate switch and ESI, 12 bytes: no padding
      Put32(0x064, 0x080u); memset(&Dev.Mem[0x480], 0xEE, 0x80);
      Msg.MessageID = 0x1ABCDEF0u; Msg.MSG_SEQ = 0x1234u; Msg.ControlFlags = (TFlags)(FLAG_FD | FLAG_BRS | FLAG_EXT | FLAG_ESI); Msg.DLC = (TDLC)9;
      CHECK_OK(MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_FIFO1, false), L"TransmitMessageToFIFO extended CAN FD");
      CHECK_EQ(0x06F786AFu, Get32(0x480), L"T0 extended");
      CHECK_EQ((0x1234u << 9) | 0x100u | 0x80u | 0x40u | 0x10u | 9u, Get32(0x484), L"T1 CAN FD, BRS, IDE, ESI, DLC 9");
      CHECK_EQ(0, memcmp(&Dev.Mem[0x488], Data, 12), L"payload 12 bytes");
      CHECK_EQ(0xEE, Get8(0x494), L"nothing written after the payload");
      //--- Remote frame without payload
      Put32(0x064, 0x080u); memset(&Dev.Mem[0x480], 0xEE, 0x80);
      Msg.MessageID = 0x7FFu; Msg.MSG_SEQ = 1u; Msg.ControlFlags = (TFlags)FLAG_RTR; Msg.DLC = (TDLC)0; Msg.PayloadData = NULL;
      CHECK_OK(MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_FIFO1, false), L"TransmitMessageToFIFO remote");
      CHECK_EQ(0x000007FFu, Get32(0x480), L"T0 remote");
      CHECK_EQ((1u << 9) | 0x20u, Get32(0x484), L"T1 remote");
      CHECK_EQ(0xEE, Get8(0x488), L"no payload for a remote frame");
      //--- A CAN 2.0 frame cannot carry more than 8 bytes even with a DLC > 8
      Put32(0x064, 0x080u); memset(&Dev.Mem[0x480], 0xEE, 0x80);
      Msg.MessageID = 0x10u; Msg.MSG_SEQ = 0u; Msg.ControlFlags = (TFlags)0; Msg.DLC = (TDLC)15; Msg.PayloadData = Data;
      CHECK_OK(MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_FIFO1, false), L"TransmitMessageToFIFO CAN 2.0 with DLC 15");
      CHECK_EQ(0, memcmp(&Dev.Mem[0x488], Data, 8), L"8 bytes of payload");
      CHECK_EQ(0xEE, Get8(0x490), L"no more than 8 bytes of payload");
      //--- The TXQ
      Setup(); Put32(0x058, 0x0C0u);
      Msg.MessageID = 0x321u; Msg.MSG_SEQ = 9u; Msg.DLC = (TDLC)1; Msg.ControlFlags = (TFlags)0;
      CHECK_OK(MCP251XFD_TransmitMessageToTXQ(&Comp, &Msg, true), L"TransmitMessageToTXQ");
      CHECK_EQ(0x00000321u, Get32(0x4C0), L"T0 of the TXQ");
      CHECK_EQ(0x03, Get8(0x051), L"TXQ update and flush");
      //--- Errors
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_TEF, false), L"transmit to the TEF");
      Msg.DLC = (TDLC)4; Msg.PayloadData = NULL;
      CHECK_ERR(ERR__NO_DATA_AVAILABLE, MCP251XFD_TransmitMessageToFIFO(&Comp, &Msg, MCP251XFD_FIFO1, false), L"a payload is needed");
    }

    /*! @brief Receive a message: the object is read from the RAM at the address of the FIFO and the FIFO is updated */
    TEST_METHOD(TestMethod_Driver_ReceiveMessage)
    {
      // R0 = identifier ; R1 = DLC<3:0>, IDE = bit 4, RTR = 5, BRS = 6, FDF = 7, ESI = 8, FILHIT<4:0> = bits 15..11 ; then the time stamp (if enabled in the FIFO), then the payload
      uint8_t Rx[64];
      TMessage Msg; memset(&Msg, 0, sizeof Msg); Msg.PayloadData = Rx;
      uint32_t Ts = 0;
      Setup(); Put32(0x064, 0x0C0u);                                                   // The next message of the FIFO1 is at 0x4C0
      //--- Standard frame of 5 bytes with a time stamp
      Put32(0x4C0, 0x555u); Put32(0x4C4, (3u << 11) | 5u); Put32(0x4C8, 0xDEADBEEFu); Put32(0x4CC, 0x40302010u); Put32(0x4D0, 0x00000050u);
      memset(Rx, 0, sizeof Rx);
      CHECK_OK(MCP251XFD_ReceiveMessageFromFIFO(&Comp, &Msg, MCP251XFD_PAYLOAD_8BYTE, &Ts, MCP251XFD_FIFO1), L"ReceiveMessageFromFIFO standard");
      CHECK_EQ(0x555u, Msg.MessageID, L"ID");
      CHECK_EQ(0, Msg.ControlFlags, L"no flags");
      CHECK_EQ(5, Msg.DLC, L"DLC");
      CHECK_EQ(0xDEADBEEFu, Ts, L"time stamp");
      const uint8_t Expected1[5] = { 0x10, 0x20, 0x30, 0x40, 0x50 };
      CHECK_EQ(0, memcmp(Rx, Expected1, 5), L"payload");
      CHECK_EQ(0x01, Get8(0x05D), L"FIFO updated (UINC)");
      //--- Extended CAN FD frame with BRS and ESI of 12 bytes, without time stamp
      Setup(); Put32(0x064, 0x0C0u);
      Put32(0x4C0, 0x06F786AFu); Put32(0x4C4, (2u << 11) | 0x100u | 0x80u | 0x40u | 0x10u | 9u);
      for (unsigned z = 0; z < 12; z++) Dev.Mem[0x4C8 + z] = (uint8_t)(0xA0 + z);
      memset(Rx, 0, sizeof Rx);
      CHECK_OK(MCP251XFD_ReceiveMessageFromFIFO(&Comp, &Msg, MCP251XFD_PAYLOAD_16BYTE, NULL, MCP251XFD_FIFO1), L"ReceiveMessageFromFIFO extended CAN FD");
      CHECK_EQ(0x1ABCDEF0u, Msg.MessageID, L"extended ID");
      CHECK_EQ(FLAG_FD | FLAG_BRS | FLAG_EXT | FLAG_ESI, Msg.ControlFlags, L"flags");
      CHECK_EQ(9, Msg.DLC, L"DLC 9");
      CHECK_EQ(0, memcmp(Rx, &Dev.Mem[0x4C8], 12), L"payload 12 bytes");
      //--- Remote frame
      Setup(); Put32(0x064, 0x0C0u); Put32(0x4C0, 0x7FFu); Put32(0x4C4, 0x20u);
      CHECK_OK(MCP251XFD_ReceiveMessageFromFIFO(&Comp, &Msg, MCP251XFD_PAYLOAD_8BYTE, NULL, MCP251XFD_FIFO1), L"ReceiveMessageFromFIFO remote");
      CHECK_EQ(FLAG_RTR, Msg.ControlFlags, L"remote flag");
      CHECK_EQ(0, Msg.DLC, L"DLC 0");
      //--- The payload of the FIFO (8 bytes) is smaller than the DLC (64 bytes): only 8 bytes are copied
      Setup(); Put32(0x064, 0x0C0u); Put32(0x4C0, 0x100u); Put32(0x4C4, 0x80u | 15u);
      for (unsigned z = 0; z < 8; z++) Dev.Mem[0x4C8 + z] = (uint8_t)(0x30 + z);
      memset(Rx, 0xEE, sizeof Rx);
      CHECK_OK(MCP251XFD_ReceiveMessageFromFIFO(&Comp, &Msg, MCP251XFD_PAYLOAD_8BYTE, NULL, MCP251XFD_FIFO1), L"ReceiveMessageFromFIFO with a payload larger than the FIFO");
      CHECK_EQ(15, Msg.DLC, L"DLC 15");
      CHECK_EQ(0, memcmp(Rx, &Dev.Mem[0x4C8], 8), L"8 bytes copied");
      CHECK_EQ(0xEE, Rx[8], L"no more than 8 bytes copied");
      //--- TEF: identifier and sequence of the transmitted message, TE1 = flags + SEQ<22:0> = bits 31..9
      Setup(); Put32(0x048, 0x0F0u); Put32(0x4F0, 0x234u); Put32(0x4F4, (0x55u << 9) | 0x80u | 2u); Put32(0x4F8, 0x12345678u);
      CHECK_OK(MCP251XFD_ReceiveMessageFromTEF(&Comp, &Msg, &Ts), L"ReceiveMessageFromTEF");
      CHECK_EQ(0x234u, Msg.MessageID, L"TEF ID");
      CHECK_EQ(0x55u, Msg.MSG_SEQ, L"TEF sequence");
      CHECK_EQ(2, Msg.DLC, L"TEF DLC");
      CHECK_EQ(FLAG_FD, Msg.ControlFlags, L"TEF flags");
      CHECK_EQ(0x12345678u, Ts, L"TEF time stamp");
      CHECK_EQ(0x01, Get8(0x041), L"TEF updated (UINC)");
      //--- Errors
      Setup(); Put32(0x064, 0x0C0u); Put32(0x4C4, 4u);
      Msg.PayloadData = NULL;
      CHECK_ERR(ERR__NO_DATA_AVAILABLE, MCP251XFD_ReceiveMessageFromFIFO(&Comp, &Msg, MCP251XFD_PAYLOAD_8BYTE, NULL, MCP251XFD_FIFO1), L"a buffer is needed for the payload");
      Msg.PayloadData = Rx;
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ReceiveMessageFromFIFO(&Comp, &Msg, MCP251XFD_PAYLOAD_8BYTE, NULL, MCP251XFD_TXQ), L"receive from the TXQ");
    }

    /*! @brief FIFO operations: reset, update, flush, status, next address */
    TEST_METHOD(TestMethod_Driver_FIFOOperations)
    {
      //--- Reset: in Configuration mode the FIFO are reset by the mode, nothing is written
      Setup();
      CHECK_OK(MCP251XFD_ResetFIFO(&Comp, MCP251XFD_FIFO3), L"ResetFIFO in Configuration mode");
      CHECK_EQ(0, Writes.size(), L"nothing written to reset a FIFO in Configuration mode");
      //--- Reset in the other modes: FRESET = bit 10 of the control register, it is self-cleared by the component
      Setup(); SetMode(MCP251XFD_NORMAL_CANFD_MODE);
      CHECK_OK(MCP251XFD_ResetFIFO(&Comp, MCP251XFD_FIFO3), L"ResetFIFO");
      CHECK_EQ(1, Writes.size(), L"one write to reset FIFO3"); CHECK_EQ(0x075, Writes.empty() ? 0 : Writes[0], L"FRESET address of FIFO3");
      Writes.clear(); CHECK_OK(MCP251XFD_ResetTXQ(&Comp), L"ResetTXQ");           CHECK_EQ(0x051, Writes.empty() ? 0 : Writes[0], L"FRESET address of the TXQ");
      Writes.clear(); CHECK_OK(MCP251XFD_ResetTEF(&Comp), L"ResetTEF");           CHECK_EQ(0x041, Writes.empty() ? 0 : Writes[0], L"FRESET address of the TEF");
      Writes.clear(); CHECK_OK(MCP251XFD_ResetFIFO(&Comp, MCP251XFD_FIFO31), L"ResetFIFO 31"); CHECK_EQ(0x1C5, Writes.empty() ? 0 : Writes[0], L"FRESET address of FIFO31");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ResetFIFO(&Comp, MCP251XFD_FIFO_COUNT), L"ResetFIFO out of range");

      //--- Update: UINC = bit 8 of the control register, TXREQ = bit 9
      Setup();
      CHECK_OK(MCP251XFD_UpdateFIFO(&Comp, MCP251XFD_FIFO3, false), L"UpdateFIFO");     CHECK_EQ(0x01, Get8(0x075), L"UINC of FIFO3");
      CHECK_OK(MCP251XFD_UpdateFIFO(&Comp, MCP251XFD_FIFO3, true), L"UpdateFIFO flush"); CHECK_EQ(0x03, Get8(0x075), L"UINC + TXREQ of FIFO3");
      CHECK_OK(MCP251XFD_UpdateTXQ(&Comp, true), L"UpdateTXQ");                        CHECK_EQ(0x03, Get8(0x051), L"UINC + TXREQ of the TXQ");
      CHECK_OK(MCP251XFD_UpdateTEF(&Comp), L"UpdateTEF");                              CHECK_EQ(0x01, Get8(0x041), L"UINC of the TEF");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_UpdateFIFO(&Comp, MCP251XFD_FIFO_COUNT, false), L"UpdateFIFO out of range");

      //--- Flush: the bit n of CiTXREQ requests the transmission of the FIFO n (bit 0 is the TXQ)
      struct Flush { eMCP251XFD_FIFO name; uint16_t address; uint8_t value; } Flushes[] =
      {
        { MCP251XFD_TXQ, 0x030, 0x01 }, { MCP251XFD_FIFO1, 0x030, 0x02 }, { MCP251XFD_FIFO2, 0x030, 0x04 }, { MCP251XFD_FIFO7, 0x030, 0x80 },
        { MCP251XFD_FIFO8, 0x031, 0x01 }, { MCP251XFD_FIFO9, 0x031, 0x02 }, { MCP251XFD_FIFO15, 0x031, 0x80 },
        { MCP251XFD_FIFO16, 0x032, 0x01 }, { MCP251XFD_FIFO24, 0x033, 0x01 }, { MCP251XFD_FIFO31, 0x033, 0x80 },
      };
      for (size_t z = 0; z < sizeof(Flushes) / sizeof(Flushes[0]); z++)
      {
        Setup();
        CHECK_OK(MCP251XFD_FlushFIFO(&Comp, Flushes[z].name), L"FlushFIFO");
        CHECK_EQ(Flushes[z].value, Get8(Flushes[z].address), L"CiTXREQ byte of the flushed FIFO");
        CHECK_EQ(0, Get32(0x030) & ~((uint32_t)Flushes[z].value << ((Flushes[z].address - 0x030) * 8)), L"no other FIFO flushed");
      }
      Setup(); CHECK_OK(MCP251XFD_FlushTXQ(&Comp), L"FlushTXQ"); CHECK_EQ(0x00000001u, Get32(0x030), L"FlushTXQ");
      Setup(); CHECK_OK(MCP251XFD_FlushAllFIFO(&Comp), L"FlushAllFIFO"); CHECK_EQ(0xFFFFFFFFu, Get32(0x030), L"FlushAllFIFO");
      CHECK_ERR(ERR__NOT_AVAILABLE, MCP251XFD_FlushFIFO(&Comp, MCP251XFD_TEF), L"the TEF cannot be flushed");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_FlushFIFO(&Comp, MCP251XFD_FIFO_COUNT), L"FlushFIFO out of range");

      //--- Status flags, next message address and index
      Setup(); setMCP251XFD_FIFOstatus Status = (setMCP251XFD_FIFOstatus)0;
      Put8(0x078, 0x55); Put8(0x054, 0x66); Put8(0x044, 0x77);
      CHECK_OK(MCP251XFD_GetFIFOStatus(&Comp, MCP251XFD_FIFO3, &Status), L"GetFIFOStatus"); CHECK_EQ(0x55, Status, L"status of FIFO3");
      CHECK_OK(MCP251XFD_GetFIFOStatus(&Comp, MCP251XFD_TXQ, &Status), L"GetFIFOStatus TXQ"); CHECK_EQ(0x66, Status, L"status of the TXQ");
      CHECK_OK(MCP251XFD_GetFIFOStatus(&Comp, MCP251XFD_TEF, &Status), L"GetFIFOStatus TEF"); CHECK_EQ(0x77, Status, L"status of the TEF");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_GetFIFOStatus(&Comp, MCP251XFD_FIFO_COUNT, &Status), L"GetFIFOStatus out of range");
      uint32_t Address = 0; uint8_t Index = 0;
      Put32(0x070, 0x120u); Put8(0x06D, 0xE5); Put32(0x058, 0x0A0u); Put8(0x055, 0x1B); Put32(0x048, 0x010u);
      CHECK_OK(MCP251XFD_GetNextMessageAddressFIFO(&Comp, MCP251XFD_FIFO2, &Address, &Index), L"GetNextMessageAddressFIFO");
      CHECK_EQ(0x120u, Address, L"user address of FIFO2"); CHECK_EQ(0x05, Index, L"message index of FIFO2 (5 bits)");
      CHECK_OK(MCP251XFD_GetNextMessageAddressTXQ(&Comp, &Address, &Index), L"GetNextMessageAddressTXQ");
      CHECK_EQ(0x0A0u, Address, L"user address of the TXQ"); CHECK_EQ(0x1B, Index, L"message index of the TXQ");
      CHECK_OK(MCP251XFD_GetNextMessageAddressTEF(&Comp, &Address), L"GetNextMessageAddressTEF");
      CHECK_EQ(0x010u, Address, L"user address of the TEF");
      CHECK_ERR(ERR__NOT_AVAILABLE, MCP251XFD_GetNextMessageAddressFIFO(&Comp, MCP251XFD_TEF, &Address, &Index), L"the TEF has no message index");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_GetNextMessageAddressFIFO(&Comp, MCP251XFD_FIFO_COUNT, &Address, &Index), L"GetNextMessageAddressFIFO out of range");

      //--- Clear the events of the FIFO: RXOVIF = bit 3, TXATIF = bit 4
      Setup(); Put8(0x060, 0xFF); Put8(0x054, 0xFF); Put8(0x044, 0xFF);
      CHECK_OK(MCP251XFD_ClearFIFOOverflowEvent(&Comp, MCP251XFD_FIFO1), L"ClearFIFOOverflowEvent"); CHECK_EQ(0xF7, Get8(0x060), L"RXOVIF of FIFO1 cleared");
      CHECK_OK(MCP251XFD_ClearFIFOAttemptsEvent(&Comp, MCP251XFD_FIFO1), L"ClearFIFOAttemptsEvent"); CHECK_EQ(0xE7, Get8(0x060), L"TXATIF of FIFO1 cleared");
      CHECK_OK(MCP251XFD_ClearTXQAttemptsEvent(&Comp), L"ClearTXQAttemptsEvent");                    CHECK_EQ(0xEF, Get8(0x054), L"TXATIF of the TXQ cleared");
      CHECK_OK(MCP251XFD_ClearTEFOverflowEvent(&Comp), L"ClearTEFOverflowEvent");                    CHECK_EQ(0xF7, Get8(0x044), L"RXOVIF of the TEF cleared");
    }

    /*! @brief Interrupts: CiINT (flags = bits 15..0, enables = bits 31..16), CiVEC, status of all the FIFO */
    TEST_METHOD(TestMethod_Driver_Interrupts)
    {
      // Interrupt enables are the bits 15..0 of the upper half of CiINT: TXIE=0, RXIE=1, TBCIE=2, MODIE=3, TEFIE=4, ECCIE=8, SPICRCIE=9, TXATIE=10, RXOVIE=11, SERRIE=12, CERRIE=13, WAKIE=14, IVMIE=15
      CHECK_EQ(0x0001, MCP251XFD_INT_TX_EVENT, L"TXIE");                 CHECK_EQ(0x0002, MCP251XFD_INT_RX_EVENT, L"RXIE");
      CHECK_EQ(0x0004, MCP251XFD_INT_TIME_BASE_COUNTER_EVENT, L"TBCIE"); CHECK_EQ(0x0008, MCP251XFD_INT_OPERATION_MODE_CHANGE_EVENT, L"MODIE");
      CHECK_EQ(0x0010, MCP251XFD_INT_TEF_EVENT, L"TEFIE");               CHECK_EQ(0x0100, MCP251XFD_INT_RAM_ECC_EVENT, L"ECCIE");
      CHECK_EQ(0x0200, MCP251XFD_INT_SPI_CRC_EVENT, L"SPICRCIE");        CHECK_EQ(0x0400, MCP251XFD_INT_TX_ATTEMPTS_EVENT, L"TXATIE");
      CHECK_EQ(0x0800, MCP251XFD_INT_RX_OVERFLOW_EVENT, L"RXOVIE");      CHECK_EQ(0x1000, MCP251XFD_INT_SYSTEM_ERROR_EVENT, L"SERRIE");
      CHECK_EQ(0x2000, MCP251XFD_INT_BUS_ERROR_EVENT, L"CERRIE");        CHECK_EQ(0x4000, MCP251XFD_INT_BUS_WAKEUP_EVENT, L"WAKIE");
      CHECK_EQ(0x8000, MCP251XFD_INT_RX_INVALID_MESSAGE_EVENT, L"IVMIE");

      //--- Configure: the clearable flags (TBCIF, MODIF, SERRIF, CERRIF, WAKIF, IVMIF = 0xF00C; the other ones are read-only) are cleared, then the enables are written
      Setup(); Put16(0x01C, 0xFFFFu);
      CHECK_OK(MCP251XFD_ConfigureInterrupt(&Comp, (setMCP251XFD_InterruptEvents)(MCP251XFD_INT_TX_EVENT | MCP251XFD_INT_RX_EVENT | MCP251XFD_INT_SYSTEM_ERROR_EVENT)), L"ConfigureInterrupt");
      CHECK_EQ(0x0FF3, Get16(0x01C), L"interrupt flags after the configuration");
      CHECK_EQ(0x1003, Get16(0x01E), L"interrupt enables");
      //--- Get and clear
      setMCP251XFD_InterruptEvents Events = (setMCP251XFD_InterruptEvents)0;
      Put16(0x01C, 0xFFFFu);
      CHECK_OK(MCP251XFD_GetInterruptEvents(&Comp, &Events), L"GetInterruptEvents"); CHECK_EQ(0xFFFF, Events, L"GetInterruptEvents");
      CHECK_OK(MCP251XFD_ClearInterruptEvents(&Comp, (setMCP251XFD_InterruptEvents)(MCP251XFD_INT_BUS_WAKEUP_EVENT | MCP251XFD_INT_RX_INVALID_MESSAGE_EVENT)), L"ClearInterruptEvents");
      CHECK_EQ(0x3FFF, Get16(0x01C), L"WAKIF and IVMIF cleared");
      Writes.clear();
      CHECK_OK(MCP251XFD_ClearInterruptEvents(&Comp, (setMCP251XFD_InterruptEvents)(MCP251XFD_INT_TX_EVENT | MCP251XFD_INT_RX_EVENT)), L"ClearInterruptEvents of flags that are not clearable");
      CHECK_EQ(0, Writes.size(), L"TXIF and RXIF cannot be cleared by the software, nothing is written");
      //--- Current event code (ICODE = CiVEC byte 0), and the name of the FIFO that has an interrupt (RXCODE = byte 3, TXCODE = byte 2)
      eMCP251XFD_InterruptFlagCode Code = (eMCP251XFD_InterruptFlagCode)0;
      Put8(0x018, 0x42);
      CHECK_OK(MCP251XFD_GetCurrentInterruptEvent(&Comp, &Code), L"GetCurrentInterruptEvent"); CHECK_EQ(0x42, Code, L"ICODE");
      eMCP251XFD_FIFO Name = MCP251XFD_NO_FIFO; setMCP251XFD_FIFOstatus Flags = (setMCP251XFD_FIFOstatus)0;
      Put8(0x01B, 5); Put8(0x078 + 12 * 2, 0x0A);                                      // RXCODE = FIFO5 ; the status of FIFO5 is at 0x090
      CHECK_OK(MCP251XFD_GetCurrentReceiveFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"GetCurrentReceiveFIFONameAndStatusInterrupt");
      CHECK_EQ(MCP251XFD_FIFO5, Name, L"FIFO that has a receive interrupt"); CHECK_EQ(0x0A, Flags, L"status of that FIFO");
      Put8(0x01B, 0x40);                                                                // No interrupt
      Name = MCP251XFD_FIFO1;
      CHECK_OK(MCP251XFD_GetCurrentReceiveFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"no receive interrupt"); CHECK_EQ(MCP251XFD_NO_FIFO, Name, L"no FIFO");
      Put8(0x01B, 0);   CHECK_ERR(ERR__UNKNOWN_ELEMENT, MCP251XFD_GetCurrentReceiveFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"RXCODE of the TXQ is not a receive FIFO");
      Put8(0x01B, 32);  CHECK_ERR(ERR__UNKNOWN_ELEMENT, MCP251XFD_GetCurrentReceiveFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"RXCODE 32");
      Put8(0x01A, 0);   // TXCODE = TXQ
      CHECK_OK(MCP251XFD_GetCurrentTransmitFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"GetCurrentTransmitFIFONameAndStatusInterrupt TXQ"); CHECK_EQ(MCP251XFD_TXQ, Name, L"TXQ has a transmit interrupt");
      Put8(0x01A, 31);  CHECK_OK(MCP251XFD_GetCurrentTransmitFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"GetCurrentTransmitFIFONameAndStatusInterrupt 31"); CHECK_EQ(MCP251XFD_FIFO31, Name, L"FIFO31 has a transmit interrupt");
      Put8(0x01A, 0x40); CHECK_OK(MCP251XFD_GetCurrentTransmitFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"no transmit interrupt"); CHECK_EQ(MCP251XFD_NO_FIFO, Name, L"no FIFO");
      Put8(0x01A, 33);  CHECK_ERR(ERR__UNKNOWN_ELEMENT, MCP251XFD_GetCurrentTransmitFIFONameAndStatusInterrupt(&Comp, &Name, &Flags), L"TXCODE 33");
      //--- All the FIFO
      setMCP251XFD_InterruptOnFIFO Pending = (setMCP251XFD_InterruptOnFIFO)0, Other = (setMCP251XFD_InterruptOnFIFO)0;
      Put32(0x020, 0x11223344u); Put32(0x028, 0x55667788u); Put32(0x024, 0x99AABBCCu); Put32(0x02C, 0xDDEEFF00u);
      CHECK_OK(MCP251XFD_GetReceiveInterruptStatusOfAllFIFO(&Comp, &Pending, &Other), L"GetReceiveInterruptStatusOfAllFIFO");
      CHECK_EQ(0x11223344u, (uint32_t)Pending, L"RXIF"); CHECK_EQ(0x55667788u, (uint32_t)Other, L"RXOVIF");
      CHECK_OK(MCP251XFD_GetTransmitInterruptStatusOfAllFIFO(&Comp, &Pending, &Other), L"GetTransmitInterruptStatusOfAllFIFO");
      CHECK_EQ(0x99AABBCCu, (uint32_t)Pending, L"TXIF"); CHECK_EQ(0xDDEEFF00u, (uint32_t)Other, L"TXATIF");
      CHECK_OK(MCP251XFD_GetTransmitPendingInterruptStatusOfAllFIFO(&Comp, &Pending), L"GetTransmitPendingInterruptStatusOfAllFIFO"); CHECK_EQ(0x99AABBCCu, (uint32_t)Pending, L"TXIF only");
      CHECK_OK(MCP251XFD_GetTransmitAttemptInterruptStatusOfAllFIFO(&Comp, &Other), L"GetTransmitAttemptInterruptStatusOfAllFIFO"); CHECK_EQ(0xDDEEFF00u, (uint32_t)Other, L"TXATIF only");
      CHECK_OK(MCP251XFD_GetReceivePendingInterruptStatusOfAllFIFO(&Comp, &Pending), L"GetReceivePendingInterruptStatusOfAllFIFO"); CHECK_EQ(0x11223344u, (uint32_t)Pending, L"RXIF only");
      CHECK_OK(MCP251XFD_GetReceiveOverflowInterruptStatusOfAllFIFO(&Comp, &Other), L"GetReceiveOverflowInterruptStatusOfAllFIFO"); CHECK_EQ(0x55667788u, (uint32_t)Other, L"RXOVIF only");
    }

    /*! @brief Pins and GPIO (IOCON: byte 0 = direction, 1 = output level, 2 = input level, 3 = pin mode) */
    TEST_METHOD(TestMethod_Driver_PinsAndGPIO)
    {
      // Pin mode: PM0 = bit 0, PM1 = bit 1, TXCANOD = bit 4, SOF = bit 5, INTOD = bit 6. Direction: TRIS0 = bit 0, TRIS1 = bit 1, XSTBYEN = bit 6
      Setup();
      CHECK_OK(MCP251XFD_ConfigurePins(&Comp, MCP251XFD_PIN_AS_INT0_TX, MCP251XFD_PIN_AS_INT1_RX, MCP251XFD_PINS_PUSHPULL_OUT, MCP251XFD_PINS_PUSHPULL_OUT, false), L"ConfigurePins interrupts");
      CHECK_EQ(0x00, Get8(0xE07), L"pin mode: INT0 and INT1");
      CHECK_EQ(0x00, Get8(0xE04), L"direction");
      CHECK_OK(MCP251XFD_ConfigurePins(&Comp, MCP251XFD_PIN_AS_GPIO0_IN, MCP251XFD_PIN_AS_GPIO1_IN, MCP251XFD_PINS_OPENDRAIN_OUT, MCP251XFD_PINS_OPENDRAIN_OUT, true), L"ConfigurePins GPIO inputs");
      CHECK_EQ(0x73, Get8(0xE07), L"pin mode: GPIO0, GPIO1, TXCAN open drain, SOF, INT open drain");
      CHECK_EQ(0x03, Get8(0xE04), L"direction: both inputs");
      CHECK_OK(MCP251XFD_ConfigurePins(&Comp, MCP251XFD_PIN_AS_GPIO0_OUT, MCP251XFD_PIN_AS_GPIO1_OUT, MCP251XFD_PINS_PUSHPULL_OUT, MCP251XFD_PINS_OPENDRAIN_OUT, false), L"ConfigurePins GPIO outputs");
      CHECK_EQ(0x13, Get8(0xE07), L"pin mode: GPIO0, GPIO1, TXCAN open drain");
      CHECK_EQ(0x00, Get8(0xE04), L"direction: both outputs");
      CHECK_OK(MCP251XFD_ConfigurePins(&Comp, MCP251XFD_PIN_AS_XSTBY, MCP251XFD_PIN_AS_INT1_RX, MCP251XFD_PINS_PUSHPULL_OUT, MCP251XFD_PINS_PUSHPULL_OUT, false), L"ConfigurePins XSTBY");
      CHECK_EQ(0x00, Get8(0xE07), L"pin mode with XSTBY");
      CHECK_EQ(0x40, Get8(0xE04), L"XSTBYEN");

      //--- Change only some pins (the mask is limited to the 2 GPIO)
      Setup();
      CHECK_OK(MCP251XFD_SetGPIOPinsDirection(&Comp, 0x03, 0x01), L"SetGPIOPinsDirection"); CHECK_EQ(0x01, Get8(0xE04), L"only GPIO0 changes");
      CHECK_OK(MCP251XFD_SetGPIOPinsDirection(&Comp, 0x03, 0x02), L"SetGPIOPinsDirection"); CHECK_EQ(0x03, Get8(0xE04), L"GPIO1 changes");
      CHECK_OK(MCP251XFD_SetGPIOPinsDirection(&Comp, 0x00, 0x01), L"SetGPIOPinsDirection"); CHECK_EQ(0x02, Get8(0xE04), L"GPIO0 back to output");
      CHECK_OK(MCP251XFD_SetGPIOPinsDirection(&Comp, 0xFF, 0xFC), L"SetGPIOPinsDirection"); CHECK_EQ(0x02, Get8(0xE04), L"bits other than the 2 GPIO are ignored");
      CHECK_OK(MCP251XFD_SetGPIOPinsOutputLevel(&Comp, 0x03, 0x02), L"SetGPIOPinsOutputLevel"); CHECK_EQ(0x02, Get8(0xE05), L"GPIO1 high");
      CHECK_OK(MCP251XFD_SetGPIOPinsOutputLevel(&Comp, 0x01, 0x01), L"SetGPIOPinsOutputLevel"); CHECK_EQ(0x03, Get8(0xE05), L"GPIO0 high");
      CHECK_OK(MCP251XFD_SetGPIOPinsOutputLevel(&Comp, 0x00, 0x03), L"SetGPIOPinsOutputLevel"); CHECK_EQ(0x00, Get8(0xE05), L"both low");
      uint8_t Levels = 0; Put8(0xE06, 0x02);
      CHECK_OK(MCP251XFD_GetGPIOPinsInputLevel(&Comp, &Levels), L"GetGPIOPinsInputLevel"); CHECK_EQ(0x02, Levels, L"input levels");
    }

    /*! @brief SPI CRC and RAM ECC (registers of the SFR), time stamp, error counters and bus diagnostic */
    TEST_METHOD(TestMethod_Driver_CRC_ECC_TimeStamp_Diagnostic)
    {
      //--- CRC: CRCERRIE = bit 24, FERRIE = bit 25 ; flags CRCERRIF = bit 16, FERRIF = bit 17 ; the CRC of the last mismatch is on the 16 first bits
      Setup();
      CHECK_OK(MCP251XFD_ConfigureCRC(&Comp, MCP251XFD_CRC_ALL_EVENTS), L"ConfigureCRC"); CHECK_EQ(0x03, Get8(0xE0B), L"CRC interrupts enabled");
      CHECK_OK(MCP251XFD_ConfigureCRC(&Comp, MCP251XFD_CRC_FORMERR_EVENT), L"ConfigureCRC"); CHECK_EQ(0x02, Get8(0xE0B), L"CRC format error interrupt only");
      setMCP251XFD_CRCEvents CrcEvents = (setMCP251XFD_CRCEvents)0; uint16_t Last = 0;
      Put32(0xE08, 0x0F03BEEFu);
      CHECK_OK(MCP251XFD_GetCRCEvents(&Comp, &CrcEvents, &Last), L"GetCRCEvents"); CHECK_EQ(0x03, CrcEvents, L"CRC flags (only the 2 flags)"); CHECK_EQ(0xBEEF, Last, L"CRC of the last mismatch");
      CHECK_OK(MCP251XFD_GetCRCEvents(&Comp, &CrcEvents, NULL), L"GetCRCEvents without CRC");
      CHECK_OK(MCP251XFD_ClearCRCEvents(&Comp), L"ClearCRCEvents");
      //--- ECC: ECCEN = bit 0, SECIE = bit 1, DEDIE = bit 2, PARITY = bits 14..8 ; flags SECIF = bit 1, DEDIF = bit 2 ; the address of the last error on the bits 27..16
      Setup();
      CHECK_OK(MCP251XFD_ConfigureECC(&Comp, true, MCP251XFD_ECC_ALL_EVENTS, 0x15), L"ConfigureECC"); CHECK_EQ(0x07, Get8(0xE0C), L"ECC enabled with interrupts"); CHECK_EQ(0x15, Get8(0xE0D), L"parity");
      CHECK_OK(MCP251XFD_ConfigureECC(&Comp, false, MCP251XFD_ECC_SEC_EVENT, 0x00), L"ConfigureECC"); CHECK_EQ(0x02, Get8(0xE0C), L"ECC disabled, single error interrupt"); CHECK_EQ(0x00, Get8(0xE0D), L"parity 0");
      setMCP251XFD_ECCEvents EccEvents = (setMCP251XFD_ECCEvents)0; uint16_t ErrAddress = 0;
      Put32(0xE10, 0x07A40006u);
      CHECK_OK(MCP251XFD_GetECCEvents(&Comp, &EccEvents, &ErrAddress), L"GetECCEvents"); CHECK_EQ(0x06, EccEvents, L"ECC flags"); CHECK_EQ(0x07A4, ErrAddress, L"address of the last ECC error");
      CHECK_OK(MCP251XFD_ClearECCEvents(&Comp), L"ClearECCEvents");

      //--- Time stamp: CiTSCON.TBCPRE<9:0> = bits 9..0, TBCEN = 16, TSEOF = 17, TSRES = 18 ; CiINT.TBCIE = bit 18
      Setup(); Put8(0x01E, 0x80);
      CHECK_OK(MCP251XFD_ConfigureTimeStamp(&Comp, true, MCP251XFD_TS_CAN20_SOF_CANFD_FDF, 40, true), L"ConfigureTimeStamp");
      CHECK_EQ(0x27, Get8(0x014), L"TBCPRE low (40 - 1)"); CHECK_EQ(0x00, Get8(0x015), L"TBCPRE high"); CHECK_EQ(0x05, Get8(0x016), L"TBCEN + TSRES");
      CHECK_EQ(0x84, Get8(0x01E), L"TBCIE set, the other interrupts are kept");
      CHECK_OK(MCP251XFD_ConfigureTimeStamp(&Comp, true, MCP251XFD_TS_CAN20_EOF, 1024, false), L"ConfigureTimeStamp");
      CHECK_EQ(0xFF, Get8(0x014), L"TBCPRE low (1024 - 1)"); CHECK_EQ(0x03, Get8(0x015), L"TBCPRE high"); CHECK_EQ(0x03, Get8(0x016), L"TBCEN + TSEOF");
      CHECK_EQ(0x80, Get8(0x01E), L"TBCIE cleared");
      CHECK_OK(MCP251XFD_ConfigureTimeStamp(&Comp, false, MCP251XFD_TS_CAN20_EOF, 100, false), L"ConfigureTimeStamp disabled");
      CHECK_EQ(0x00, Get8(0x014) | Get8(0x015) | Get8(0x016), L"time stamp disabled");
      Writes.clear();
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureTimeStamp(&Comp, true, MCP251XFD_TS_CAN20_SOF, 0, false), L"prescaler 0");
      CHECK_ERR(ERR__PARAMETER_ERROR, MCP251XFD_ConfigureTimeStamp(&Comp, true, MCP251XFD_TS_CAN20_SOF, 1025, false), L"prescaler 1025");
      CHECK_EQ(0, Writes.size(), L"nothing written with a wrong prescaler");
      uint32_t Stamp = 0;
      CHECK_OK(MCP251XFD_SetTimeStamp(&Comp, 0xA1B2C3D4u), L"SetTimeStamp"); CHECK_EQ(0xA1B2C3D4u, Get32(0x010), L"CiTBC");
      CHECK_OK(MCP251XFD_GetTimeStamp(&Comp, &Stamp), L"GetTimeStamp"); CHECK_EQ(0xA1B2C3D4u, Stamp, L"time stamp read");

      //--- Error counters: CiTREC.REC = byte 0, TEC = byte 1, status flags = byte 2
      Setup(); Put32(0x034, 0x00201A0Fu);
      uint8_t Tec = 0, Rec = 0; eMCP251XFD_TXRXErrorStatus ErrStatus = (eMCP251XFD_TXRXErrorStatus)0;
      CHECK_OK(MCP251XFD_GetTransmitReceiveErrorCountAndStatus(&Comp, &Tec, &Rec, &ErrStatus), L"GetTransmitReceiveErrorCountAndStatus");
      CHECK_EQ(0x1A, Tec, L"transmit error counter"); CHECK_EQ(0x0F, Rec, L"receive error counter"); CHECK_EQ(0x20, ErrStatus, L"error status");
      CHECK_OK(MCP251XFD_GetTransmitErrorCount(&Comp, &Tec), L"GetTransmitErrorCount"); CHECK_EQ(0x1A, Tec, L"TEC");
      CHECK_OK(MCP251XFD_GetReceiveErrorCount(&Comp, &Rec), L"GetReceiveErrorCount");   CHECK_EQ(0x0F, Rec, L"REC");
      //--- Bus diagnostic: 2 registers of 32 bits, the second one is a union of 4 bytes
      Put32(0x038, 0x04030201u); Put32(0x03C, 0x80F1ABCDu);
      MCP251XFD_CiBDIAG0_Register Diag0; MCP251XFD_CiBDIAG1_Register Diag1; memset(&Diag0, 0, sizeof Diag0); memset(&Diag1, 0, sizeof Diag1);
      CHECK_EQ(4, sizeof(MCP251XFD_CiBDIAG1_Register), L"size of the bus diagnostic register");
      CHECK_OK(MCP251XFD_GetBusDiagnostic(&Comp, &Diag0, &Diag1), L"GetBusDiagnostic");
      CHECK_EQ(0x04030201u, Diag0.CiBDIAG0, L"CiBDIAG0"); CHECK_EQ(0x80F1ABCDu, Diag1.CiBDIAG1, L"CiBDIAG1");
      CHECK_EQ(0xABCD, Diag1.Reg.ErrorFreeCounter, L"error free message counter (16 first bits)");
      CHECK_EQ(0x80F1, (unsigned)Diag1.Reg.Flags, L"diagnostic flags (16 last bits)");
      CHECK_EQ(0xABCD, Diag1.Bits.EFMSGCNT, L"EFMSGCNT");
      CHECK_OK(MCP251XFD_ClearBusDiagnostic(&Comp, true, false), L"ClearBusDiagnostic 0"); CHECK_EQ(0u, Get32(0x038), L"CiBDIAG0 cleared"); CHECK_EQ(0x80F1ABCDu, Get32(0x03C), L"CiBDIAG1 not cleared");
      CHECK_OK(MCP251XFD_ClearBusDiagnostic(&Comp, false, true), L"ClearBusDiagnostic 1"); CHECK_EQ(0u, Get32(0x03C), L"CiBDIAG1 cleared");
    }

    /*! @brief Sleep modes */
    TEST_METHOD(TestMethod_Driver_SleepMode)
    {
      bool Sleeping = false; eMCP251XFD_PowerStates From = MCP251XFD_DEVICE_SLEEP_NOT_CONFIGURED;
      //--- Nothing is possible while the sleep is not configured
      Setup(); Comp.InternalConfig = MCP251XFD_DEV_PS_SET(MCP251XFD_DEVICE_SLEEP_NOT_CONFIGURED) | MCP251XFD_DEV_ID_SET(MCP2518FD);
      CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_EnterSleepMode(&Comp), L"EnterSleepMode before the configuration");
      CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_IsDeviceInSleepMode(&Comp, &Sleeping), L"IsDeviceInSleepMode before the configuration");
      CHECK_ERR(ERR__CONFIGURATION, MCP251XFD_WakeUp(&Comp, &From), L"WakeUp before the configuration");
      //--- Configuration: wake-up filter (CiCON: WAKFIL = bit 8, WFT<1:0> = bits 10..9) and bus wake-up interrupt enable (CiINT: WAKIE = bit 14)
      Put8(0x001, 0x08); Put8(0x01F, 0x01);                                            // BUSY (bit 11) and an interrupt enable are already set: they must be kept
      CHECK_OK(MCP251XFD_ConfigureSleepMode(&Comp, false, MCP251XFD_T10FILTER_170ns, true), L"ConfigureSleepMode");
      CHECK_EQ(0x08 | 0x04 | 0x01, Get8(0x001), L"wake-up filter enabled with WFT = 0b10");
      CHECK_EQ(0x40 | 0x01, Get8(0x01F), L"bus wake-up interrupt enabled");
      CHECK_EQ(0, Get8(0xE00) & 0x08, L"LPMEN not set");
      CHECK_OK(MCP251XFD_ConfigureSleepMode(&Comp, false, MCP251XFD_NO_FILTER, false), L"ConfigureSleepMode without filter");
      CHECK_EQ(0x08, Get8(0x001), L"wake-up filter disabled, BUSY kept");
      CHECK_EQ(0x01, Get8(0x01F), L"bus wake-up interrupt disabled");
      //--- Low power mode: MCP2518FD only (OSC.LPMEN = bit 3)
      CHECK_OK(MCP251XFD_ConfigureSleepMode(&Comp, true, MCP251XFD_NO_FILTER, false), L"ConfigureSleepMode low power");
      CHECK_EQ(0x08, Get8(0xE00) & 0x08, L"LPMEN set");
      Comp.InternalConfig = MCP251XFD_DEV_PS_SET(MCP251XFD_DEVICE_NORMAL_POWER_STATE) | MCP251XFD_DEV_ID_SET(MCP2517FD);
      CHECK_ERR(ERR__NOT_SUPPORTED, MCP251XFD_ConfigureSleepMode(&Comp, true, MCP251XFD_NO_FILTER, false), L"low power mode of the MCP2517FD");

      //--- Enter the sleep mode, check it, wake up
      Setup(); Comp.InternalConfig = MCP251XFD_DEV_PS_SET(MCP251XFD_DEVICE_NORMAL_POWER_STATE) | MCP251XFD_DEV_ID_SET(MCP2518FD);
      CHECK_OK(MCP251XFD_EnterSleepMode(&Comp), L"EnterSleepMode");
      CHECK_EQ(MCP251XFD_SLEEP_MODE, Get8(0x003) & 0x07, L"REQOP = sleep");
      CHECK_EQ(MCP251XFD_DEVICE_SLEEP_STATE, MCP251XFD_DEV_PS_GET(Comp.InternalConfig), L"power state in the driver");
      CHECK_ERR(ERR__ALREADY_IN_SLEEP, MCP251XFD_EnterSleepMode(&Comp), L"EnterSleepMode twice");
      Put8(0xE00, (uint8_t)(Get8(0xE00) | 0x04));                                      // OSCDIS: the oscillator is stopped in sleep mode
      CHECK_OK(MCP251XFD_IsDeviceInSleepMode(&Comp, &Sleeping), L"IsDeviceInSleepMode"); CHECK_EQ(1, Sleeping, L"the device sleeps");
      CHECK_OK(MCP251XFD_WakeUp(&Comp, &From), L"WakeUp");
      CHECK_EQ(MCP251XFD_DEVICE_SLEEP_STATE, From, L"WakeUp from sleep");
      CHECK_EQ(0, Get8(0xE00) & 0x04, L"OSCDIS cleared");
      CHECK_EQ(MCP251XFD_DEVICE_NORMAL_POWER_STATE, MCP251XFD_DEV_PS_GET(Comp.InternalConfig), L"power state after wake up");
      CHECK_OK(MCP251XFD_IsDeviceInSleepMode(&Comp, &Sleeping), L"IsDeviceInSleepMode"); CHECK_EQ(0, Sleeping, L"the device is awake");
      //--- Low power sleep: the wake up is done by the chip select, the state cannot be read
      Setup(); Comp.InternalConfig = MCP251XFD_DEV_PS_SET(MCP251XFD_DEVICE_NORMAL_POWER_STATE) | MCP251XFD_DEV_ID_SET(MCP2518FD) | MCP251XFD_SFR_OSC8_LPMEN;
      CHECK_OK(MCP251XFD_EnterSleepMode(&Comp), L"EnterSleepMode low power");
      CHECK_EQ(MCP251XFD_DEVICE_LOWPOWER_SLEEP_STATE, MCP251XFD_DEV_PS_GET(Comp.InternalConfig), L"low power state in the driver");
      CHECK_ERR(ERR__NOT_SUPPORTED, MCP251XFD_IsDeviceInSleepMode(&Comp, &Sleeping), L"IsDeviceInSleepMode in low power sleep");
      CHECK_OK(MCP251XFD_WakeUp(&Comp, &From), L"WakeUp from low power sleep"); CHECK_EQ(MCP251XFD_DEVICE_LOWPOWER_SLEEP_STATE, From, L"WakeUp from low power sleep");
      CHECK_EQ(MCP251XFD_DEVICE_NORMAL_POWER_STATE, MCP251XFD_BusWakeUpFromState(&Comp), L"BusWakeUpFromState after the wake up");
    }

    /*! @brief Device initialization: clocks, SPI, reset, device detection, pins, bit time, CAN controller and interrupts */
    static uint32_t ProgrammedNominalBitrate(uint32_t sysclk)
    {
      const uint32_t v = Get32(0x004);
      const uint32_t brp = (v >> 24) + 1u, tseg1 = ((v >> 16) & 0xFFu) + 1u, tseg2 = ((v >> 8) & 0x7Fu) + 1u;
      return sysclk / (brp * (1u + tseg1 + tseg2));
    }
    static uint32_t ProgrammedDataBitrate(uint32_t sysclk)
    {
      const uint32_t v = Get32(0x008);
      const uint32_t brp = (v >> 24) + 1u, tseg1 = ((v >> 16) & 0x1Fu) + 1u, tseg2 = ((v >> 8) & 0x0Fu) + 1u;
      return sysclk / (brp * (1u + tseg1 + tseg2));
    }
    static void FillInitConfig(MCP251XFD_Config& c, uint32_t xtal, uint32_t osc, eMCP251XFD_CLKINtoSYSCLK sysclkConfig, uint32_t sysclk, uint32_t nominal, uint32_t data, TStats* stats, uint32_t* sysclkResult)
    {
      memset(&c, 0, sizeof c);
      c.XtalFreq = xtal; c.OscFreq = osc; c.SysclkConfig = sysclkConfig; c.ClkoPinConfig = MCP251XFD_CLKO_DivBy10; c.SYSCLK_Result = sysclkResult;
#if API_V2
#  if defined(MCP251XFD_AUTOMATIC_BITRATE_CALCULUS) || defined(CAN_AUTOMATIC_BITRATE_CALCULUS)
      c.BusConfig.DesiredNominalBitrate = nominal; c.BusConfig.DesiredDataBitrate = data; c.BusConfig.NominalSamplePoint = 80; c.BusConfig.DataSamplePoint = 80;
      c.BitTimeStats = stats;
#  else
      CAN_CANFDbusConfig bus; memset(&bus, 0, sizeof bus); bus.DesiredNominalBitrate = nominal; bus.DesiredDataBitrate = data; bus.NominalSamplePoint = 80; bus.DataSamplePoint = 80;
      c.BitTimeConfig.Stats = stats;
      MCP251XFD_CalculateBitTimeConfiguration(sysclk, bus, &c.BitTimeConfig);
#  endif
#else
      (void)sysclk;
      c.NominalBitrate = nominal; c.DataBitrate = data; c.BitTimeStats = stats;
#endif
      c.Bandwidth = MCP251XFD_NO_DELAY; c.ControlFlags = (setMCP251XFD_CANCtrlFlags)MCP251XFD_CANFD_USE_ISO_CRC;
      c.GPIO0PinMode = MCP251XFD_PIN_AS_INT0_TX; c.GPIO1PinMode = MCP251XFD_PIN_AS_INT1_RX; c.INTsOutMode = MCP251XFD_PINS_PUSHPULL_OUT; c.TXCANOutMode = MCP251XFD_PINS_PUSHPULL_OUT;
      c.SysInterruptFlags = (setMCP251XFD_InterruptEvents)(MCP251XFD_INT_TX_EVENT | MCP251XFD_INT_RX_EVENT | MCP251XFD_INT_SYSTEM_ERROR_EVENT);
    }
    static void SetupForInit(uint32_t spiSpeed)
    {
      Setup();
#if API_V2
      Comp.SPIclockSpeed = spiSpeed;
#else
      Comp.SPIClockSpeed = spiSpeed;
#endif
    }

    TEST_METHOD(TestMethod_Driver_InitDevice)
    {
      MCP251XFD_Config Conf; TStats Stats; uint32_t Sysclk = 0;
      eMCP251XFD_Devices Device = MCP2517FD; uint8_t Id = 0, Rev = 0;

      //--- Crystal 40MHz, SYSCLK = CLKIN, CLKO = SYSCLK / 10, 500 kbit/s and 2 Mbit/s
      SetupForInit(8000000u); memset(&Stats, 0, sizeof Stats); Put8(0xE14, 0x2F);
      FillInitConfig(Conf, 40000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN, 40000000u, 500000u, 2000000u, &Stats, &Sysclk);
      CHECK_OK(Init_MCP251XFD(&Comp, &Conf), L"Init_MCP251XFD 40MHz");
      CHECK_EQ(40000000u, Sysclk, L"SYSCLK");
      CHECK_EQ(1, Dev.SpiInits, L"the SPI interface is initialized once"); CHECK_EQ(8000000u, Dev.LastSpiSpeed, L"SPI speed");
      CHECK_EQ(0x60, Get8(0xE00), L"OSC: no PLL, no divider, CLKO / 10, low power mode bit cleared");
      CHECK_EQ(500000u, ProgrammedNominalBitrate(40000000u), L"nominal bitrate programmed"); CHECK_EQ(2000000u, ProgrammedDataBitrate(40000000u), L"data bitrate programmed");
      CHECK_EQ(500000u, Stats.NominalBitrate, L"nominal bitrate of the statistics");
      CHECK_EQ(MCP251XFD_CANFD_ENABLED, Comp.InternalConfig & MCP251XFD_CANFD_ENABLED, L"CAN FD enabled in the driver");
      CHECK_EQ(MCP251XFD_CONFIGURATION_MODE, Get8(0x003) & 0x07, L"the component stays in Configuration mode");
      CHECK_EQ(0x20, Get8(0x000) & 0x20, L"ISO CRC");
      CHECK_EQ(0x1003, Get16(0x01E), L"interrupt enables");
      CHECK_EQ(0x00, Get8(0xE07), L"pins: INT0 and INT1");
      CHECK_OK(MCP251XFD_GetDeviceID(&Comp, &Device, &Id, &Rev), L"GetDeviceID"); CHECK_EQ(MCP2518FD, Device, L"MCP2518FD detected (LPMEN stays set)"); CHECK_EQ(2, Id, L"device ID"); CHECK_EQ(15, Rev, L"device revision");

      //--- MCP2517FD: the LPMEN bit is not kept
      SetupForInit(8000000u); Dev.Is2518 = false;
      FillInitConfig(Conf, 40000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN, 40000000u, 1000000u, MCP251XFD_NO_CANFD, NULL, NULL);
      CHECK_OK(Init_MCP251XFD(&Comp, &Conf), L"Init_MCP251XFD MCP2517FD");
      CHECK_OK(MCP251XFD_GetDeviceID(&Comp, &Device, NULL, NULL), L"GetDeviceID"); CHECK_EQ(MCP2517FD, Device, L"MCP2517FD detected");
      CHECK_EQ(1000000u, ProgrammedNominalBitrate(40000000u), L"nominal bitrate programmed (CAN 2.0)");
      CHECK_EQ(0, Comp.InternalConfig & MCP251XFD_CANFD_ENABLED, L"CAN FD not enabled");

      //--- Crystal 4MHz with the PLL (x10) => SYSCLK 40MHz: the bit time is computed for the SYSCLK, not for the crystal
      SetupForInit(8000000u); Sysclk = 0;
      FillInitConfig(Conf, 4000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN_MUL_10, 40000000u, 500000u, 2000000u, NULL, &Sysclk);
      CHECK_OK(Init_MCP251XFD(&Comp, &Conf), L"Init_MCP251XFD 4MHz x10");
      CHECK_EQ(40000000u, Sysclk, L"SYSCLK with the PLL");
      CHECK_EQ(0x61, Get8(0xE00), L"OSC: PLL enabled, CLKO / 10");
      CHECK_EQ(500000u, ProgrammedNominalBitrate(40000000u), L"nominal bitrate programmed with the PLL"); CHECK_EQ(2000000u, ProgrammedDataBitrate(40000000u), L"data bitrate programmed with the PLL");

      //--- Crystal 20MHz divided by 2 => SYSCLK 10MHz (the SPI clock is limited to 85% of SYSCLK / 2)
      SetupForInit(4000000u); Sysclk = 0;
      FillInitConfig(Conf, 20000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN_DIV_2, 10000000u, 500000u, 1000000u, NULL, &Sysclk);
      CHECK_OK(Init_MCP251XFD(&Comp, &Conf), L"Init_MCP251XFD 20MHz / 2");
      CHECK_EQ(10000000u, Sysclk, L"SYSCLK with the divider"); CHECK_EQ(0x70, Get8(0xE00), L"OSC: SCLKDIV, CLKO / 10");
      CHECK_EQ(500000u, ProgrammedNominalBitrate(10000000u), L"nominal bitrate programmed at 10MHz"); CHECK_EQ(1000000u, ProgrammedDataBitrate(10000000u), L"data bitrate programmed at 10MHz");

      //--- Crystal 4MHz x5 (PLL x10 then divided by 2) => SYSCLK 20MHz (the SPI clock is limited to 85% of SYSCLK / 2)
      SetupForInit(4000000u); Sysclk = 0;
      FillInitConfig(Conf, 4000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN_MUL_5, 20000000u, 500000u, 2000000u, NULL, &Sysclk);
      CHECK_OK(Init_MCP251XFD(&Comp, &Conf), L"Init_MCP251XFD 4MHz x5");
      CHECK_EQ(20000000u, Sysclk, L"SYSCLK with the PLL and the divider"); CHECK_EQ(0x71, Get8(0xE00), L"OSC: PLL enabled, SCLKDIV, CLKO / 10");
      CHECK_EQ(500000u, ProgrammedNominalBitrate(20000000u), L"nominal bitrate programmed at 20MHz"); CHECK_EQ(2000000u, ProgrammedDataBitrate(20000000u), L"data bitrate programmed at 20MHz");

      //--- Oscillator (no crystal) 20MHz
      SetupForInit(8000000u); Sysclk = 0;
      FillInitConfig(Conf, 0u, 20000000u, MCP251XFD_SYSCLK_IS_CLKIN, 20000000u, 250000u, 1000000u, NULL, &Sysclk);
      CHECK_OK(Init_MCP251XFD(&Comp, &Conf), L"Init_MCP251XFD oscillator 20MHz");
      CHECK_EQ(20000000u, Sysclk, L"SYSCLK of the oscillator");
      CHECK_EQ(250000u, ProgrammedNominalBitrate(20000000u), L"nominal bitrate programmed with an oscillator"); CHECK_EQ(1000000u, ProgrammedDataBitrate(20000000u), L"data bitrate programmed with an oscillator");

      //--- Errors
      SetupForInit(8000000u); FillInitConfig(Conf, 3999999u, 0u, MCP251XFD_SYSCLK_IS_CLKIN, 4000000u, 500000u, 2000000u, NULL, NULL);
      CHECK_ERR(ERR__FREQUENCY_ERROR, Init_MCP251XFD(&Comp, &Conf), L"crystal below 4MHz");
      Conf.XtalFreq = 40000001u; CHECK_ERR(ERR__FREQUENCY_ERROR, Init_MCP251XFD(&Comp, &Conf), L"crystal above 40MHz");
      Conf.XtalFreq = 0u; Conf.OscFreq = 1999999u; CHECK_ERR(ERR__FREQUENCY_ERROR, Init_MCP251XFD(&Comp, &Conf), L"oscillator below 2MHz");
      Conf.XtalFreq = 0u; Conf.OscFreq = 0u; CHECK_ERR(ERR__CONFIGURATION, Init_MCP251XFD(&Comp, &Conf), L"no crystal and no oscillator");
      FillInitConfig(Conf, 40000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN_MUL_10, 40000000u, 500000u, 2000000u, NULL, NULL);
      CHECK_ERR(ERR__FREQUENCY_ERROR, Init_MCP251XFD(&Comp, &Conf), L"CLKIN x 10 above the limit of the PLL");
      FillInitConfig(Conf, 40000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN, 40000000u, 500000u, 2000000u, NULL, NULL);
      SetupForInit(17000001u); CHECK_ERR(ERR__SPI_FREQUENCY_ERROR, Init_MCP251XFD(&Comp, &Conf), L"SPI clock above the limit of the component");
      FillInitConfig(Conf, 20000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN_DIV_2, 10000000u, 500000u, 1000000u, NULL, NULL);
      SetupForInit(5000000u); CHECK_ERR(ERR__SPI_FREQUENCY_ERROR, Init_MCP251XFD(&Comp, &Conf), L"SPI clock above 85% of SYSCLK / 2");
      FillInitConfig(Conf, 40000000u, 0u, MCP251XFD_SYSCLK_IS_CLKIN, 40000000u, 500000u, 2000000u, NULL, NULL);
      SetupForInit(8000000u); Dev.Dead = true; CHECK_ERR(ERR__NO_DEVICE_DETECTED, Init_MCP251XFD(&Comp, &Conf), L"no component on the SPI");
      SetupForInit(8000000u); Dev.OscNeverReady = true; CHECK_ERR(ERR__DEVICE_TIMEOUT, Init_MCP251XFD(&Comp, &Conf), L"oscillator never ready");
#ifdef CHECK_NULL_PARAM
      CHECK_ERR(ERR__PARAMETER_ERROR, Init_MCP251XFD(NULL, &Conf), L"Init without component");
      CHECK_ERR(ERR__PARAMETER_ERROR, Init_MCP251XFD(&Comp, NULL), L"Init without configuration");
#endif
    }
  };
}
