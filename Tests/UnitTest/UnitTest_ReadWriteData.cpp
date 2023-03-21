#include "stdafx.h"
#include "CppUnitTest.h"
#include <cstdint>
#include <windows.h>
#include <WinUser.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework
{
  template<> inline std::wstring ToString<eERRORRESULT>(const eERRORRESULT& t) { RETURN_WIDE_STRING(t); }
//  template<> inline std::wstring ToString<unsigned short>(const unsigned short& t) { RETURN_WIDE_STRING(t); }
} } }


//#define CAAA(C, A)      (uint8_t)((C << 4) | ((A >> 8) & 0xF)), (uint8_t)(A & 0xFF)
#define TEST_BUF_COUNT    ( ((2 + 1 + 2) * 256) + 5 ) // 256 bytes each send with Command+CRC (worst case is Safe Write with 2 for Command + 2 for CRC for each byte send) + 5 dummy bytes

uint32_t TransferCounter = 0;
uint32_t BytesTransfered = 0;
size_t   TestBufPos = 0;
uint8_t TxTestBuf[TEST_BUF_COUNT]; // Data send through SPI when transfering data
uint8_t RxTestBuf[TEST_BUF_COUNT]; // Data receive through SPI when transfering data

#define CAAA_0(C, A)    (uint8_t)(((C) << 4) | ((A) >> 8 & 0xF))
#define CAAA_1(C, A)    (uint8_t)((A) & 0xFF)
#define GET_CRC_AT(T, P)  (uint16_t)(((T[(P)] << 8) & 0xFF00) | ((T[(P) + 1]) & 0xFF))




eERRORRESULT InterfaceTransferReadWrite(void *pIntDev, uint8_t, uint8_t *txData, uint8_t *rxData, uint32_t size)
{
  memcpy(&TxTestBuf[TestBufPos], txData, size);   // Copy what is send to SPI to TxTestBuf
  if (rxData != NULL)
    memcpy(rxData, &RxTestBuf[TestBufPos], size); // Copy what is receive from SPI to RxTestBuf
  TransferCounter++;                              // Increment buffer
  BytesTransfered += size;                        // Add the count of byte transfered to the previous number of bytes transfered
  TestBufPos += size;

  return ERR_OK;
}

eERRORRESULT InterfaceTransferError(void*, uint8_t, uint8_t*, uint8_t*, uint32_t)
{
  return ERR__TEST_ERROR;
}

namespace UnitTest_BufSize81
{
  TEST_CLASS(UnitTest_BufSize81)
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

    void Init_MCP251XFDstruct(MCP251XFD *struc, setMCP251XFD_DriverConfig driverConf)
    {
      if (struc == NULL) return;

      struc->UserDriverData = NULL;
      //--- Driver configuration ---
      struc->DriverConfig = driverConf;
      //--- Interface driver call functions ---
      struc->fnSPI_Init     = NULL;
      struc->fnSPI_Transfer = NULL;
      //--- CRC16-CMS call function ---
      struc->fnComputeCRC16 = NULL;
      //--- IO configuration ---
      struc->GPIOsOutState = 0b00;

      TransferCounter = 0;
      BytesTransfered = 0;
    }

    bool ArraysAreEqual(uint8_t *arrayA, uint8_t *arrayB, size_t count)
    {
      if ((arrayA == NULL) || (arrayB == NULL)) return false;
      return memcmp(arrayA, arrayB, count) == 0;
    }

    bool CheckOnlyCtrls(uint8_t *pArray, size_t dataCount, uint8_t inst, uint16_t addr, bool noCheckCRC = false)
    {
      size_t  RemSize = TEST_BUF_COUNT;
      uint32_t CtrlByteCount = ControlItemByteCount(inst);                    // Get control item byte count
      bool InRAM = (addr >= MCP251XFD_RAM_ADDR) && (addr < (MCP251XFD_RAM_ADDR + MCP251XFD_RAM_SIZE)); // True if address is in the RAM region ; False if address is in Controller SFR or CAN SFR region
      while ((dataCount > 0) && (RemSize > (CtrlByteCount + dataCount)))
      {
        uint8_t* pBeginA = pArray;                                            // Save the begining of the frame in case of CRC
        // Check Command bytes
        if (*pArray != ((inst << 4) | ((addr >> 8) & 0xF))) return false;
        pArray++;
        if (*pArray != (addr & 0xFF)) return false;
        pArray++;
        RemSize -= 2;
        uint8_t Len = (uint8_t)(dataCount > (MCP251XFD_TRANS_BUF_SIZE - CtrlByteCount) ? (MCP251XFD_TRANS_BUF_SIZE - CtrlByteCount) : dataCount);
        if (inst == MCP251XFD_SPI_INSTRUCTION_READ_CRC)                       // Check length if any
        {
          if (InRAM)
          {
            Len &= 0xFFFC;
            if ((Len >> 2) != *pArray) return false;
          }
          else
            if (Len != *pArray) return false;
          pArray++;
          RemSize--;
        }
        dataCount -= Len;
        pArray += Len;
        RemSize -= Len;
        addr += Len;
        if ((inst == MCP251XFD_SPI_INSTRUCTION_READ_CRC))                     // Check CRC if any
        {
          if (!noCheckCRC)
          {
            uint16_t CRC = ComputeCRC16CMS(pBeginA, Len + CtrlByteCount - 2);
            if (((CRC >> 8) & 0xFF) != *pArray) return false;                 // CRC MSB
            if ((CRC & 0xFF) != *pArray) return false;                        // CRC LSB
          }
          pArray += 2;
          RemSize -= 2;
        }
      }
      return (dataCount == 0);
    }





    uint32_t ControlItemByteCount(uint8_t inst)
    {
      switch (inst)
      {
        case MCP251XFD_SPI_INSTRUCTION_READ_CRC:
        case MCP251XFD_SPI_INSTRUCTION_WRITE_CRC:
        {
          return 2 + 1 + 2; // 2 for Command + 1 for Length + 2 for CRC
        }
        break;
        case MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE:
        {
          return 2 + 2; // 2 for Command + 2 for CRC
        }
        break;
        case MCP251XFD_SPI_INSTRUCTION_READ:
        case MCP251XFD_SPI_INSTRUCTION_WRITE:
        default:
        {
          return 2; // 2 for Command
        }
      }
    }

    bool FillRxArray(uint8_t inst, uint16_t addr, uint8_t *pArray, uint8_t *pDataFill, size_t dataCount)
    { // Fill like FF(F)D..D(CC)FF(F)D..D(CC)GGGGGG where count of 'D' is dataCount and 'D' value is pDataFill and 'G' is garbadge byte. Count of 'D' between 'FF(F)' is MCP251XFD_TRANS_BUF_SIZE - 2 for Cmd {F} - 1 for Length {F} (if any) - 2 for CRC {CC} (if any)
      if (pArray == NULL) return false;
      size_t RemSize = TEST_BUF_COUNT;
      uint32_t CtrlByteCount = ControlItemByteCount(inst); // Get control item byte count

      while ((dataCount > 0) && (RemSize > (CtrlByteCount + dataCount)))
      {
        uint8_t Len = (uint8_t)(dataCount > (MCP251XFD_TRANS_BUF_SIZE - CtrlByteCount) ? (MCP251XFD_TRANS_BUF_SIZE - CtrlByteCount) : dataCount);
        uint8_t* pBeginA = pArray;                       // Save the begining of the frame in case of CRC
        if (inst == MCP251XFD_SPI_INSTRUCTION_READ_CRC)
        {
          *pArray++ = (inst << 4) | ((addr >> 8) & 0xF); // Command MSB
          *pArray++ = addr & 0xFF;                       // Command LSB
          if ((addr >= MCP251XFD_RAM_ADDR) && (addr < (MCP251XFD_RAM_ADDR + MCP251XFD_RAM_SIZE)))
               *pArray++ = Len >> 2;                     // Add Length for RAM registers
          else *pArray++ = Len;                          // Add Length for SFR registers
          RemSize--;
        }
        else
        {
          *pArray++ = 0xFF;                              // Command MSB
          *pArray++ = 0xFF;                              // Command LSB
        }
        RemSize -= 2;
        memcpy(pArray, pDataFill, Len);
        dataCount -= Len;
        pDataFill += Len;
        pArray    += Len;
        RemSize   -= Len;
        addr      += Len;
        if (inst == MCP251XFD_SPI_INSTRUCTION_READ_CRC)  // Add CRC if any
        {
          uint16_t CRC = ComputeCRC16CMS(pBeginA, Len + CtrlByteCount - 2);
          *pArray++ = (CRC >> 8) & 0xFF;                 // CRC MSB
          *pArray++ = CRC & 0xFF;                        // CRC LSB
          pBeginA[0] = 0xFF;                             // After CRC, put 0xFF on Command MSB
          pBeginA[1] = 0xFF;                             // After CRC, put 0xFF on Command LSB
          pBeginA[2] = 0xFF;                             // After CRC, put 0xFF on Length
          RemSize -= 2;
        }
      }
      return (dataCount == 0);
    }





    bool FillRxArray_NormalRead(uint16_t addr, uint8_t *pArray, uint8_t dataFillByte, size_t dataCount)
    { // Fill like FFD..D00D..DGGGGGG where count of 'D' is dataCount and 'D' value is dataFillByte and 'G' is garbadge byte. Count of 'D' between 'FF' is MCP251XFD_TRANS_BUF_SIZE-2
      uint8_t *pDataFill{ new uint8_t[dataCount] {} };
      memset(pDataFill, dataFillByte, dataCount);
      bool Result = FillRxArray(MCP251XFD_SPI_INSTRUCTION_READ, addr, pArray, pDataFill, dataCount);
      delete[] pDataFill;
      return Result;
    }

    bool FillRxArray_NormalRead(uint16_t addr, uint8_t *pArray, uint8_t *pDataFill, size_t dataCount)
    { // Fill like FFD..D00D..DGGGGGG where count of 'D' is dataCount and 'D' value is pDataFill and 'G' is garbadge byte. Count of 'D' between 'FF' is MCP251XFD_TRANS_BUF_SIZE-2
      return FillRxArray(MCP251XFD_SPI_INSTRUCTION_READ, addr, pArray, pDataFill, dataCount);
    }

    bool FillRxArray_CRCRead(uint16_t addr, uint8_t *pArray, uint8_t dataFillByte, size_t dataCount)
    { // Fill like FFFD..DCCFFFD..DCCGGGGGG where count of 'D' is dataCount and 'D' value is dataFillByte and 'G' is garbadge byte. Count of 'D' between 'FFF' is MCP251XFD_TRANS_BUF_SIZE-5
      uint8_t *pDataFill{ new uint8_t[dataCount] {} };
      memset(pDataFill, dataFillByte, dataCount);
      bool Result = FillRxArray(MCP251XFD_SPI_INSTRUCTION_READ_CRC, addr, pArray, pDataFill, dataCount);
      delete[] pDataFill;
      return Result;
    }

    bool FillRxArray_CRCRead(uint16_t addr, uint8_t *pArray, uint8_t *pDataFill, size_t dataCount)
    { // Fill like FFFD..DFFFD..DGGGGGG where count of 'D' is dataCount and 'D' value is pDataFill and 'G' is garbadge byte. Count of 'D' between 'FFF' is MCP251XFD_TRANS_BUF_SIZE-5
      return FillRxArray(MCP251XFD_SPI_INSTRUCTION_READ_CRC, addr, pArray, pDataFill, dataCount);
    }





    bool FillTxArray(uint8_t inst, uint16_t addr, uint8_t *pArray, size_t dataCount)
    {
      memset(&TxTestBuf[0], 0xCC, TEST_BUF_COUNT);                                      // Fill TxTestBuf with 0xCC

      size_t  RemSize = TEST_BUF_COUNT;
      uint32_t CtrlByteCount = ControlItemByteCount(inst);                              // Get control item byte count
      while ((dataCount > 0) && (RemSize > (CtrlByteCount + dataCount)))
      {
        uint8_t* pBeginA = pArray;                                                      // Save the begining of the frame in case of CRC
        *pArray++ = (inst << 4) | ((addr >> 8) & 0xF);
        *pArray++ = addr & 0xFF;
        RemSize -= 2;
        uint8_t Len = (uint8_t)(dataCount > (MCP251XFD_TRANS_BUF_SIZE - CtrlByteCount) ? (MCP251XFD_TRANS_BUF_SIZE - CtrlByteCount) : dataCount);
        if (inst == MCP251XFD_SPI_INSTRUCTION_READ_CRC) { *pArray++ = Len; RemSize--; } // Add length if any
        dataCount -= Len;
        pArray    += Len;
        RemSize   -= Len;
        addr      += Len;
        if (inst == MCP251XFD_SPI_INSTRUCTION_READ_CRC)                                 // Add CRC if any
        {
          uint16_t CRC = ComputeCRC16CMS(pBeginA, Len + CtrlByteCount - 2);
          *pArray++ = (CRC >> 8) & 0xFF;                                                // CRC MSB
          *pArray++ = CRC & 0xFF;                                                       // CRC LSB
          RemSize -= 2;
        }
      }
      return (dataCount == 0);
    }

    bool FillTxArray_NormalRead(uint16_t addr, uint8_t *pArray, size_t dataCount, uint8_t dataFillByte = 0xCC)
    {
      // Fill data that will be sent while transfer
      if (FillRxArray_NormalRead(addr, pArray, dataFillByte, dataCount) == false) return false;
      return FillTxArray(MCP251XFD_SPI_INSTRUCTION_READ, addr, pArray, dataCount);
    }

    bool FillTxArray_CRCRead(uint16_t addr, uint8_t *pArray, size_t dataCount, uint8_t dataFillByte = 0xCC)
    {
      // Fill data that will be sent while transfer
      if (FillRxArray_CRCRead(addr, pArray, dataFillByte, dataCount) == false) return false;
      return FillTxArray(MCP251XFD_SPI_INSTRUCTION_READ_CRC, addr, pArray, dataCount);
    }





  public:
    TEST_CLASS_INITIALIZE(ClassInitialize)
    {
      char str[256];
      sprintf_s(str, "MCP251XFD_TRANS_BUF_SIZE set to %d", MCP251XFD_TRANS_BUF_SIZE);
      Logger::WriteMessage(str);
    }

    TEST_METHOD(TestMethod_MCP251XFD_ReadData_NORMALREAD)
    {
      Assert::AreEqual(81u, (uint32_t)MCP251XFD_TRANS_BUF_SIZE, L"Test can't perform with MCP251XFD_TRANS_BUF_SIZE != 81");
      Assert::IsTrue((MCP251XFD_TRANS_BUF_SIZE + 5) < TEST_BUF_COUNT, L"Test can't perform with TEST_BUF_COUNT too low");
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD MCP251XFD_TestNormalRead;
      Init_MCP251XFDstruct(&MCP251XFD_TestNormalRead, MCP251XFD_DRIVER_NORMAL_USE); // Init the structure

      //=============================================================================
      // Parameters check
      //=============================================================================
      //--- Test ((pComp == NULL) || (data == NULL)) ---
      Error = MCP251XFD_ReadData(NULL, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test ((pComp == NULL) || (data == NULL)), should be ERR__PARAMETER_ERROR");
      //--- Test (pComp == NULL) only ---
      Error = MCP251XFD_ReadData(NULL, 0, &RxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (pComp == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (data == NULL) only ---
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (data == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (address > MCP251XFD_END_ADDR) only ---
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_END_ADDR + 1, &RxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (address > MCP251XFD_END_ADDR), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer == NULL) only ---
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_END_ADDR, &RxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fnInterfaceTransfer == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer return error) only ---
      MCP251XFD_TestNormalRead.fnSPI_Transfer = &InterfaceTransferError;
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, 0, &RxTestBuf[0], 1);
      Assert::AreEqual(ERR__TEST_ERROR, Error, L"Test (fnInterfaceTransfer return error), only should be ERR__TEST_ERROR");

      Assert::AreEqual(0u, TransferCounter, L"TransferCounter should be 0 at Parameters check"); // Should go 0 time in the InterfaceTransfer function
      Assert::AreEqual(0u, BytesTransfered, L"BytesTransfered should be 0 at Parameters check"); // Should have transfer 0 byte in the InterfaceTransfer function

      //=============================================================================
      // Read Data
      //=============================================================================
      uint8_t ReceiveBuf[TEST_BUF_COUNT];                                    // Data received when calling MCP251XFD_ReadData function
      uint8_t TransmitBuf[TEST_BUF_COUNT];                                   // Data transmitted when calling MCP251XFD_ReadData function
      uint8_t DataToReceive[256];                                            // Send test table
      MCP251XFD_TestNormalRead.fnSPI_Transfer = &InterfaceTransferReadWrite; // Set the tranfer interface function
      for (size_t z = 0; z <= 0xFF; z++) DataToReceive[z] = (uint8_t)z;      // Generate test table, each entry equals to its index

      //--- Test (size = 0) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 0); // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size = 0}, should be ERR_OK");                                   // Check the return value
      Assert::AreEqual(0u, TransferCounter, L"Test {size = 0}, TransferCounter should be 0");                  // Should go 0 time in the InterfaceTransfer function

      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                             // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillTxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &TransmitBuf[0], 1), L"Test {size=1; SFR}, error generating data for test");     // Set all bytes to receive to 0xAA
      Assert::IsTrue(FillRxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], 0xAA, 1), L"Test {size=1; SFR}, error generating data for test"); // Set the byte to receive to 0xAA
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 1);                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=1; SFR}, should be ERR_OK");                                                                             // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=1; SFR}, TransferCounter should be 1");                                                            // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(3u, BytesTransfered, L"Test {size=1; SFR}, BytesTransfered should be 3");                                                            // Should have transfer 3 bytes in the InterfaceTransfer function
      Assert::IsTrue(ArraysAreEqual(&TxTestBuf[0], &TransmitBuf[0], 3), L"Test {size=1; SFR}, Wrong data sent by SPI");                                     // Check commands that have been send trough SPI (Check only SPI instruction : CAAA)
      Assert::AreEqual((uint8_t)0xAA, ReceiveBuf[0], L"Test {size=1; SFR}, Wrong data received by SPI");                                                    // Check what have been read by MCP251XFD_ReadData

      //--- Test (size = BUF limit) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                        // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillTxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &TransmitBuf[0], (81 - 2)), L"Test {size=BUFlimit; SFR}, error generating data for test");                  // Set all bytes to receive to 0xAA
      Assert::IsTrue(FillRxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], (81 - 2)), L"Test {size=BUFlimit; SFR}, error generating data for test"); // Set all bytes to receive to 0xAA
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], (81 - 2));                                                                  // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit; SFR}, should be ERR_OK");                                                                                                 // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=BUFlimit; SFR}, TransferCounter should be 1");                                                                                // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(81u, BytesTransfered, L"Test {size=BUFlimit; SFR}, BytesTransfered should be BUF");                                                                             // Should have transfer full buffer bytes in the InterfaceTransfer function
      Assert::IsTrue(ArraysAreEqual(&TxTestBuf[0], &TransmitBuf[0], TEST_BUF_COUNT), L"Test {size=BUFlimit; SFR}, Wrong data sent by SPI");                                            // Check commands that have been send trough SPI (Check only SPI instruction : CAAA)
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], (81 - 2)), L"Test {size=BUFlimit; SFR}, Wrong data received by SPI");                                           // Check what have been read by MCP251XFD_ReadData

      //--- Test (size = BUF limit + 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                              // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], (81 - 2 + 1)), L"Test {size=BUFlimit+1; SFR}, error generating data for test"); // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], (81 - 2 + 1));                                                                    // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit+1; SFR}, should be ERR_OK");                                                                                                     // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=BUFlimit+1; SFR}, TransferCounter should be 2");                                                                                    // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual((81u - 2u + 1u) + (2u * 2u), BytesTransfered, L"Test {size=BUFlimit+1; SFR}, BytesTransfered should be BUF+1+FramesCtrls");                                           // Should have transfer full buffer + 1 byte bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], (81 - 2 + 1), MCP251XFD_SPI_INSTRUCTION_READ, MCP251XFD_CONTROLLER_SFR_ADDR), L"Test {size=BUFlimit+1; SFR}, Wrong data sent by SPI");    // Check commands that have been send trough SPI (Check only SPI instruction : CAAA)
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], (81 - 2 + 1)), L"Test {size=BUFlimit+1; SFR}, Wrong data received by SPI");                                           // Check what that have been read by MCP251XFD_ReadData

      //--- Test (size = 256 bytes) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                              // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], 256), L"Test {size=256; SFR}, error generating data for test"); // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 256);                                                             // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR}, should be ERR_OK");                                                                                            // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; SFR}, TransferCounter should be 4");                                                                           // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual((256u + (4u * 2u)), BytesTransfered, L"Test {size=256; SFR}, BytesTransfered should be 256+4*Ctrls");                                                 // Should have transfer (256 bytes + (4 transfers * 2 bytes for control) bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 256, MCP251XFD_SPI_INSTRUCTION_READ, MCP251XFD_CONTROLLER_SFR_ADDR), L"Test {size=256; SFR}, Wrong data sent by SPI");    // Check commands that have been send trough SPI (Check only SPI instruction : CAAA)
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 256), L"Test {size=256; SFR}, Wrong data received by SPI");                                           // Check what that have been read by MCP251XFD_ReadData

      //--- Test (size = 256 bytes; Clear Buffer before read) ---
      MCP251XFD_TestNormalRead.DriverConfig = (setMCP251XFD_DriverConfig)((uint8_t)MCP251XFD_TestNormalRead.DriverConfig | (uint8_t)MCP251XFD_DRIVER_CLEAR_BUFFER_BEFORE_READ);         // Add clear buffer before transfer flag
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                         // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillTxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &TransmitBuf[0], 256, 0x00), L"Test {size=256; SFR; CLEAR_BUF}, error generating data for test");            // Set all bytes to receive to 0xAA
      Assert::IsTrue(FillRxArray_NormalRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], 256), L"Test {size=256; SFR; CLEAR_BUF}, error generating data for test"); // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestNormalRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 256);                                                                        // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR; CLEAR_BUF}, should be ERR_OK");                                                                                            // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; SFR; CLEAR_BUF}, TransferCounter should be 4");                                                                           // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual((256u + (4u * 2u)), BytesTransfered, L"Test {size=256; SFR; CLEAR_BUF}, BytesTransfered should be 256+4*Ctrls");                                                 // Should have transfer (256 bytes + (4 transfers * 2 bytes for control) bytes in the InterfaceTransfer function
      Assert::IsTrue(ArraysAreEqual(&TxTestBuf[0], &TransmitBuf[0], 256), L"Test {size=256; SFR; CLEAR_BUF}, Wrong data sent by SPI");                                                  // Check commands that have been send trough SPI (Check only SPI instruction : CAAA)
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 256), L"Test {size=256; SFR; CLEAR_BUF}, Wrong data received by SPI");                                           // Check what that have been read by MCP251XFD_ReadData
    }



    TEST_METHOD(TestMethod_MCP251XFD_ReadData_CRCREAD)
    {
      Assert::AreEqual(81u, (uint32_t)MCP251XFD_TRANS_BUF_SIZE, L"Test can't perform with MCP251XFD_TRANS_BUF_SIZE != 81");
      Assert::IsTrue((MCP251XFD_TRANS_BUF_SIZE + 5) < TEST_BUF_COUNT, L"Test can't perform with TEST_BUF_COUNT too low");
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD MCP251XFD_TestCRCRead;
      Init_MCP251XFDstruct(&MCP251XFD_TestCRCRead, MCP251XFD_DRIVER_USE_READ_WRITE_CRC); // Init the structure

      //=============================================================================
      // Parameters check
      //=============================================================================
      //--- Test ((pComp == NULL) || (data == NULL)) ---
      Error = MCP251XFD_ReadData(NULL, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test ((pComp == NULL) || (data == NULL)), should be ERR__PARAMETER_ERROR");
      //--- Test (pComp == NULL) only ---
      Error = MCP251XFD_ReadData(NULL, 0, &RxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (pComp == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (data == NULL) only ---
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (data == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (address > MCP251XFD_END_ADDR) only ---
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_END_ADDR + 1, &RxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (address > MCP251XFD_END_ADDR), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer == NULL) only ---
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_END_ADDR, &RxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fnInterfaceTransfer == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer return error) only ---
      MCP251XFD_TestCRCRead.fnSPI_Transfer = &InterfaceTransferError;
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, 0, &RxTestBuf[0], 1);
      Assert::AreEqual(ERR__TEST_ERROR, Error, L"Test (fnInterfaceTransfer return error), only should be ERR__TEST_ERROR");

      Assert::AreEqual(0u, TransferCounter, L"TransferCounter should be 0 at Parameters check"); // Should go 0 time in the InterfaceTransfer function
      Assert::AreEqual(0u, BytesTransfered, L"BytesTransfered should be 0 at Parameters check"); // Should have transfer 0 byte in the InterfaceTransfer function

      //=============================================================================
      // Read Data in SFR address
      //=============================================================================
      uint8_t ReceiveBuf[TEST_BUF_COUNT];                                 // Data received when calling MCP251XFD_ReadData function
      uint8_t DataToReceive[256];                                         // Send test table
      MCP251XFD_TestCRCRead.fnSPI_Transfer = &InterfaceTransferReadWrite; // Set the tranfer interface function
      for (size_t z = 0; z <= 0xFF; z++) DataToReceive[z] = (uint8_t)z;   // Generate test table, each entry equals to its index

      //--- Test (size = 0) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                             // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 0); // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size = 0}, should be ERR_OK");                                // Check the return value
      Assert::AreEqual(0u, TransferCounter, L"Test {size = 0}, TransferCounter should be 0");               // Should go 0 time in the InterfaceTransfer function

      //--- Test (size = 1; No CRC16SUB function) ---
      TransferCounter = 0; BytesTransfered = 0;  TestBufPos = 0;                                                   // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 1);        // *** Test function
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test {size=1; SFR; NoCRC}, should be ERR__PARAMETER_ERROR"); // Check the return value

      MCP251XFD_TestCRCRead.fnComputeCRC16 = &ComputeCRC16CMS;                 // Set the compute CRC16-CMS function

      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                 // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], 0xAA, 1), L"Test {size=1; SFR}, error generating data for test");                        // Set the byte to receive to 0xAA
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 1);                                                                     // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=1; SFR}, should be ERR_OK");                                                                                                 // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=1; SFR}, TransferCounter should be 1");                                                                                // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(6u, BytesTransfered, L"Test {size=1; SFR}, BytesTransfered should be 6");                                                                                // Should have transfer 6 bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 1, MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_CONTROLLER_SFR_ADDR, true), L"Test {size=1; SFR}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::AreEqual((uint8_t)0xAA, ReceiveBuf[0], L"Test {size=1; SFR}, Wrong data received by SPI");                                                                        // Check what have been read by MCP251XFD_ReadData

      //--- Test (size = BUF limit) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                               // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], (81 - 5)), L"Test {size=BUFlimit; SFR}, error generating data for test");           // Set all bytes to receive to 0xAA
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], (81 - 5));                                                                            // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit; SFR}, should be ERR_OK");                                                                                                        // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=BUFlimit; SFR}, TransferCounter should be 1");                                                                                       // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(81u, BytesTransfered, L"Test {size=BUFlimit; SFR}, BytesTransfered should be BUF");                                                                                    // Should have transfer full buffer bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], (81 - 5), MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_CONTROLLER_SFR_ADDR, true), L"Test {size=BUFlimit; SFR}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], (81 - 5)), L"Test {size=BUFlimit; SFR}, Wrong data received by SPI");                                                  // Check what have been read by MCP251XFD_ReadData

      //--- Test (size = BUF limit + 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                                     // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], (81 - 5 + 1)), L"Test {size=BUFlimit+1; SFR}, error generating data for test");           // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], (81 - 5 + 1));                                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit+1; SFR}, should be ERR_OK");                                                                                                            // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=BUFlimit+1; SFR}, TransferCounter should be 2");                                                                                           // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual((81 - 5 + 1) + (2u * 5u), BytesTransfered, L"Test {size=BUFlimit+1; SFR}, BytesTransfered should be BUF+1+FramesCtrls");                                                     // Should have transfer 87 bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], (81 - 5 + 1), MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_CONTROLLER_SFR_ADDR, true), L"Test {size=BUFlimit+1; SFR}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], (81 - 5 + 1)), L"Test {size=BUFlimit+1; SFR}, Wrong data received by SPI");                                                  // Check what that have been read by MCP251XFD_ReadData

      //--- Test (size = 256 bytes) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                     // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], 256), L"Test {size=256; SFR}, error generating data for test");           // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 256);                                                                       // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR}, should be ERR_OK");                                                                                                   // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; SFR}, TransferCounter should be 4");                                                                                  // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual(256u + (4u * 5u), BytesTransfered, L"Test {size=256; SFR}, BytesTransfered should be 256+4*Ctrls");                                                          // Should have transfer (256 bytes + (4 transfers * 5 bytes for control) bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 256, MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_CONTROLLER_SFR_ADDR, true), L"Test {size=256; SFR}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 256), L"Test {size=256; SFR}, Wrong data received by SPI");                                                  // Check what that have been read by MCP251XFD_ReadData

      //--- Test (size = 256 bytes; Clear Buffer before read) ---
      MCP251XFD_TestCRCRead.DriverConfig = (setMCP251XFD_DriverConfig)((uint8_t)MCP251XFD_TestCRCRead.DriverConfig | (uint8_t)MCP251XFD_DRIVER_CLEAR_BUFFER_BEFORE_READ);                      // Add clear buffer before transfer flag
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                                // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_CONTROLLER_SFR_ADDR, &RxTestBuf[0], &DataToReceive[0], 256), L"Test {size=256; SFR; CLEAR_BUF}, error generating data for test");           // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_CONTROLLER_SFR_ADDR, &ReceiveBuf[0], 256);                                                                                  // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR; CLEAR_BUF}, should be ERR_OK");                                                                                                   // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; SFR; CLEAR_BUF}, TransferCounter should be 4");                                                                                  // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual(256u + (4u * 5u), BytesTransfered, L"Test {size=256; SFR; CLEAR_BUF}, BytesTransfered should be 256+4*Ctrls");                                                          // Should have transfer (256 bytes + (4 transfers * 5 bytes for control) bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 256, MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_CONTROLLER_SFR_ADDR, true), L"Test {size=256; SFR; CLEAR_BUF}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 256), L"Test {size=256; SFR; CLEAR_BUF}, Wrong data received by SPI");                                                  // Check what that have been read by MCP251XFD_ReadData


      //=============================================================================
      // Read Data in RAM address
      //=============================================================================
      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                     // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], 1);    // *** Test function
      Assert::AreEqual(ERR__DATA_MODULO, Error, L"Test {size=1; RAM}, should be ERR__DATA_MODULO"); // Check the return value

      //--- Test (size = 4) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                      // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_RAM_ADDR, &RxTestBuf[0], &DataToReceive[0], 4), L"Test {size=4; RAM}, error generating data for test");           // Set the byte to receive to 0xAA
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], 4);                                                                     // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=4; RAM}, should be ERR_OK");                                                                                      // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=4; RAM}, TransferCounter should be 1");                                                                     // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(9u, BytesTransfered, L"Test {size=4; RAM}, BytesTransfered should be 9");                                                                     // Should have transfer 9 bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 4, MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_RAM_ADDR, true), L"Test {size=4; RAM}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 4), L"Test {size=4; RAM}, Wrong data received by SPI");                                       // Check what have been read by MCP251XFD_ReadData

      //--- Test (size = BUF limit) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                    // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_RAM_ADDR, &RxTestBuf[0], &DataToReceive[0], (81 - 5)), L"Test {size=BUFlimit; RAM}, error generating data for test");           // Set all bytes to receive to 0xAA
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], (81 - 5));                                                                            // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit; RAM}, should be ERR_OK");                                                                                             // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=BUFlimit; RAM}, TransferCounter should be 1");                                                                            // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(81u, BytesTransfered, L"Test {size=BUFlimit; RAM}, BytesTransfered should be BUF");                                                                         // Should have transfer 81 bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], (81 - 5), MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_RAM_ADDR, true), L"Test {size=BUFlimit; RAM}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], (81 - 5)), L"Test {size=BUFlimit; RAM}, Wrong data received by SPI");                                       // Check what have been read by MCP251XFD_ReadData

      //--- Test (size = BUF limit + 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                              // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], (81 - 5 + 1));  // *** Test function
      Assert::AreEqual(ERR__DATA_MODULO, Error, L"Test {size=BUFlimit+1; RAM}, should be ERR__DATA_MODULO"); // Check the return value

      //--- Test (size = BUF limit + 4) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                          // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_RAM_ADDR, &RxTestBuf[0], &DataToReceive[0], (81 - 5 + 4)), L"Test {size=BUFlimit+4; RAM}, error generating data for test");           // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], (81 - 5 + 4));                                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit+4; RAM}, should be ERR_OK");                                                                                                 // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=BUFlimit+4; RAM}, TransferCounter should be 2");                                                                                // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual((81 - 5 + 4) + (2u * 5u), BytesTransfered, L"Test {size=BUFlimit+4; RAM}, BytesTransfered should be BUF+4+FramesCtrls");                                          // Should have transfer full buffer bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], (81 - 5 + 4), MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_RAM_ADDR, true), L"Test {size=BUFlimit+4; RAM}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], (81 - 5 + 4)), L"Test {size=BUFlimit+4; RAM}, Wrong data received by SPI");                                       // Check what that have been read by MCP251XFD_ReadData

      //--- Test (size = 256 bytes) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                          // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_RAM_ADDR, &RxTestBuf[0], &DataToReceive[0], 256), L"Test {size=256; RAM}, error generating data for test");           // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], 256);                                                                       // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; RAM}, should be ERR_OK");                                                                                        // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; RAM}, TransferCounter should be 4");                                                                       // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual(256u + (4u * 5u), BytesTransfered, L"Test {size=256; RAM}, BytesTransfered should be 256+4*Ctrls");                                               // Should have transfer (256 bytes + (4 transfers * 5 bytes for control) bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 256, MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_RAM_ADDR, true), L"Test {size=256; RAM}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 256), L"Test {size=256; RAM}, Wrong data received by SPI");                                       // Check what that have been read by MCP251XFD_ReadData

      //--- Test (size = 256 bytes; Clear Buffer before read) ---
      MCP251XFD_TestCRCRead.DriverConfig = (setMCP251XFD_DriverConfig)((uint8_t)MCP251XFD_TestCRCRead.DriverConfig | (uint8_t)MCP251XFD_DRIVER_CLEAR_BUFFER_BEFORE_READ);           // Add clear buffer before transfer flag
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                     // Reset InterfaceTransfer and BytesTransfered count to 0
      Assert::IsTrue(FillRxArray_CRCRead(MCP251XFD_RAM_ADDR, &RxTestBuf[0], &DataToReceive[0], 256), L"Test {size=256; RAM; CLEAR_BUF}, error generating data for test");           // Set all bytes to receive to 0xAA (force 2 bunch of data)
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCRead, MCP251XFD_RAM_ADDR, &ReceiveBuf[0], 256);                                                                                  // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; RAM; CLEAR_BUF}, should be ERR_OK");                                                                                        // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; RAM; CLEAR_BUF}, TransferCounter should be 4");                                                                       // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual(256u + (4u * 5u), BytesTransfered, L"Test {size=256; RAM; CLEAR_BUF}, BytesTransfered should be 256+4*Ctrls");                                               // Should have transfer (256 bytes + (4 transfers * 5 bytes for control) bytes in the InterfaceTransfer function
      Assert::IsTrue(CheckOnlyCtrls(&TxTestBuf[0], 256, MCP251XFD_SPI_INSTRUCTION_READ_CRC, MCP251XFD_RAM_ADDR, true), L"Test {size=256; RAM; CLEAR_BUF}, Wrong data sent by SPI"); // Check commands that have been send trough SPI (Check only SPI instruction : CAAA and Len parameter) don't check CRC because the controller don't check neither
      Assert::IsTrue(ArraysAreEqual(&DataToReceive[0], &ReceiveBuf[0], 256), L"Test {size=256; RAM; CLEAR_BUF}, Wrong data received by SPI");                                       // Check what that have been read by MCP251XFD_ReadData
    }





    TEST_METHOD(TestMethod_MCP251XFD_WriteData_NORMALWRITE)
    {
      Assert::AreEqual(81u, (uint32_t)MCP251XFD_TRANS_BUF_SIZE, L"Test can't perform with MCP251XFD_TRANS_BUF_SIZE != 81");
      Assert::IsTrue((MCP251XFD_TRANS_BUF_SIZE + 5) < TEST_BUF_COUNT, L"Test can't perform with TEST_BUF_COUNT too low");
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD MCP251XFD_TestNormalWrite;
      Init_MCP251XFDstruct(&MCP251XFD_TestNormalWrite, MCP251XFD_DRIVER_NORMAL_USE); // Init the structure

      //=============================================================================
      // Parameters check
      //=============================================================================
      //--- Test ((pComp == NULL) || (data == NULL)) ---
      Error = MCP251XFD_WriteData(NULL, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test ((pComp == NULL) || (data == NULL)), should be ERR__PARAMETER_ERROR");
      //--- Test (pComp == NULL) only ---
      Error = MCP251XFD_WriteData(NULL, 0, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (pComp == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (data == NULL) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (data == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (address > MCP251XFD_END_ADDR) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_END_ADDR + 1, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (address > MCP251XFD_END_ADDR), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer == NULL) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_END_ADDR, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fnInterfaceTransfer == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer return error) only ---
      MCP251XFD_TestNormalWrite.fnSPI_Transfer = &InterfaceTransferError;
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, 0, &TxTestBuf[0], 1);
      Assert::AreEqual(ERR__TEST_ERROR, Error, L"Test (fnInterfaceTransfer return error), only should be ERR__TEST_ERROR");

      Assert::AreEqual(0u, TransferCounter, L"TransferCounter should be 0 at Parameters check"); // Should go 0 time in the InterfaceTransfer function
      Assert::AreEqual(0u, BytesTransfered, L"BytesTransfered should be 0 at Parameters check"); // Should have transfer 0 byte in the InterfaceTransfer function

      //=============================================================================
      // Read Data
      //=============================================================================
      uint8_t DataToSend[256];                                                // Send test table
      MCP251XFD_TestNormalWrite.fnSPI_Transfer = &InterfaceTransferReadWrite; // Set the tranfer interface function
      for (size_t z = 0; z <= 0xFF; z++) DataToSend[z] = (uint8_t)z;          // Generate test table, each entry equals to its index

      //--- Test (size = 0) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                        // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 0); // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size = 0}, should be ERR_OK");                           // Check the return value
      Assert::AreEqual(0u, TransferCounter, L"Test {size = 0}, TransferCounter should be 0");          // Should go 0 time in the InterfaceTransfer function

      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                        // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 1);                                                       // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=1; SFR}, should be ERR_OK");                                                                                        // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=1; SFR}, TransferCounter should be 1");                                                                       // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(3u, BytesTransfered, L"Test {size=1; SFR}, BytesTransfered should be 3");                                                                       // Should have transfer 3 bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=1; SFR}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=1; SFR}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::AreEqual(DataToSend[0], TxTestBuf[2], L"Test {size=1; SFR}, Wrong 3rd byte transfered by SPI");                                                          // The byte in 3rd position should be the same as the first to send

      //--- Test (size = BUF limit) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                        // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], (81 - 2));                                                // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit; SFR}, should be ERR_OK");                                                                                 // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=BUFlimit; SFR}, TransferCounter should be 1");                                                                // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(81u, BytesTransfered, L"Test {size=BUFlimit; SFR}, BytesTransfered should be BUF");                                                             // Should have transfer full buffer bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=1; SFR}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=1; SFR}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[2], (81 - 2)), L"Test {size=1; SFR}, Wrong bytes transfered by SPI");                                   // Bytes transfered should be the same as the bytes in DataToSend

      //--- Test (size = BUF limit + 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                             // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], (81 - 2 + 1));                                                                 // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit+1; SFR}, should be ERR_OK");                                                                                                    // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=BUFlimit+1; SFR}, TransferCounter should be 2");                                                                                   // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual((81u - 2u + 1u) + (2u * 2u), BytesTransfered, L"Test {size=BUFlimit+1; SFR}, BytesTransfered should be BUF+1+FramesCtrls");                                          // Should have transfer full buffer + 1 byte bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=1; SFR}, Wrong 1st byte in set 1 transfered by SPI");             // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=1; SFR}, Wrong 2nd byte in set 1 transfered by SPI");             // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[2], (81 - 2)), L"Test {size=1; SFR}, Wrong bytes in set 1 transfered by SPI");                                               // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2)), TxTestBuf[81], L"Test {size=1; SFR}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2)), TxTestBuf[82], L"Test {size=1; SFR}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[79], &TxTestBuf[83], 1), L"Test {size=1; SFR}, Wrong bytes in set 2 transfered by SPI");                                                    // Bytes transfered in the set 2 should be the same as the bytes in DataToSend

      //--- Test (size = 256 bytes) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                                         // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestNormalWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 256);                                                                                      // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR}, should be ERR_OK");                                                                                                                       // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; SFR}, TransferCounter should be 4");                                                                                                      // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual((256u + (4u * 2u)), BytesTransfered, L"Test {size=256; SFR}, BytesTransfered should be 256+4*Ctrls");                                                                            // Should have transfer (256 bytes + (4 transfers * 2 bytes for control) bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=1; SFR}, Wrong 1st byte in set 1 transfered by SPI");                         // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=1; SFR}, Wrong 2nd byte in set 1 transfered by SPI");                         // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[2], (81 - 2)), L"Test {size=1; SFR}, Wrong bytes in set 1 transfered by SPI");                                                           // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2) * 1), TxTestBuf[81 * 1 + 0], L"Test {size=1; SFR}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2) * 1), TxTestBuf[81 * 1 + 1], L"Test {size=1; SFR}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[79 * 1], &TxTestBuf[81 * 1 + 2], (81 - 2)), L"Test {size=1; SFR}, Wrong bytes in set 2 transfered by SPI");                                             // Bytes transfered in the set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2) * 2), TxTestBuf[81 * 2 + 0], L"Test {size=1; SFR}, Wrong 1st byte in set 3 transfered by SPI"); // The byte in 1st position of the set 3 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2) * 2), TxTestBuf[81 * 2 + 1], L"Test {size=1; SFR}, Wrong 2nd byte in set 3 transfered by SPI"); // The byte in 2nd position of the set 3 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[79 * 2], &TxTestBuf[81 * 2 + 2], (81 - 2)), L"Test {size=1; SFR}, Wrong bytes in set 3 transfered by SPI");                                             // Bytes transfered in the set 3 should be the same as the bytes in DataToSend
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2) * 3), TxTestBuf[81 * 3 + 0], L"Test {size=1; SFR}, Wrong 1st byte in set 4 transfered by SPI"); // The byte in 1st position of the set 4 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 2) * 3), TxTestBuf[81 * 3 + 1], L"Test {size=1; SFR}, Wrong 2nd byte in set 4 transfered by SPI"); // The byte in 2nd position of the set 4 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[79 * 3], &TxTestBuf[81 * 3 + 2], 19), L"Test {size=1; SFR}, Wrong bytes in set 4 transfered by SPI");                                                   // Bytes transfered in the set 4 should be the same as the bytes in DataToSend
    }

    TEST_METHOD(TestMethod_MCP251XFD_WriteData_CRCWRITE)
    {
      Assert::AreEqual(81u, (uint32_t)MCP251XFD_TRANS_BUF_SIZE, L"Test can't perform with MCP251XFD_TRANS_BUF_SIZE != 81");
      Assert::IsTrue((MCP251XFD_TRANS_BUF_SIZE + 5) < TEST_BUF_COUNT, L"Test can't perform with TEST_BUF_COUNT too low");
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD MCP251XFD_TestCRCWrite;
      Init_MCP251XFDstruct(&MCP251XFD_TestCRCWrite, MCP251XFD_DRIVER_USE_READ_WRITE_CRC); // Init the structure

      //=============================================================================
      // Parameters check
      //=============================================================================
      //--- Test ((pComp == NULL) || (data == NULL)) ---
      Error = MCP251XFD_WriteData(NULL, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test ((pComp == NULL) || (data == NULL)), should be ERR__PARAMETER_ERROR");
      //--- Test (pComp == NULL) only ---
      Error = MCP251XFD_WriteData(NULL, 0, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (pComp == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (data == NULL) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (data == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (address > MCP251XFD_END_ADDR) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_END_ADDR + 1, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (address > MCP251XFD_END_ADDR), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer == NULL) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_END_ADDR, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fnInterfaceTransfer == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer return error) only ---
      MCP251XFD_TestCRCWrite.fnComputeCRC16 = &ComputeCRC16CMS;                 // Set the compute CRC16-CMS function
      MCP251XFD_TestCRCWrite.fnSPI_Transfer = &InterfaceTransferError;
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, 0, &TxTestBuf[0], 1);
      Assert::AreEqual(ERR__TEST_ERROR, Error, L"Test (fnInterfaceTransfer return error), only should be ERR__TEST_ERROR");

      Assert::AreEqual(0u, TransferCounter, L"TransferCounter should be 0 at Parameters check"); // Should go 0 time in the InterfaceTransfer function
      Assert::AreEqual(0u, BytesTransfered, L"BytesTransfered should be 0 at Parameters check"); // Should have transfer 0 byte in the InterfaceTransfer function

      //=============================================================================
      // Read Data in SFR address
      //=============================================================================
      uint16_t CRCtoCheck = 0;                                             // This is where the CRC of the set will be stored
      uint8_t DataToSend[256];                                             // Send test table
      MCP251XFD_TestCRCWrite.fnComputeCRC16 = NULL;                        // Unset the compute CRC16-CMS function
      MCP251XFD_TestCRCWrite.fnSPI_Transfer = &InterfaceTransferReadWrite; // Set the tranfer interface function
      for (size_t z = 0; z <= 0xFF; z++) DataToSend[z] = (uint8_t)z;       // Generate test table, each entry equals to its index

      //--- Test (size = 0) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                               // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 0); // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size = 0}, should be ERR_OK");                                  // Check the return value
      Assert::AreEqual(0u, TransferCounter, L"Test {size = 0}, TransferCounter should be 0");                 // Should go 0 time in the InterfaceTransfer function

      //--- Test (size = 1; No CRC16SUB function) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                    // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestCRCWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 1);       // *** Test function
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test {size=1; SFR; NoCRC}, should be ERR__PARAMETER_ERROR"); // Check the return value

      MCP251XFD_TestCRCWrite.fnComputeCRC16 = &ComputeCRC16CMS;            // Set the compute CRC16-CMS function

      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                            // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 1);                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=1; SFR}, should be ERR_OK");                                                                                            // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=1; SFR}, TransferCounter should be 1");                                                                           // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(6u, BytesTransfered, L"Test {size=1; SFR}, BytesTransfered should be 3");                                                                           // Should have transfer 3 bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=1; SFR}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=1; SFR}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)1u, TxTestBuf[2], L"Test {size=1; SFR}, Wrong 3rd byte transfered by SPI");                                                                // The byte in 3rd position should be the same as the length of data byte to send
      Assert::AreEqual(DataToSend[0], TxTestBuf[3], L"Test {size=1; SFR}, Wrong 4th byte transfered by SPI");                                                              // The byte in 4th position should be the same as the first to send
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (2 + 1 + 1)), GET_CRC_AT(TxTestBuf, 4), L"Test {size=1; SFR}, Wrong CRC of transfered data");                        // The CRC of data transfered do not match the attempt one

      //--- Test (size = BUF limit) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                   // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], (81 - 5));                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit; SFR}, should be ERR_OK");                                                                                            // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=BUFlimit; SFR}, TransferCounter should be 1");                                                                           // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(81u, BytesTransfered, L"Test {size=BUFlimit; SFR}, BytesTransfered should be BUF");                                                                        // Should have transfer full buffer bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=BUFlimit; SFR}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=BUFlimit; SFR}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(81 - 5), TxTestBuf[2], L"Test {size=BUFlimit; SFR}, Wrong 3rd byte transfered by SPI");                                                          // The byte in 3rd position should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], (81 - 5)), L"Test {size=BUFlimit; SFR}, Wrong bytes transfered by SPI");                                       // Bytes transfered should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (81 - 5 + 2 + 1)), GET_CRC_AT(TxTestBuf, 79), L"Test {size=BUFlimit; SFR}, Wrong CRC of transfered data");                  // The CRC of data transfered do not match the attempt one

      //--- Test (size = BUF limit + 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                                          // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], (81 - 5 + 1));                                                                                 // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit+1; SFR}, should be ERR_OK");                                                                                                                 // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=BUFlimit+1; SFR}, TransferCounter should be 2");                                                                                                // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual((81u - 5u + 1u) + (5u * 2u), BytesTransfered, L"Test {size=BUFlimit+1; SFR}, BytesTransfered should be BUF+1+FramesCtrls");                                                       // Should have transfer full buffer + 1 byte bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=BUFlimit+1; SFR}, Wrong 1st byte in set 1 transfered by SPI");             // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=BUFlimit+1; SFR}, Wrong 2nd byte in set 1 transfered by SPI");             // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(81 - 5), TxTestBuf[2], L"Test {size=BUFlimit+1; SFR}, Wrong 3rd byte in set 1 transfered by SPI");                                                                      // The byte in 3rd position of the set 1 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], (81 - 5)), L"Test {size=BUFlimit+1; SFR}, Wrong bytes in set 1 transfered by SPI");                                                   // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (81 - 5 + 2 + 1)), GET_CRC_AT(TxTestBuf, 79), L"Test {size=BUFlimit+1; SFR}, Wrong CRC in set 1 of transfered data");                              // The CRC of data transfered in set 1 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5)), TxTestBuf[81], L"Test {size=BUFlimit+1; SFR}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5)), TxTestBuf[82], L"Test {size=BUFlimit+1; SFR}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)1u, TxTestBuf[83], L"Test {size=BUFlimit+1; SFR}, Wrong 3rd byte in set 2 transfered by SPI");                                                                           // The byte in 3rd position of the set 2 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[(81 - 5)], &TxTestBuf[84], 1), L"Test {size=BUFlimit+1; SFR}, Wrong bytes in set 2 transfered by SPI");                                                  // Bytes transfered in the set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81], (2 + 1 + 1)), GET_CRC_AT(TxTestBuf, 85), L"Test {size=BUFlimit+1; SFR}, Wrong CRC in set 2 of transfered data");                                  // The CRC of data transfered in set 2 do not match the attempt one

      //--- Test (size = 256 bytes) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                                               // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 256);                                                                                               // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR}, should be ERR_OK");                                                                                                                             // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; SFR}, TransferCounter should be 4");                                                                                                            // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual((256u + (4u * 5u)), BytesTransfered, L"Test {size=256; SFR}, BytesTransfered should be 256+4*Ctrls");                                                                                  // Should have transfer (256 bytes + (4 transfers * 2 bytes for control) bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=256; SFR}, Wrong 1st byte in set 1 transfered by SPI");                         // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=256; SFR}, Wrong 2nd byte in set 1 transfered by SPI");                         // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(81 - 5), TxTestBuf[2], L"Test {size=256; SFR}, Wrong 3rd byte in set 1 transfered by SPI");                                                                                  // The byte in 3rd position of the set 1 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], (81 - 5)), L"Test {size=256; SFR}, Wrong bytes set 1 transfered by SPI");                                                                  // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (81 - 5 + 3)), GET_CRC_AT(TxTestBuf, 79), L"Test {size=256; SFR}, Wrong CRC in set 1 of transfered data");                                              // The CRC of data transfered in set 1 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5) * 1), TxTestBuf[81 * 1 + 0], L"Test {size=256; SFR}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5) * 1), TxTestBuf[81 * 1 + 1], L"Test {size=256; SFR}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(81 - 5), TxTestBuf[81 * 1 + 2], L"Test {size=256; SFR}, Wrong 3rd byte in set 2 transfered by SPI");                                                                         // The byte in 3rd position of the set 2 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[76 * 1], &TxTestBuf[81 * 1 + 3], (81 - 5)), L"Test {size=256; SFR}, Wrong bytes in set 2 transfered by SPI");                                                 // Bytes transfered in the set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81 * 1], (81 - 5 + 3)), GET_CRC_AT(TxTestBuf, 81 * 2 - 2), L"Test {size=256; SFR}, Wrong CRC in set 2 of transfered data");                                 // The CRC of data transfered in set 2 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5) * 2), TxTestBuf[81 * 2 + 0], L"Test {size=256; SFR}, Wrong 1st byte in set 3 transfered by SPI"); // The byte in 1st position of the set 3 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5) * 2), TxTestBuf[81 * 2 + 1], L"Test {size=256; SFR}, Wrong 2nd byte in set 3 transfered by SPI"); // The byte in 2nd position of the set 3 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(81 - 5), TxTestBuf[81 * 2 + 2], L"Test {size=256; SFR}, Wrong 3rd byte in set 3 transfered by SPI");                                                                         // The byte in 3rd position of the set 3 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[76 * 2], &TxTestBuf[81 * 2 + 3], (81 - 5)), L"Test {size=256; SFR}, Wrong bytes in set 3 transfered by SPI");                                                 // Bytes transfered in the set 3 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81 * 2], (81 - 5 + 3)), GET_CRC_AT(TxTestBuf, 81 * 3 - 2), L"Test {size=256; SFR}, Wrong CRC in set 3 of transfered data");                                 // The CRC of data transfered in set 3 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5) * 3), TxTestBuf[81 * 3 + 0], L"Test {size=256; SFR}, Wrong 1st byte in set 4 transfered by SPI"); // The byte in 1st position of the set 4 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_CONTROLLER_SFR_ADDR + (81 - 5) * 3), TxTestBuf[81 * 3 + 1], L"Test {size=256; SFR}, Wrong 2nd byte in set 4 transfered by SPI"); // The byte in 2nd position of the set 4 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(28), TxTestBuf[81 * 3 + 2], L"Test {size=256; SFR}, Wrong 3rd byte in set 4 transfered by SPI");                                                                             // The byte in 3rd position of the set 4 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[76 * 3], &TxTestBuf[81 * 3 + 3], 28), L"Test {size=256; SFR}, Wrong bytes in set 4 transfered by SPI");                                                       // Bytes transfered in the set 4 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81 * 3], (28 + 3)), GET_CRC_AT(TxTestBuf, 81 * 3 + 3 + 28), L"Test {size=256; SFR}, Wrong CRC in set 4 of transfered data");                                // The CRC of data transfered in set 4 do not match the attempt one



      //=============================================================================
      // Read Data in RAM address
      //=============================================================================
      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                    // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 1); // *** Test function
      Assert::AreEqual(ERR__DATA_MODULO, Error, L"Test {size=1; RAM}, should be ERR__DATA_MODULO");  // Check the return value

      //--- Test (size = 4) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                 // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 4);                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=4; RAM}, should be ERR_OK");                                                                                 // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=4; RAM}, TransferCounter should be 1");                                                                // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(9u, BytesTransfered, L"Test {size=4; RAM}, BytesTransfered should be 9");                                                                // Should have transfer 9 bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[0], L"Test {size=4; RAM}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[1], L"Test {size=4; RAM}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)1u, TxTestBuf[2], L"Test {size=4; RAM}, Wrong 3rd byte transfered by SPI");                                                     // The byte in 3rd position should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], 4), L"Test {size=4; RAM}, Wrong bytes transfered by SPI");                                   // Bytes transfered should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (2 + 1 + 4)), GET_CRC_AT(TxTestBuf, 7), L"Test {size=4; RAM}, Wrong CRC of transfered data");             // The CRC of data transfered do not match the attempt one

      //--- Test (size = BUF limit) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                        // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], (81 - 5));                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit; RAM}, should be ERR_OK");                                                                                 // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=BUFlimit; RAM}, TransferCounter should be 1");                                                                // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(81u, BytesTransfered, L"Test {size=BUFlimit; RAM}, BytesTransfered should be BUF");                                                             // Should have transfer 81 bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[0], L"Test {size=BUFlimit; RAM}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[1], L"Test {size=BUFlimit; RAM}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)((81 - 5) >> 2), TxTestBuf[2], L"Test {size=BUFlimit; RAM}, Wrong 3rd byte transfered by SPI");                                        // The byte in 3rd position should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], (81 - 5)), L"Test {size=BUFlimit; RAM}, Wrong bytes transfered by SPI");                            // Bytes transfered should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (81 - 5 + 2 + 1)), GET_CRC_AT(TxTestBuf, 79), L"Test {size=BUFlimit; RAM}, Wrong CRC of transfered data");       // The CRC of data transfered do not match the attempt one

      //--- Test (size = BUF limit + 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                               // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], (81 - 5 + 1)); // *** Test function
      Assert::AreEqual(ERR__DATA_MODULO, Error, L"Test {size=BUFlimit+1; RAM}, should be ERR__DATA_MODULO");  // Check the return value

      //--- Test (size = BUF limit + 4) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                               // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], (81 - 5 + 4));                                                                                 // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=BUFlimit+4; RAM}, should be ERR_OK");                                                                                                      // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=BUFlimit+4; RAM}, TransferCounter should be 2");                                                                                     // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual((81u - 5 + 4) + (2u * 5u), BytesTransfered, L"Test {size=BUFlimit+4; RAM}, BytesTransfered should be BUF+4+FramesCtrls");                                              // Should have transfer full buffer bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[0], L"Test {size=BUFlimit+4; RAM}, Wrong 1st byte in set 1 transfered by SPI");             // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[1], L"Test {size=BUFlimit+4; RAM}, Wrong 2nd byte in set 1 transfered by SPI");             // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)((81 - 5) >> 2), TxTestBuf[2], L"Test {size=BUFlimit+4; RAM}, Wrong 3rd byte in set 1 transfered by SPI");                                                    // The byte in 3rd position of the set 1 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], (81 - 5)), L"Test {size=BUFlimit+4; RAM}, Wrong bytes in set 1 transfered by SPI");                                        // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (81 - 5 + 2 + 1)), GET_CRC_AT(TxTestBuf, 79), L"Test {size=BUFlimit+4; RAM}, Wrong CRC in set 1 of transfered data");                   // The CRC of data transfered in set 1 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5)), TxTestBuf[81], L"Test {size=BUFlimit+4; RAM}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5)), TxTestBuf[82], L"Test {size=BUFlimit+4; RAM}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)1u, TxTestBuf[83], L"Test {size=BUFlimit+4; RAM}, Wrong 3rd byte in set 2 transfered by SPI");                                                                // The byte in 3rd position of the set 2 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[(81 - 5)], &TxTestBuf[84], 1), L"Test {size=BUFlimit+4; RAM}, Wrong bytes in set 2 transfered by SPI");                                       // Bytes transfered in the set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81], (2 + 1 + 4)), GET_CRC_AT(TxTestBuf, 88), L"Test {size=BUFlimit+4; RAM}, Wrong CRC in set 2 of transfered data");                       // The CRC of data transfered in set 2 do not match the attempt one

      //--- Test (size = 256 bytes) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                                    // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestCRCWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 256);                                                                                               // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; RAM}, should be ERR_OK");                                                                                                                  // Check the return value
      Assert::AreEqual(4u, TransferCounter, L"Test {size=256; RAM}, TransferCounter should be 4");                                                                                                 // Should go 4 times in the InterfaceTransfer function
      Assert::AreEqual(256u + (4u * 5u), BytesTransfered, L"Test {size=256; RAM}, BytesTransfered should be 256+4*Ctrls");                                                                         // Should have transfer (256 bytes + (4 transfers * 5 bytes for control) bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[0], L"Test {size=256; RAM}, Wrong 1st byte in set 1 transfered by SPI");                         // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR), TxTestBuf[1], L"Test {size=256; RAM}, Wrong 2nd byte in set 1 transfered by SPI");                         // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)((81 - 5) >> 2), TxTestBuf[2], L"Test {size=256; RAM}, Wrong 3rd byte in set 1 transfered by SPI");                                                                // The byte in 3rd position of the set 1 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[3], (81 - 5)), L"Test {size=256; RAM}, Wrong bytes set 1 transfered by SPI");                                                       // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (81 - 5 + 3)), GET_CRC_AT(TxTestBuf, 79), L"Test {size=256; RAM}, Wrong CRC in set 1 of transfered data");                                   // The CRC of data transfered in set 1 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5) * 1), TxTestBuf[81 * 1 + 0], L"Test {size=256; RAM}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5) * 1), TxTestBuf[81 * 1 + 1], L"Test {size=256; RAM}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)((81 - 5) >> 2), TxTestBuf[81 * 1 + 2], L"Test {size=256; RAM}, Wrong 3rd byte in set 2 transfered by SPI");                                                       // The byte in 3rd position of the set 2 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[76 * 1], &TxTestBuf[81 * 1 + 3], (81 - 5)), L"Test {size=256; RAM}, Wrong bytes in set 2 transfered by SPI");                                      // Bytes transfered in the set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81 * 1], (81 - 5 + 3)), GET_CRC_AT(TxTestBuf, 81 * 2 - 2), L"Test {size=256; RAM}, Wrong CRC in set 2 of transfered data");                      // The CRC of data transfered in set 2 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5) * 2), TxTestBuf[81 * 2 + 0], L"Test {size=256; RAM}, Wrong 1st byte in set 3 transfered by SPI"); // The byte in 1st position of the set 3 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5) * 2), TxTestBuf[81 * 2 + 1], L"Test {size=256; RAM}, Wrong 2nd byte in set 3 transfered by SPI"); // The byte in 2nd position of the set 3 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)((81 - 5) >> 2), TxTestBuf[81 * 2 + 2], L"Test {size=256; RAM}, Wrong 3rd byte in set 3 transfered by SPI");                                                       // The byte in 3rd position of the set 3 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[76 * 2], &TxTestBuf[81 * 2 + 3], (81 - 5)), L"Test {size=256; RAM}, Wrong bytes in set 3 transfered by SPI");                                      // Bytes transfered in the set 3 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81 * 2], (81 - 5 + 3)), GET_CRC_AT(TxTestBuf, 81 * 3 - 2), L"Test {size=256; RAM}, Wrong CRC in set 3 of transfered data");                      // The CRC of data transfered in set 3 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5) * 3), TxTestBuf[81 * 3 + 0], L"Test {size=256; RAM}, Wrong 1st byte in set 4 transfered by SPI"); // The byte in 1st position of the set 4 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC, MCP251XFD_RAM_ADDR + (81 - 5) * 3), TxTestBuf[81 * 3 + 1], L"Test {size=256; RAM}, Wrong 2nd byte in set 4 transfered by SPI"); // The byte in 2nd position of the set 4 should be the same as the second control byte to send
      Assert::AreEqual((uint8_t)(28 >> 2), TxTestBuf[81 * 3 + 2], L"Test {size=256; RAM}, Wrong 3rd byte in set 4 transfered by SPI");                                                             // The byte in 3rd position of the set 4 should be the same as the length of data byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[76 * 3], &TxTestBuf[81 * 3 + 3], 28), L"Test {size=256; RAM}, Wrong bytes in set 4 transfered by SPI");                                            // Bytes transfered in the set 4 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[81 * 3], (28 + 3)), GET_CRC_AT(TxTestBuf, 81 * 3 + 3 + 28), L"Test {size=256; RAM}, Wrong CRC in set 4 of transfered data");                     // The CRC of data transfered in set 4 do not match the attempt one
    }



    TEST_METHOD(TestMethod_MCP251XFD_WriteData_SAFEWRITE)
    {
      Assert::AreEqual(81u, (uint32_t)MCP251XFD_TRANS_BUF_SIZE, L"Test can't perform with MCP251XFD_TRANS_BUF_SIZE != 81");
      Assert::IsTrue((MCP251XFD_TRANS_BUF_SIZE + 5) < TEST_BUF_COUNT, L"Test can't perform with TEST_BUF_COUNT too low");
      eERRORRESULT Error = ERR__TEST_ERROR;
      MCP251XFD MCP251XFD_TestSafeWrite;
      Init_MCP251XFDstruct(&MCP251XFD_TestSafeWrite, (setMCP251XFD_DriverConfig)(MCP251XFD_DRIVER_USE_READ_WRITE_CRC | MCP251XFD_DRIVER_USE_SAFE_WRITE)); // Init the structure

      //=============================================================================
      // Parameters check
      //=============================================================================
      //--- Test ((pComp == NULL) || (data == NULL)) ---
      Error = MCP251XFD_WriteData(NULL, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test ((pComp == NULL) || (data == NULL)), should be ERR__PARAMETER_ERROR");
      //--- Test (pComp == NULL) only ---
      Error = MCP251XFD_WriteData(NULL, 0, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (pComp == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (data == NULL) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, 0, NULL, 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (data == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (address > MCP251XFD_END_ADDR) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_END_ADDR + 1, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (address > MCP251XFD_END_ADDR), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer == NULL) only ---
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_END_ADDR, &TxTestBuf[0], 0);
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test (fnInterfaceTransfer == NULL), only should be ERR__PARAMETER_ERROR");
      //--- Test (fnInterfaceTransfer return error) only ---
      MCP251XFD_TestSafeWrite.fnComputeCRC16 = &ComputeCRC16CMS;                 // Set the compute CRC16-CMS function
      MCP251XFD_TestSafeWrite.fnSPI_Transfer = &InterfaceTransferError;
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, 0, &TxTestBuf[0], 1);
      Assert::AreEqual(ERR__TEST_ERROR, Error, L"Test (fnInterfaceTransfer return error), only should be ERR__TEST_ERROR");

      Assert::AreEqual(0u, TransferCounter, L"TransferCounter should be 0 at Parameters check"); // Should go 0 time in the InterfaceTransfer function
      Assert::AreEqual(0u, BytesTransfered, L"BytesTransfered should be 0 at Parameters check"); // Should have transfer 0 byte in the InterfaceTransfer function

      //=============================================================================
      // Read Data in SFR address
      //=============================================================================
      uint16_t CRCtoCheck = 0;                                              // This is where the CRC of the set will be stored
      uint8_t DataToSend[256];                                              // Send test table
      MCP251XFD_TestSafeWrite.fnComputeCRC16 = NULL;                        // Unset the compute CRC16-CMS function
      MCP251XFD_TestSafeWrite.fnSPI_Transfer = &InterfaceTransferReadWrite; // Set the tranfer interface function
      for (size_t z = 0; z <= 0xFF; z++) DataToSend[z] = (uint8_t)z;        // Generate test table, each entry equals to its index

      //--- Test (size = 0) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 0); // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size = 0}, should be ERR_OK");                                   // Check the return value
      Assert::AreEqual(0u, TransferCounter, L"Test {size = 0}, TransferCounter should be 0");                  // Should go 0 time in the InterfaceTransfer function

      //--- Test (size = 1; No CRC16SUB function) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                    // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_ReadData(&MCP251XFD_TestSafeWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 1);      // *** Test function
      Assert::AreEqual(ERR__PARAMETER_ERROR, Error, L"Test {size=1; SFR; NoCRC}, should be ERR__PARAMETER_ERROR"); // Check the return value

      MCP251XFD_TestSafeWrite.fnComputeCRC16 = &ComputeCRC16CMS;            // Set the compute CRC16-CMS function

      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                             // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 1);                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=1; SFR}, should be ERR_OK");                                                                                             // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=1; SFR}, TransferCounter should be 1");                                                                            // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(5u, BytesTransfered, L"Test {size=1; SFR}, BytesTransfered should be 5");                                                                            // Should have transfer 5 bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=1; SFR}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=1; SFR}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::AreEqual(DataToSend[0], TxTestBuf[2], L"Test {size=1; SFR}, Wrong 3rd byte transfered by SPI");                                                               // The byte in 3rd position should be the same as the first to send
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (2 + 1)), GET_CRC_AT(TxTestBuf, 3), L"Test {size=1; SFR}, Wrong CRC of transfered data");                             // The CRC of data transfered do not match the attempt one

      //--- Test (size = 2) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                              // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 2);                                                                               // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=2; SFR}, should be ERR_OK");                                                                                                              // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=2; SFR}, TransferCounter should be 2");                                                                                             // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual(10u, BytesTransfered, L"Test {size=2; SFR}, BytesTransfered should be 10");                                                                                           // Should have transfer full buffer + 1 byte bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[0], L"Test {size=2; SFR}, Wrong 1st byte in set 1 transfered by SPI");         // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR), TxTestBuf[1], L"Test {size=2; SFR}, Wrong 2nd byte in set 1 transfered by SPI");         // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::AreEqual(DataToSend[0], TxTestBuf[2], L"Test {size=2; SFR}, Wrong bytes in set 1 transfered by SPI");                                                                          // Bytes transfered in the set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], 3), GET_CRC_AT(TxTestBuf, 3), L"Test {size=2; SFR}, Wrong CRC in set 1 of transfered data");                                           // The CRC of data transfered in set 1 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + 1), TxTestBuf[5 + 0], L"Test {size=2; SFR}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + 1), TxTestBuf[5 + 1], L"Test {size=2; SFR}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::AreEqual(DataToSend[1], TxTestBuf[5 + 2], L"Test {size=2; SFR}, Wrong bytes in set 2 transfered by SPI");                                                                      // Bytes transfered in the set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[5], 3), GET_CRC_AT(TxTestBuf, 5 + 3), L"Test {size=2; SFR}, Wrong CRC in set 2 of transfered data");                                       // The CRC of data transfered in set 2 do not match the attempt one

      //--- Test (size = 256) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                               // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_CONTROLLER_SFR_ADDR, &DataToSend[0], 256);                                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; SFR}, should be ERR_OK");                                                                                                             // Check the return value
      Assert::AreEqual(256u, TransferCounter, L"Test {size=256; SFR}, TransferCounter should be 256");                                                                                        // Should go 256 times in the InterfaceTransfer function
      Assert::AreEqual(256u * 5, BytesTransfered, L"Test {size=256; SFR}, BytesTransfered should be 1280");                                                                                   // Should have transfer 3 bytes in the InterfaceTransfer function
      for (uint32_t z = 0; z < 256; z++)
      {
        Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + z), TxTestBuf[(z * 5) + 0], L"Test {size=256; SFR}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
        Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_CONTROLLER_SFR_ADDR + z), TxTestBuf[(z * 5) + 1], L"Test {size=256; SFR}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
        Assert::AreEqual(DataToSend[z], TxTestBuf[(z * 5) + 2], L"Test {size=256; SFR}, Wrong 3rd byte transfered by SPI");                                                                   // The byte in 3rd position should be the same as the first to send
        Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[(z * 5) + 0], (2 + 1)), GET_CRC_AT(TxTestBuf, (z * 5) + 3), L"Test {size=256; SFR}, Wrong CRC of transfered data");                       // The CRC of data transfered do not match the attempt one
      }



      //=============================================================================
      // Read Data in RAM address
      //=============================================================================
      //--- Test (size = 1) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                     // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 1); // *** Test function
      Assert::AreEqual(ERR__DATA_MODULO, Error, L"Test {size=1; RAM}, should be ERR__DATA_MODULO"); // Check the return value

      //--- Test (size = 4) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                  // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 4);                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=4; RAM}, should be ERR_OK");                                                                                  // Check the return value
      Assert::AreEqual(1u, TransferCounter, L"Test {size=4; RAM}, TransferCounter should be 1");                                                                 // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(8u, BytesTransfered, L"Test {size=4; RAM}, BytesTransfered should be 8");                                                                 // Should have transfer 8 bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR), TxTestBuf[0], L"Test {size=4; RAM}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR), TxTestBuf[1], L"Test {size=4; RAM}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[2], 4), L"Test {size=4; RAM}, Wrong bytes transfered by SPI");                                    // Bytes transfered should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (2 + 4)), GET_CRC_AT(TxTestBuf, 6), L"Test {size=4; RAM}, Wrong CRC of transfered data");                  // The CRC of data transfered do not match the attempt one

      //--- Test (size = 5) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                     // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 5); // *** Test function
      Assert::AreEqual(ERR__DATA_MODULO, Error, L"Test {size=5; RAM}, should be ERR__DATA_MODULO"); // Check the return value

      //--- Test (size = 8) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                   // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 8);                                                                               // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=8; RAM}, should be ERR_OK");                                                                                                   // Check the return value
      Assert::AreEqual(2u, TransferCounter, L"Test {size=8; RAM}, TransferCounter should be 2");                                                                                  // Should go 2 times in the InterfaceTransfer function
      Assert::AreEqual(16u, BytesTransfered, L"Test {size=8; RAM}, BytesTransfered should be 10");                                                                                // Should have transfer full buffer + 1 byte bytes in the InterfaceTransfer function
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR), TxTestBuf[0], L"Test {size=8; RAM}, Wrong 1st byte in set 1 transfered by SPI");         // The byte in 1st position of the set 1 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR), TxTestBuf[1], L"Test {size=8; RAM}, Wrong 2nd byte in set 1 transfered by SPI");         // The byte in 2nd position of the set 1 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[0], &TxTestBuf[2], 4), L"Test {size=8; RAM}, Wrong bytes in set 1 transfered by SPI");                                            // Bytes transfered in set 1 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[0], (2 + 4)), GET_CRC_AT(TxTestBuf, 6), L"Test {size=8; RAM}, Wrong CRC in set 1 of transfered data");                          // The CRC of data transfered in set 1 do not match the attempt one
      Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR + 4), TxTestBuf[8 + 0], L"Test {size=8; RAM}, Wrong 1st byte in set 2 transfered by SPI"); // The byte in 1st position of the set 2 should be the same as the first control byte to send
      Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR + 4), TxTestBuf[8 + 1], L"Test {size=8; RAM}, Wrong 2nd byte in set 2 transfered by SPI"); // The byte in 2nd position of the set 2 should be the same as the second control byte to send
      Assert::IsTrue(ArraysAreEqual(&DataToSend[4], &TxTestBuf[8 + 2], 4), L"Test {size=8; RAM}, Wrong bytes in set 2 transfered by SPI");                                        // Bytes transfered in set 2 should be the same as the bytes in DataToSend
      Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[8], (2 + 4)), GET_CRC_AT(TxTestBuf, 8 + 6), L"Test {size=8; RAM}, Wrong CRC in set 2 of transfered data");                      // The CRC of data transfered in set 2 do not match the attempt one

      //--- Test (size = 256) ---
      TransferCounter = 0; BytesTransfered = 0; TestBufPos = 0;                                                                                                                    // Reset InterfaceTransfer and BytesTransfered count to 0
      Error = MCP251XFD_WriteData(&MCP251XFD_TestSafeWrite, MCP251XFD_RAM_ADDR, &DataToSend[0], 256);                                                                              // *** Test function
      Assert::AreEqual(ERR_OK, Error, L"Test {size=256; RAM}, should be ERR_OK");                                                                                                  // Check the return value
      Assert::AreEqual(64u, TransferCounter, L"Test {size=256; RAM}, TransferCounter should be 64");                                                                               // Should go 1 time in the InterfaceTransfer function
      Assert::AreEqual(64u * 8, BytesTransfered, L"Test {size=256; RAM}, BytesTransfered should be 512");                                                                          // Should have transfer 512 bytes in the InterfaceTransfer function
      for (uint32_t z = 0; z < 256; z += 4)
      {
        Assert::AreEqual(CAAA_0(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR + z), TxTestBuf[(z * 2) + 0], L"Test {size=256; RAM}, Wrong 1st byte transfered by SPI"); // The byte in 1st position should be the same as the first control byte to send
        Assert::AreEqual(CAAA_1(MCP251XFD_SPI_INSTRUCTION_SAFE_WRITE, MCP251XFD_RAM_ADDR + z), TxTestBuf[(z * 2) + 1], L"Test {size=256; RAM}, Wrong 2nd byte transfered by SPI"); // The byte in 2nd position should be the same as the second control byte to send
        Assert::IsTrue(ArraysAreEqual(&DataToSend[z], &TxTestBuf[(z * 2) + 2], 4), L"Test {size=256; RAM}, Wrong bytes in set 2 transfered by SPI");                               // Bytes transfered in set 2 should be the same as the bytes in DataToSend
        Assert::AreEqual(ComputeCRC16CMS(&TxTestBuf[(z * 2) + 0], (2 + 4)), GET_CRC_AT(TxTestBuf, (z * 2) + 6), L"Test {size=256; RAM}, Wrong CRC of transfered data");            // The CRC of data transfered do not match the attempt one
      }
    }
  };
}