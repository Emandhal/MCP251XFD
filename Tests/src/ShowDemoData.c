/*******************************************************************************
  File name:    ShowDemoData.c
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 29/04/2020
  Description:  Some functions to show on console for the DEMO

  History :
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "Main.h"
#include "ShowDemoData.h"
#include "Conf_MCP251XFD.h"
#include "Console.h"
#include "Console_V71Interface.h"
#include "CANEXTFunctions.h"
#include "MCP251XFD_V71InterfaceSync.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
#include "stdafx.h"
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------



//**********************************************************************************************************************************************************
const char EXTStringsNames[2][4+1/* \0 */] =
{
  "EXT1",
  "EXT2",
};

#define GetExtFromComponent  (1u - ((uint8_t)(pComp == &MCP251XFD_Ext1) & 0x1))
#define EXTx                 EXTStringsNames[GetExtFromComponent]



//-----------------------------------------------------------------------------
const uint8_t CSIdx[2] = {SPI_CS_EXT1, SPI_CS_EXT2};

//=============================================================================
// Show which device is detected on the console
//=============================================================================
void ShowDeviceDetected(MCP251XFD *pComp, uint32_t sysclk)
{
  eERRORRESULT Error;
  uint8_t Id = 0, Revision = 0;
  eMCP251XFD_Devices Dev = MCP2517FD;
  Error = MCP251XFD_GetDeviceID(pComp, &Dev, &Id, &Revision);
  if (Error == ERR_OK)
  {
    unsigned int FsckI  =  sysclk / 1000000;                // Integer form of MHz
		unsigned int FsckD  = (sysclk / 1000) - (FsckI * 1000); // Decimal form of MHz with 3 digits of precision
    unsigned int SpiClk = GetSPIClock(SPI0, CSIdx[GetExtFromComponent]);
    unsigned int FspiI  =  SpiClk / 1000000;                // Integer form of MHz
    unsigned int FspiD  = (SpiClk / 1000) - (FspiI * 1000); // Decimal form of MHz with 3 digits of precision
    if (Dev == MCP2517FD)
         LOGINFO("%s: Device detected %s\t\t   @ %u.%uMHz (SPI @ %u.%uMHz)"             , EXTx, MCP251XFD_DevicesNames[Dev],               FsckI, FsckD, FspiI, FspiD);
    else LOGINFO("%s: Device detected %s (Id:0x%X Rev:0x%X) @ %u.%uMHz (SPI @ %u.%uMHz)", EXTx, MCP251XFD_DevicesNames[Dev], Id, Revision, FsckI, FsckD, FspiI, FspiD);
  }
  else ShowDeviceError(pComp, Error);
}





//=============================================================================
// Show device configuration
//=============================================================================
void ShowDeviceConfiguration(MCP251XFD_BitTimeStats* btStats)
{
  if (btStats == NULL) return;
  char SamplePoint[10];

  //--- Show nominal bitrate infos ---
  unsigned int NominalkBps = btStats->NominalBitrate / 1000u;
  Float_ToString((float)btStats->NSamplePoint / 100.0,  &SamplePoint[0], sizeof(SamplePoint), 2, 2, true);
  LOGINFO("      Nominal Bitrate: %ukbps, Sample Point: %s (Max bus length: %um)", NominalkBps, SamplePoint, (unsigned int)btStats->MaxBusLength);

  //--- Show data bitrate infos ---
  if (btStats->DataBitrate != 0)
  {
    unsigned int DatakBps = btStats->DataBitrate / 1000u;
    Float_ToString((float)btStats->DSamplePoint / 100.0,  &SamplePoint[0], sizeof(SamplePoint), 2, 2, true);
    LOGINFO("      Data    Bitrate: %ukbps, Sample Point: %s", DatakBps, SamplePoint);
  }
}





//=============================================================================
// Show device FIFO configuration
//=============================================================================
const char YesNoNames[3][3+1/* \0 */] =
{
  " NO",
  "YES",
  "N/A",
};

const char MCP251XFD_FIFONames[MCP251XFD_FIFO_COUNT][7] =
{
  "  TXQ ",
  " FIFO1",
  " FIFO2",
  " FIFO3",
  " FIFO4",
  " FIFO5",
  " FIFO6",
  " FIFO7",
  " FIFO8",
  " FIFO9",
  "FIFO10",
  "FIFO11",
  "FIFO12",
  "FIFO13",
  "FIFO14",
  "FIFO15",
  "FIFO16",
  "FIFO17",
  "FIFO18",
  "FIFO19",
  "FIFO20",
  "FIFO21",
  "FIFO22",
  "FIFO23",
  "FIFO24",
  "FIFO25",
  "FIFO26",
  "FIFO27",
  "FIFO28",
  "FIFO29",
  "FIFO30",
  "FIFO31",
};

void ShowDeviceFIFOConfiguration(MCP251XFD_FIFO *listFIFO, size_t count)
{
  if (listFIFO == NULL) return;

  //--- Some Checks ---
  for (size_t z = 0; z < count; z++)
    if (listFIFO[z].RAMInfos == NULL)
    {
      LOGDEBUG("      Can't show FIFO configuration because one or more RAMInfos are missing");
      return;
    }

  //--- Show RAM used ---
  uint32_t TotalRAM = 0;
  for (size_t z = 0; z < count; z++) TotalRAM += listFIFO[z].RAMInfos->ByteInFIFO;
  LOGDEBUG("      Total RAM used: %u Bytes", (unsigned int)TotalRAM);

  //--- Show table header ---
  LOGDEBUG("      ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄ¿");
  LOGDEBUG("      ³  FIFO  ³ Obj ³ Payload ³ TS  ³ ByteObj ³ ByteFIFO ³ StartAddr ³ EndAddr ³");
  LOGDEBUG("      ÃÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄ´");

  //--- Show TEF infos ---
  for (size_t z = 0; z < count; z++)
    if (listFIFO[z].Name == MCP251XFD_TEF)
    {
      MCP251XFD_RAMInfos *RI = listFIFO[z].RAMInfos;
      size_t TS = ((listFIFO[z].ControlFlags & MCP251XFD_FIFO_ADD_TIMESTAMP_ON_OBJ) > 0 ? 1 : 0);
      LOGDEBUG("      ³  TEF   ³  %2u ³   N/A   ³ %s ³    %2u   ³   %4u   ³   0x%3X   ³  0x%3X  ³", (unsigned int)(listFIFO[z].Size+1), YesNoNames[TS], (unsigned int)RI->ByteInObject, (unsigned int)RI->ByteInFIFO, (unsigned int)RI->RAMStartAddress, (unsigned int)(RI->RAMStartAddress+RI->ByteInFIFO));
      break;
    }

  //--- Show TXQ infos ---
  for (size_t z = 0; z < count; z++)
    if (listFIFO[z].Name == MCP251XFD_TXQ)
    {
      MCP251XFD_RAMInfos *RI = listFIFO[z].RAMInfos;
      LOGDEBUG("      ³  TXQ   ³  %2u ³    %2u   ³ N/A ³    %2u   ³   %4u   ³   0x%3X   ³  0x%3X  ³", (unsigned int)(listFIFO[z].Size+1), MCP251XFD_PayloadToByte(listFIFO[z].Payload), (unsigned int)RI->ByteInObject, (unsigned int)RI->ByteInFIFO, (unsigned int)RI->RAMStartAddress, (unsigned int)(RI->RAMStartAddress+RI->ByteInFIFO));
      break;
    }

  //--- Show FIFOs infos ---
  for (int32_t zFIFO = 1; zFIFO < MCP251XFD_FIFO_COUNT; zFIFO++)
    for (size_t z = 0; z < count; z++)
      if (listFIFO[z].Name == zFIFO)
      {
        MCP251XFD_RAMInfos *RI = listFIFO[z].RAMInfos;
        size_t TS = ((listFIFO[z].ControlFlags & MCP251XFD_FIFO_ADD_TIMESTAMP_ON_RX) > 0 ? 1 : 0);
        if (listFIFO[z].Direction == MCP251XFD_TRANSMIT_FIFO) TS = 2;
        LOGDEBUG("      ³ %s ³  %2u ³    %2u   ³ %s ³    %2u   ³   %4u   ³   0x%3X   ³  0x%3X  ³", MCP251XFD_FIFONames[zFIFO], (unsigned int)(listFIFO[z].Size+1), MCP251XFD_PayloadToByte(listFIFO[z].Payload), YesNoNames[TS], (unsigned int)RI->ByteInObject, (unsigned int)RI->ByteInFIFO, (unsigned int)RI->RAMStartAddress, (unsigned int)(RI->RAMStartAddress+RI->ByteInFIFO));
        break;
      }

  //--- Show table closing ---
  LOGDEBUG("      ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÙ");
}





//=============================================================================
// Show device Filter configuration
//=============================================================================
const char MCP251XFD_FilterNames[MCP251XFD_FILTER_COUNT][9] =
{
  "Filter 0",
  "Filter 1",
  "Filter 2",
  "Filter 3",
  "Filter 4",
  "Filter 5",
  "Filter 6",
  "Filter 7",
  "Filter 8",
  "Filter 9",
  "Filter10",
  "Filter11",
  "Filter12",
  "Filter13",
  "Filter14",
  "Filter15",
  "Filter16",
  "Filter17",
  "Filter18",
  "Filter19",
  "Filter20",
  "Filter21",
  "Filter22",
  "Filter23",
  "Filter24",
  "Filter25",
  "Filter26",
  "Filter27",
  "Filter28",
  "Filter29",
  "Filter30",
  "Filter31",
};

static char GetCorrespondingCharOf(uint32_t acceptanceID, uint32_t acceptanceMask, uint8_t bit)
{
  if ((acceptanceMask & (1 << bit)) > 0) // Corresponding bit Mask set ?
  {
    return ((acceptanceID & (1 << bit)) > 0) ? '1' : '0'; // Put the Address Acceptance actual bit
  }
  else return '.'; // Else set a '.' to indicate "don't care"
}

void ShowDeviceFilterConfiguration(MCP251XFD_Filter *listFilter, size_t count, bool UseSID11)
{
  if (listFilter == NULL) return;
  #define ADDR_BITMAP_MAX  ( MCP251XFD_EID_Size + 1 + MCP251XFD_SID_Size )

  char AcceptBitMap[ADDR_BITMAP_MAX+1]; // 30bits + /0
  AcceptBitMap[ADDR_BITMAP_MAX] = 0;    // Set the end char

  //--- Show table header ---
  LOGDEBUG("      ÚÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄ¿");
  LOGDEBUG("      ³   Name   ³ Addr: SID            EID       ³ Point To ³");
  LOGDEBUG("      ÃÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄ´");

  //--- Show Filters infos ---
  for (size_t z = 0; z < count; z++)
  {
    for (int32_t zChar = ADDR_BITMAP_MAX; --zChar >= 0;) AcceptBitMap[zChar] = ' '; // Clear bitmap

    switch (listFilter[z].Match)
    {
      case MCP251XFD_MATCH_ONLY_SID:
      {
        for (int32_t zChar = (MCP251XFD_SID_Size+1); --zChar >= 0;)
        {
          if (UseSID11 && (zChar == MCP251XFD_SID_Size))
               AcceptBitMap[MCP251XFD_SID_Size-zChar] = GetCorrespondingCharOf(listFilter[z].AcceptanceID, listFilter[z].AcceptanceMask, zChar);
          else
          {
            if (!UseSID11 && (zChar == MCP251XFD_SID_Size)) AcceptBitMap[MCP251XFD_SID_Size-zChar] = ' ';
            else AcceptBitMap[MCP251XFD_SID_Size-zChar] = GetCorrespondingCharOf(listFilter[z].AcceptanceID, listFilter[z].AcceptanceMask, zChar);
          }
        }
      }
    	break;
      case MCP251XFD_MATCH_ONLY_EID:
      /*{
        for (int32_t zChar = EID_Size; --zChar >= 0;)
        AcceptBitMap[SID_Size+EID_Size-zChar] = GetCorrespondingCharOf(listFilter[z].AcceptanceID, listFilter[z].AcceptanceMask, zChar);
      }
      break;*/
      case MCP251XFD_MATCH_SID_EID:
      {
        for (int32_t zChar = (MCP251XFD_SID_Size+MCP251XFD_EID_Size+1); --zChar >= 0;)
        {
          if (UseSID11 && (zChar == (MCP251XFD_SID_Size+MCP251XFD_EID_Size)))
               AcceptBitMap[MCP251XFD_SID_Size+MCP251XFD_EID_Size-zChar] = GetCorrespondingCharOf(listFilter[z].AcceptanceID, listFilter[z].AcceptanceMask, zChar);
          else
          {
            if (!UseSID11 && (zChar == (MCP251XFD_SID_Size+MCP251XFD_EID_Size))) AcceptBitMap[MCP251XFD_SID_Size+MCP251XFD_EID_Size-zChar] = ' ';
            else AcceptBitMap[MCP251XFD_SID_Size+MCP251XFD_EID_Size-zChar] = GetCorrespondingCharOf(listFilter[z].AcceptanceID, listFilter[z].AcceptanceMask, zChar);
          }
        }
      }
      break;
    }
    LOGDEBUG("      ³ %s ³ %s -> %s  ³", MCP251XFD_FilterNames[listFilter[z].Filter], AcceptBitMap, MCP251XFD_FIFONames[listFilter[z].PointTo]);
  }

  //--- Show table closing ---
  LOGDEBUG("      ÀÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
}





//=============================================================================
// Show more device configuration
//=============================================================================
void ShowMoreDeviceConfiguration(MCP251XFD *pComp)
{
#ifdef USE_MCP251XFD_TOOLS
  eMCP251XFD_Devices Dev = MCP2517FD;
  MCP251XFD_GetDeviceID(pComp, &Dev, NULL, NULL);
  uint32_t GPIOchangeSpeed = CalculateMinTimeBeforeGPIOChangeState_V71(pComp, SPI0, CSIdx[GetExtFromComponent]);
  uint32_t FreqChange = (uint32_t)(1000000000.0f / ((float)GPIOchangeSpeed * 2.0f));
  LOGDEBUG("      GPIO change speed: %uns (Continuous change: %uHz)", (unsigned int)GPIOchangeSpeed, (unsigned int)FreqChange);
#endif
}





//**********************************************************************************************************************************************************
typedef union
{
  unsigned int Uint32[MCP251XFD_CONTROLLER_SFR_SIZE / sizeof(unsigned int)];
  uint8_t      Bytes[MCP251XFD_CONTROLLER_SFR_SIZE / sizeof(uint8_t)];
} ControllerSFRData;

void GetAndShowMCP251XFD_SFRreg(MCP251XFD *pComp)
{
  ControllerSFRData CSD;
  eERRORRESULT Error = MCP251XFD_ReadData(pComp, MCP251XFD_CONTROLLER_SFR_ADDR, &CSD.Bytes[0], MCP251XFD_CONTROLLER_SFR_SIZE);
  if (Error == ERR_OK)
  {
    LOGINFO("%s: MCP251XFD Special Function Registers:", EXTx);
    LOGINFO("%s:   OSC     = 0x%08X", EXTx, CSD.Uint32[0]);
    LOGINFO("%s:   IOCON   = 0x%08X", EXTx, CSD.Uint32[1]);
    LOGINFO("%s:   CRC     = 0x%08X", EXTx, CSD.Uint32[2]);
    LOGINFO("%s:   ECCCON  = 0x%08X", EXTx, CSD.Uint32[3]);
    LOGINFO("%s:   ECCSTAT = 0x%08X", EXTx, CSD.Uint32[4]);
    LOGINFO("%s:   DEVID   = 0x%08X", EXTx, CSD.Uint32[5]);
  }
  else ShowDeviceError(pComp, Error);
}



#define CAN_SFR_SIZE  ( 16 * sizeof(unsigned int) )

typedef union
{
  unsigned int Uint32[CAN_SFR_SIZE / sizeof(unsigned int)];
  uint8_t      Bytes[CAN_SFR_SIZE / sizeof(uint8_t)];
} CANControllerSFRData;

void GetAndShowMCP251XFD_CANSFRreg(MCP251XFD *pComp)
{
  CANControllerSFRData CCSD;
  eERRORRESULT Error = MCP251XFD_ReadData(pComp, MCP251XFD_CAN_CONTROLLER_ADDR, &CCSD.Bytes[0], CAN_SFR_SIZE);
  if (Error == ERR_OK)
  {
    LOGINFO("%s: CAN Controller Special Function Registers:", EXTx);
    LOGINFO("%s:   C1CON    = 0x%08X", EXTx, CCSD.Uint32[ 0]);
    LOGINFO("%s:   C1NBTCFG = 0x%08X", EXTx, CCSD.Uint32[ 1]);
    LOGINFO("%s:   C1DBTCFG = 0x%08X", EXTx, CCSD.Uint32[ 2]);
    LOGINFO("%s:   C1TDC    = 0x%08X", EXTx, CCSD.Uint32[ 3]);
    LOGINFO("%s:   C1TBC    = 0x%08X", EXTx, CCSD.Uint32[ 4]);
    LOGINFO("%s:   C1TSCON  = 0x%08X", EXTx, CCSD.Uint32[ 5]);
    LOGINFO("%s:   C1VEC    = 0x%08X", EXTx, CCSD.Uint32[ 6]);
    LOGINFO("%s:   C1INT    = 0x%08X", EXTx, CCSD.Uint32[ 7]);
    LOGINFO("%s:   C1RXIF   = 0x%08X", EXTx, CCSD.Uint32[ 8]);
    LOGINFO("%s:   C1TXIF   = 0x%08X", EXTx, CCSD.Uint32[ 9]);
    LOGINFO("%s:   C1RXOVIF = 0x%08X", EXTx, CCSD.Uint32[10]);
    LOGINFO("%s:   C1TXATIF = 0x%08X", EXTx, CCSD.Uint32[11]);
    LOGINFO("%s:   C1TXREQ  = 0x%08X", EXTx, CCSD.Uint32[12]);
    LOGINFO("%s:   C1TREC   = 0x%08X", EXTx, CCSD.Uint32[13]);
    LOGINFO("%s:   C1BDIAG0 = 0x%08X", EXTx, CCSD.Uint32[14]);
    LOGINFO("%s:   C1BDIAG1 = 0x%08X", EXTx, CCSD.Uint32[15]);
  }
  else ShowDeviceError(pComp, Error);
}



#define CAN_SFR_FIFO_SIZE  ( (3+1+3+3*31) * sizeof(unsigned int) )

typedef union
{
  unsigned int Uint32[CAN_SFR_FIFO_SIZE / sizeof(unsigned int)];
  uint8_t      Bytes[CAN_SFR_FIFO_SIZE / sizeof(uint8_t)];
} FIFOSFRData;

void GetAndShowMCP251XFD_FIFOreg(MCP251XFD *pComp)
{
  FIFOSFRData FSD;
  eERRORRESULT Error = MCP251XFD_ReadData(pComp, RegMCP251XFD_CiTEFCON, &FSD.Bytes[0], CAN_SFR_FIFO_SIZE);
  if (Error == ERR_OK)
  {
    LOGINFO("%s: CAN Controller FIFOs:", EXTx);
    LOGINFO("%s:   TEF   : C1TEFCON    = 0x%08X ; C1TEFSTA    = 0x%08X ; C1TEFUA    = 0x%08X", EXTx, FSD.Uint32[ 0], FSD.Uint32[ 1], FSD.Uint32[ 2]);
    LOGINFO("%s:   TXQ   : C1TXQCON    = 0x%08X ; C1TXQSTA    = 0x%08X ; C1TXQUA    = 0x%08X", EXTx, FSD.Uint32[ 4], FSD.Uint32[ 5], FSD.Uint32[ 6]);
    LOGINFO("%s:   FIFO1 : C1FIFOCON1  = 0x%08X ; C1FIFOSTA1  = 0x%08X ; C1FIFOUA1  = 0x%08X", EXTx, FSD.Uint32[ 7], FSD.Uint32[ 8], FSD.Uint32[ 9]);
    LOGINFO("%s:   FIFO2 : C1FIFOCON2  = 0x%08X ; C1FIFOSTA2  = 0x%08X ; C1FIFOUA2  = 0x%08X", EXTx, FSD.Uint32[10], FSD.Uint32[11], FSD.Uint32[12]);
    LOGINFO("%s:   FIFO3 : C1FIFOCON3  = 0x%08X ; C1FIFOSTA3  = 0x%08X ; C1FIFOUA3  = 0x%08X", EXTx, FSD.Uint32[13], FSD.Uint32[14], FSD.Uint32[15]);
    LOGINFO("%s:   FIFO4 : C1FIFOCON4  = 0x%08X ; C1FIFOSTA4  = 0x%08X ; C1FIFOUA4  = 0x%08X", EXTx, FSD.Uint32[16], FSD.Uint32[17], FSD.Uint32[18]);
    LOGINFO("%s:   FIFO5 : C1FIFOCON5  = 0x%08X ; C1FIFOSTA5  = 0x%08X ; C1FIFOUA5  = 0x%08X", EXTx, FSD.Uint32[19], FSD.Uint32[20], FSD.Uint32[21]);
    LOGINFO("%s:   FIFO6 : C1FIFOCON6  = 0x%08X ; C1FIFOSTA6  = 0x%08X ; C1FIFOUA6  = 0x%08X", EXTx, FSD.Uint32[22], FSD.Uint32[23], FSD.Uint32[24]);
    LOGINFO("%s:   FIFO7 : C1FIFOCON7  = 0x%08X ; C1FIFOSTA7  = 0x%08X ; C1FIFOUA7  = 0x%08X", EXTx, FSD.Uint32[25], FSD.Uint32[26], FSD.Uint32[27]);
    LOGINFO("%s:   FIFO8 : C1FIFOCON8  = 0x%08X ; C1FIFOSTA8  = 0x%08X ; C1FIFOUA8  = 0x%08X", EXTx, FSD.Uint32[28], FSD.Uint32[29], FSD.Uint32[30]);
    LOGINFO("%s:   FIFO9 : C1FIFOCON9  = 0x%08X ; C1FIFOSTA9  = 0x%08X ; C1FIFOUA9  = 0x%08X", EXTx, FSD.Uint32[31], FSD.Uint32[32], FSD.Uint32[33]);
    LOGINFO("%s:   FIFO10: C1FIFOCON10 = 0x%08X ; C1FIFOSTA10 = 0x%08X ; C1FIFOUA10 = 0x%08X", EXTx, FSD.Uint32[34], FSD.Uint32[35], FSD.Uint32[36]);
    LOGINFO("%s:   FIFO11: C1FIFOCON11 = 0x%08X ; C1FIFOSTA11 = 0x%08X ; C1FIFOUA11 = 0x%08X", EXTx, FSD.Uint32[37], FSD.Uint32[38], FSD.Uint32[39]);
    LOGINFO("%s:   FIFO12: C1FIFOCON12 = 0x%08X ; C1FIFOSTA12 = 0x%08X ; C1FIFOUA12 = 0x%08X", EXTx, FSD.Uint32[40], FSD.Uint32[41], FSD.Uint32[42]);
    LOGINFO("%s:   FIFO13: C1FIFOCON13 = 0x%08X ; C1FIFOSTA13 = 0x%08X ; C1FIFOUA13 = 0x%08X", EXTx, FSD.Uint32[43], FSD.Uint32[44], FSD.Uint32[45]);
    LOGINFO("%s:   FIFO14: C1FIFOCON14 = 0x%08X ; C1FIFOSTA14 = 0x%08X ; C1FIFOUA14 = 0x%08X", EXTx, FSD.Uint32[46], FSD.Uint32[47], FSD.Uint32[48]);
    LOGINFO("%s:   FIFO15: C1FIFOCON15 = 0x%08X ; C1FIFOSTA15 = 0x%08X ; C1FIFOUA15 = 0x%08X", EXTx, FSD.Uint32[49], FSD.Uint32[50], FSD.Uint32[51]);
    LOGINFO("%s:   FIFO16: C1FIFOCON16 = 0x%08X ; C1FIFOSTA16 = 0x%08X ; C1FIFOUA16 = 0x%08X", EXTx, FSD.Uint32[52], FSD.Uint32[53], FSD.Uint32[54]);
    LOGINFO("%s:   FIFO17: C1FIFOCON17 = 0x%08X ; C1FIFOSTA17 = 0x%08X ; C1FIFOUA17 = 0x%08X", EXTx, FSD.Uint32[55], FSD.Uint32[56], FSD.Uint32[57]);
    LOGINFO("%s:   FIFO18: C1FIFOCON18 = 0x%08X ; C1FIFOSTA18 = 0x%08X ; C1FIFOUA18 = 0x%08X", EXTx, FSD.Uint32[58], FSD.Uint32[59], FSD.Uint32[60]);
    LOGINFO("%s:   FIFO19: C1FIFOCON19 = 0x%08X ; C1FIFOSTA19 = 0x%08X ; C1FIFOUA19 = 0x%08X", EXTx, FSD.Uint32[61], FSD.Uint32[62], FSD.Uint32[63]);
    LOGINFO("%s:   FIFO20: C1FIFOCON20 = 0x%08X ; C1FIFOSTA20 = 0x%08X ; C1FIFOUA20 = 0x%08X", EXTx, FSD.Uint32[64], FSD.Uint32[65], FSD.Uint32[66]);
    LOGINFO("%s:   FIFO21: C1FIFOCON21 = 0x%08X ; C1FIFOSTA21 = 0x%08X ; C1FIFOUA21 = 0x%08X", EXTx, FSD.Uint32[67], FSD.Uint32[68], FSD.Uint32[69]);
    LOGINFO("%s:   FIFO22: C1FIFOCON22 = 0x%08X ; C1FIFOSTA22 = 0x%08X ; C1FIFOUA22 = 0x%08X", EXTx, FSD.Uint32[70], FSD.Uint32[71], FSD.Uint32[72]);
    LOGINFO("%s:   FIFO23: C1FIFOCON23 = 0x%08X ; C1FIFOSTA23 = 0x%08X ; C1FIFOUA23 = 0x%08X", EXTx, FSD.Uint32[73], FSD.Uint32[74], FSD.Uint32[75]);
    LOGINFO("%s:   FIFO24: C1FIFOCON24 = 0x%08X ; C1FIFOSTA24 = 0x%08X ; C1FIFOUA24 = 0x%08X", EXTx, FSD.Uint32[76], FSD.Uint32[77], FSD.Uint32[78]);
    LOGINFO("%s:   FIFO25: C1FIFOCON25 = 0x%08X ; C1FIFOSTA25 = 0x%08X ; C1FIFOUA25 = 0x%08X", EXTx, FSD.Uint32[79], FSD.Uint32[80], FSD.Uint32[81]);
    LOGINFO("%s:   FIFO26: C1FIFOCON26 = 0x%08X ; C1FIFOSTA26 = 0x%08X ; C1FIFOUA26 = 0x%08X", EXTx, FSD.Uint32[82], FSD.Uint32[83], FSD.Uint32[84]);
    LOGINFO("%s:   FIFO27: C1FIFOCON27 = 0x%08X ; C1FIFOSTA27 = 0x%08X ; C1FIFOUA27 = 0x%08X", EXTx, FSD.Uint32[85], FSD.Uint32[86], FSD.Uint32[87]);
    LOGINFO("%s:   FIFO28: C1FIFOCON28 = 0x%08X ; C1FIFOSTA28 = 0x%08X ; C1FIFOUA28 = 0x%08X", EXTx, FSD.Uint32[88], FSD.Uint32[89], FSD.Uint32[90]);
    LOGINFO("%s:   FIFO29: C1FIFOCON29 = 0x%08X ; C1FIFOSTA29 = 0x%08X ; C1FIFOUA29 = 0x%08X", EXTx, FSD.Uint32[91], FSD.Uint32[92], FSD.Uint32[93]);
    LOGINFO("%s:   FIFO30: C1FIFOCON30 = 0x%08X ; C1FIFOSTA30 = 0x%08X ; C1FIFOUA30 = 0x%08X", EXTx, FSD.Uint32[94], FSD.Uint32[95], FSD.Uint32[96]);
    LOGINFO("%s:   FIFO31: C1FIFOCON31 = 0x%08X ; C1FIFOSTA31 = 0x%08X ; C1FIFOUA31 = 0x%08X", EXTx, FSD.Uint32[97], FSD.Uint32[98], FSD.Uint32[99]);
  }
  else ShowDeviceError(pComp, Error);
}



#define CAN_SFR_FILTER_SIZE  ( (8+2*32) * sizeof(unsigned int) )

typedef union
{
  unsigned int Uint32[CAN_SFR_FILTER_SIZE / sizeof(unsigned int)];
  uint8_t      Bytes[CAN_SFR_FILTER_SIZE / sizeof(uint8_t)];
} FilterSFRData;

void GetAndShowMCP251XFD_FILTERreg(MCP251XFD *pComp)
{
  FIFOSFRData FSD;
  eERRORRESULT Error = MCP251XFD_ReadData(pComp, RegMCP251XFD_CiFLTCON0, &FSD.Bytes[0], CAN_SFR_FILTER_SIZE);
  if (Error == ERR_OK)
  {
    LOGINFO("%s: CAN Controller Filters:", EXTx);
    LOGINFO("%s:   Filter0 : C1FLTCON0 = 0x%02X ; C1FLTOBJ0  = 0x%08X ; C1MASK0  = 0x%08X", EXTx, FSD.Bytes[ 0], FSD.Uint32[ 8], FSD.Uint32[ 9]);
    LOGINFO("%s:   Filter1 : C1FLTCON0 = 0x%02X ; C1FLTOBJ1  = 0x%08X ; C1MASK1  = 0x%08X", EXTx, FSD.Bytes[ 1], FSD.Uint32[10], FSD.Uint32[11]);
    LOGINFO("%s:   Filter2 : C1FLTCON0 = 0x%02X ; C1FLTOBJ2  = 0x%08X ; C1MASK2  = 0x%08X", EXTx, FSD.Bytes[ 2], FSD.Uint32[12], FSD.Uint32[13]);
    LOGINFO("%s:   Filter3 : C1FLTCON0 = 0x%02X ; C1FLTOBJ3  = 0x%08X ; C1MASK3  = 0x%08X", EXTx, FSD.Bytes[ 3], FSD.Uint32[14], FSD.Uint32[15]);
    LOGINFO("%s:   Filter4 : C1FLTCON1 = 0x%02X ; C1FLTOBJ4  = 0x%08X ; C1MASK4  = 0x%08X", EXTx, FSD.Bytes[ 4], FSD.Uint32[16], FSD.Uint32[17]);
    LOGINFO("%s:   Filter5 : C1FLTCON1 = 0x%02X ; C1FLTOBJ5  = 0x%08X ; C1MASK5  = 0x%08X", EXTx, FSD.Bytes[ 5], FSD.Uint32[18], FSD.Uint32[19]);
    LOGINFO("%s:   Filter6 : C1FLTCON1 = 0x%02X ; C1FLTOBJ6  = 0x%08X ; C1MASK6  = 0x%08X", EXTx, FSD.Bytes[ 6], FSD.Uint32[20], FSD.Uint32[21]);
    LOGINFO("%s:   Filter7 : C1FLTCON1 = 0x%02X ; C1FLTOBJ7  = 0x%08X ; C1MASK7  = 0x%08X", EXTx, FSD.Bytes[ 7], FSD.Uint32[22], FSD.Uint32[23]);
    LOGINFO("%s:   Filter8 : C1FLTCON2 = 0x%02X ; C1FLTOBJ8  = 0x%08X ; C1MASK8  = 0x%08X", EXTx, FSD.Bytes[ 8], FSD.Uint32[24], FSD.Uint32[25]);
    LOGINFO("%s:   Filter9 : C1FLTCON2 = 0x%02X ; C1FLTOBJ9  = 0x%08X ; C1MASK9  = 0x%08X", EXTx, FSD.Bytes[ 9], FSD.Uint32[26], FSD.Uint32[27]);
    LOGINFO("%s:   Filter10: C1FLTCON2 = 0x%02X ; C1FLTOBJ10 = 0x%08X ; C1MASK10 = 0x%08X", EXTx, FSD.Bytes[10], FSD.Uint32[28], FSD.Uint32[29]);
    LOGINFO("%s:   Filter11: C1FLTCON2 = 0x%02X ; C1FLTOBJ11 = 0x%08X ; C1MASK11 = 0x%08X", EXTx, FSD.Bytes[11], FSD.Uint32[30], FSD.Uint32[31]);
    LOGINFO("%s:   Filter12: C1FLTCON3 = 0x%02X ; C1FLTOBJ12 = 0x%08X ; C1MASK12 = 0x%08X", EXTx, FSD.Bytes[12], FSD.Uint32[32], FSD.Uint32[33]);
    LOGINFO("%s:   Filter13: C1FLTCON3 = 0x%02X ; C1FLTOBJ13 = 0x%08X ; C1MASK13 = 0x%08X", EXTx, FSD.Bytes[13], FSD.Uint32[34], FSD.Uint32[35]);
    LOGINFO("%s:   Filter14: C1FLTCON3 = 0x%02X ; C1FLTOBJ14 = 0x%08X ; C1MASK14 = 0x%08X", EXTx, FSD.Bytes[14], FSD.Uint32[36], FSD.Uint32[37]);
    LOGINFO("%s:   Filter15: C1FLTCON3 = 0x%02X ; C1FLTOBJ15 = 0x%08X ; C1MASK15 = 0x%08X", EXTx, FSD.Bytes[15], FSD.Uint32[38], FSD.Uint32[39]);
    LOGINFO("%s:   Filter16: C1FLTCON4 = 0x%02X ; C1FLTOBJ16 = 0x%08X ; C1MASK16 = 0x%08X", EXTx, FSD.Bytes[16], FSD.Uint32[40], FSD.Uint32[41]);
    LOGINFO("%s:   Filter17: C1FLTCON4 = 0x%02X ; C1FLTOBJ17 = 0x%08X ; C1MASK17 = 0x%08X", EXTx, FSD.Bytes[17], FSD.Uint32[42], FSD.Uint32[43]);
    LOGINFO("%s:   Filter18: C1FLTCON4 = 0x%02X ; C1FLTOBJ18 = 0x%08X ; C1MASK18 = 0x%08X", EXTx, FSD.Bytes[18], FSD.Uint32[44], FSD.Uint32[45]);
    LOGINFO("%s:   Filter19: C1FLTCON4 = 0x%02X ; C1FLTOBJ19 = 0x%08X ; C1MASK19 = 0x%08X", EXTx, FSD.Bytes[19], FSD.Uint32[46], FSD.Uint32[47]);
    LOGINFO("%s:   Filter20: C1FLTCON5 = 0x%02X ; C1FLTOBJ20 = 0x%08X ; C1MASK20 = 0x%08X", EXTx, FSD.Bytes[20], FSD.Uint32[48], FSD.Uint32[49]);
    LOGINFO("%s:   Filter21: C1FLTCON5 = 0x%02X ; C1FLTOBJ21 = 0x%08X ; C1MASK21 = 0x%08X", EXTx, FSD.Bytes[21], FSD.Uint32[50], FSD.Uint32[51]);
    LOGINFO("%s:   Filter22: C1FLTCON5 = 0x%02X ; C1FLTOBJ22 = 0x%08X ; C1MASK22 = 0x%08X", EXTx, FSD.Bytes[22], FSD.Uint32[52], FSD.Uint32[53]);
    LOGINFO("%s:   Filter23: C1FLTCON5 = 0x%02X ; C1FLTOBJ23 = 0x%08X ; C1MASK23 = 0x%08X", EXTx, FSD.Bytes[23], FSD.Uint32[54], FSD.Uint32[55]);
    LOGINFO("%s:   Filter24: C1FLTCON6 = 0x%02X ; C1FLTOBJ24 = 0x%08X ; C1MASK24 = 0x%08X", EXTx, FSD.Bytes[24], FSD.Uint32[56], FSD.Uint32[57]);
    LOGINFO("%s:   Filter25: C1FLTCON6 = 0x%02X ; C1FLTOBJ25 = 0x%08X ; C1MASK25 = 0x%08X", EXTx, FSD.Bytes[25], FSD.Uint32[58], FSD.Uint32[59]);
    LOGINFO("%s:   Filter26: C1FLTCON6 = 0x%02X ; C1FLTOBJ26 = 0x%08X ; C1MASK26 = 0x%08X", EXTx, FSD.Bytes[26], FSD.Uint32[60], FSD.Uint32[61]);
    LOGINFO("%s:   Filter27: C1FLTCON6 = 0x%02X ; C1FLTOBJ27 = 0x%08X ; C1MASK27 = 0x%08X", EXTx, FSD.Bytes[27], FSD.Uint32[62], FSD.Uint32[63]);
    LOGINFO("%s:   Filter28: C1FLTCON7 = 0x%02X ; C1FLTOBJ28 = 0x%08X ; C1MASK28 = 0x%08X", EXTx, FSD.Bytes[28], FSD.Uint32[64], FSD.Uint32[65]);
    LOGINFO("%s:   Filter29: C1FLTCON7 = 0x%02X ; C1FLTOBJ29 = 0x%08X ; C1MASK29 = 0x%08X", EXTx, FSD.Bytes[29], FSD.Uint32[66], FSD.Uint32[67]);
    LOGINFO("%s:   Filter30: C1FLTCON7 = 0x%02X ; C1FLTOBJ30 = 0x%08X ; C1MASK30 = 0x%08X", EXTx, FSD.Bytes[30], FSD.Uint32[68], FSD.Uint32[69]);
    LOGINFO("%s:   Filter31: C1FLTCON7 = 0x%02X ; C1FLTOBJ31 = 0x%08X ; C1MASK31 = 0x%08X", EXTx, FSD.Bytes[31], FSD.Uint32[70], FSD.Uint32[71]);
  }
  else ShowDeviceError(pComp, Error);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Show the current error of the device
//=============================================================================
void ShowDeviceError(MCP251XFD *pComp, eERRORRESULT error)
{
  char* pStr = NULL;
/*  switch (error)
  {
#   define X(a, b, c) case a: pStr = (char*)c; break;
    ERRORS_TABLE
#   undef X
  }*/
  pStr = (char*)ERR_ErrorStrings[error];

  if (pStr != NULL)
       LOGERROR("%s: Device error: %s", EXTx, pStr);
  else LOGERROR("%s: Device error: Unknown error (%u)", EXTx, (unsigned int)error);
}





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond