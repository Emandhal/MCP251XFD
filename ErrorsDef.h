/*!****************************************************************************
 * @file    ErrorsDef.h
 * @author  Fabien MAILLY
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   Errors definitions
 *
 * @details These errors definitions are compatibles with all the libraries
 * realized by the author
 *****************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020-2022 Fabien MAILLY
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
#ifndef ERRORSDEF_H_
#define ERRORSDEF_H_
//=============================================================================

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------





// WARNING! Here after use the X-Macros. See on Internet how it works before modifying something
#define ERRORS_TABLE                                                                              \
/*  // --- Success ---                                                                          */\
    X(ERR_OK                   , =   0, "Succeeded"                                             ) \
/*  // --- General errors ---                                                                   */\
    X(ERR__NO_DEVICE_DETECTED  ,      , "No device detected"                                    ) \
    X(ERR__OUT_OF_RANGE        ,      , "Value out of range"                                    ) \
    X(ERR__UNKNOWN_ELEMENT     ,      , "Unknown element (type or value)"                       ) \
    X(ERR__CONFIGURATION       ,      , "Configuration error"                                   ) \
    X(ERR__NOT_SUPPORTED       ,      , "Not supported"                                         ) \
    X(ERR__OUT_OF_MEMORY       ,      , "Out of memory"                                         ) \
    X(ERR__NOT_AVAILABLE       ,      , "Function not available"                                ) \
    X(ERR__DEVICE_TIMEOUT      ,      , "Device timeout"                                        ) \
    X(ERR__PARAMETER_ERROR     ,      , "Parameter error"                                       ) \
    X(ERR__NOT_READY           ,      , "Device not ready"                                      ) \
    X(ERR__NO_DATA_AVAILABLE   ,      , "No data available"                                     ) \
    X(ERR__FREQUENCY_ERROR     ,      , "Frequency error"                                       ) \
    X(ERR__CRC_ERROR           ,      , "CRC mismatch error"                                    ) \
    X(ERR__BAUDRATE_ERROR      ,      , "Baudrate error"                                        ) \
    X(ERR__UNKNOWN             ,      , "Unknown error"                                         ) \
    X(ERR__NO_AUTHORIZATION    ,      , "No authorization to execute the command received"      ) \
    X(ERR__NULL_POINTER        ,      , "Null pointer"                                          ) \
    X(ERR__VERSION             ,      , "Version error"                                         ) \
    X(ERR__NOT_IMPLEMENTED     ,      , "Not implemented"                                       ) \
    X(ERR__DATA_NOT_INITIALIZED,      , "Data not initialized"                                  ) \
    X(ERR__PERIPHERAL_NOT_VALID,      , "Peripheral not valid"                                  ) \
    X(ERR__UNKNOWN_COMMAND     ,      , "Unknown command"                                       ) \
    X(ERR__BAD_ADDRESS         ,      , "Bad start address or end address"                      ) \
    X(ERR__BAD_DESTINATAIRE    ,      , "Bad destinataire of the frame"                         ) \
    X(ERR__BAD_DATA_SIZE       ,      , "Bad data size"                                         ) \
    X(ERR__BAD_FRAME_TYPE      ,      , "Bad frame type"                                        ) \
    X(ERR__NO_REPONSE          ,      , "No response"                                           ) \
    X(ERR__TIMEOUT             ,      , "General timeout"                                       ) \
    X(ERR__BUFFER_FULL         ,      , "Buffer full"                                           ) \
    X(ERR__UNKNOWN_DEVICE      ,      , "Unknown device error"                                  ) \
    X(ERR__NULL_BUFFER         ,      , "Null buffer parameter"                                 ) \
    X(ERR__TOO_MANY_BAD        ,      , "Too many bad things"                                   ) \
    X(ERR__TWO_BAD_SIDE_BY_SIDE,      , "Two bad things side by side"                           ) \
    X(ERR__TRANSMIT_ERROR      ,      , "Transmit error"                                        ) \
    X(ERR__RECEIVE_ERROR       ,      , "Receive error"                                         ) \
    X(ERR__UNKNOWN_CHANNEL     ,      , "Unknown channel error"                                 ) \
    X(ERR__BAD_DATA            ,      , "Bad data"                                              ) \
    X(ERR__BUSY                ,      , "Busy"                                                  ) \
    X(ERR__EMPTY_DATA          ,      , "Empty data"                                            ) \
    X(ERR__NOT_FOUND           ,      , "Not found"                                             ) \
    X(ERR__INVALID_HANDLE      ,      , "Invalid handle"                                        ) \
    X(ERR__ADDRESS_ALIGNMENT   ,      , "Address alignment error"                               ) \
    X(ERR__READ_ERROR          ,      , "Read error"                                            ) \
    X(ERR__WRITE_ERROR         ,      , "Write error"                                           ) \
    X(ERR__DATA_MODULO         ,      , "Wrong data modulo for the operation"                   ) \
    X(ERR__BUFFER_OVERRIDE     ,      , "Buffer data override"                                  ) \
    X(ERR__PARSE_ERROR         ,      , "Error while parsing data"                              ) \
    X(ERR__OLD_DATA            ,      , "Old data"                                              ) \
    X(ERR__CANCELED            ,      , "Canceled"                                              ) \
    X(ERR__NO_SYNC             ,      , "No synchronization"                                    ) \
    X(ERR__NOT_INITIALIZED     ,      , "Not initialized"                                       ) \
    X(ERR__BAD_ENDIANNESS      ,      , "Bad endianness"                                        ) \
    X(ERR__INVALID_DATA        ,      , "Invalid data"                                          ) \
/*  // Device mode                                                                              */\
    X(ERR__NOT_IN_SLEEP_MODE   , =  90, "Operation impossible in sleep mode"                    ) \
    X(ERR__ALREADY_IN_SLEEP    ,      , "Already in sleep mode"                                 ) \
    X(ERR__NOT_CONFIG_MODE     ,      , "Operation impossible in configuration mode"            ) \
    X(ERR__NEED_CONFIG_MODE    ,      , "Device not in configuration mode"                      ) \
    X(ERR__CANNOT_GO_IDLE_STATE,      , "Cannot go in idle state"                               ) \
                                                                                                  \
/*  //--- Devices errors ---                                                                    */\
/*  // CAN errors                                                                               */\
    X(ERR__RAM_TEST_FAIL       , = 100, "RAM test fail"                                         ) \
    X(ERR__BITTIME_ERROR       ,      , "Can't calculate a good Bit Time"                       ) \
    X(ERR__TOO_MANY_TEF        ,      , "Too many TEF to configure"                             ) \
    X(ERR__TOO_MANY_TXQ        ,      , "Too many TXQ to configure"                             ) \
    X(ERR__TOO_MANY_FIFO       ,      , "Too many FIFO to configure"                            ) \
    X(ERR__TOO_MANY_BUFFER     ,      , "Too many Buffer to configure"                          ) \
    X(ERR__SID11_NOT_AVAILABLE ,      , "SID11 not available in CAN2.0 mode"                    ) \
    X(ERR__FILTER_CONSISTENCY  ,      , "Filter inconsistency between Mask and filter"          ) \
    X(ERR__FILTER_TOO_LARGE    ,      , "Filter too large between filter and config"            ) \
    X(ERR__BYTE_COUNT_MODULO_4 ,      , "Byte count should be modulo 4"                         ) \
/*  // SD card errors                                                                           */\
    X(ERR__NO_CARD             ,      , "No card present"                                       ) \
    X(ERR__UNUSABLE_CARD       ,      , "Unusable SD card"                                      ) \
    X(ERR__CARD_ERROR          ,      , "SD card error"                                         ) \
    X(ERR__CARD_COMMAND_ERROR  ,      , "SD card command error"                                 ) \
    X(ERR__CARD_WRITE_PROTECTED,      , "SD card is write protected"                            ) \
    X(ERR__CARD_ECC_FAIL       ,      , "SD card ECC fail"                                      ) \
    X(ERR__CARD_OUT_OF_RANGE   ,      , "SD card out of range argument"                         ) \
    X(ERR__CARD_STUCK_BUSY     ,      , "SD card busy for too long time"                        ) \
                                                                                                  \
/*  //--- Interfaces errors ---                                                                 */\
/*  // SPI errors                                                                               */\
    X(ERR__SPI_PARAMETER_ERROR , = 200, "SPI parameter error"                                   ) \
    X(ERR__SPI_COMM_ERROR      ,      , "SPI communication error"                               ) \
    X(ERR__SPI_CONFIG_ERROR    ,      , "SPI configuration error"                               ) \
    X(ERR__SPI_TIMEOUT         ,      , "SPI communication timeout"                             ) \
    X(ERR__SPI_INVALID_DATA    ,      , "SPI invalid data"                                      ) \
    X(ERR__SPI_FREQUENCY_ERROR ,      , "SPI frequency error"                                   ) \
    X(ERR__SPI_OVERFLOW_ERROR  ,      , "SPI overflow error"                                    ) \
    X(ERR__SPI_UNDERFLOW_ERROR ,      , "SPI underflow error"                                   ) \
    X(ERR__SPI_BUSY            ,      , "SPI busy"                                              ) \
    X(ERR__SPI_OTHER_BUSY      ,      , "SPI busy by other transfer"                            ) \
/*  // I2C errors                                                                               */\
    X(ERR__I2C_NACK            , = 210, "Received a I2C not acknowledge"                        ) \
    X(ERR__I2C_NACK_ADDR       ,      , "Received a I2C not acknowledge while transferring addr") \
    X(ERR__I2C_NACK_DATA       ,      , "Received a I2C not acknowledge while transferring data") \
    X(ERR__I2C_PARAMETER_ERROR ,      , "I2C parameter error"                                   ) \
    X(ERR__I2C_COMM_ERROR      ,      , "I2C communication error"                               ) \
    X(ERR__I2C_CONFIG_ERROR    ,      , "I2C configuration error"                               ) \
    X(ERR__I2C_TIMEOUT         ,      , "I2C communication timeout"                             ) \
    X(ERR__I2C_DEVICE_NOT_READY,      , "I2C device not ready"                                  ) \
    X(ERR__I2C_INVALID_ADDRESS ,      , "I2C invalid address"                                   ) \
    X(ERR__I2C_INVALID_COMMAND ,      , "I2C invalid command"                                   ) \
    X(ERR__I2C_FREQUENCY_ERROR ,      , "I2C frequency error"                                   ) \
    X(ERR__I2C_OVERFLOW_ERROR  ,      , "I2C overflow error"                                    ) \
    X(ERR__I2C_UNDERFLOW_ERROR ,      , "I2C underflow error"                                   ) \
    X(ERR__I2C_BUSY            ,      , "I2C busy"                                              ) \
    X(ERR__I2C_OTHER_BUSY      ,      , "I2C busy by other transfer"                            ) \
/*  // DMA errors                                                                               */\
    X(ERR__DMA_NOT_CONFIGURED  , = 225, "DMA not configured"                                    ) \
    X(ERR__DMA_ERROR           ,      , "DMA error"                                             ) \
    X(ERR__DMA_OVERFLOW_ERROR  ,      , "DMA overflow error"                                    ) \
    X(ERR__DMA_UNDERFLOW_ERROR ,      , "DMA underflow error"                                   ) \
    X(ERR__DMA_WRITE_BUS_ERROR ,      , "DMA write bus error"                                   ) \
    X(ERR__DMA_READ_BUS_ERROR  ,      , "DMA read bus error"                                    ) \
/*  // Test error                                                                               */\
    X(ERR__TEST_ERROR          , = 255, "Test error"                                            )





//! Function's return error enumerator
typedef enum
{
#ifdef USE_REDUCED_ERRORS_VALUE
#  define X(eName, val, str) eName,
#else
#  define X(eName, val, str) eName val,
#endif
  ERRORS_TABLE
#undef X
  ERR__ERRORS_MAX, // Keep last
} eERRORRESULT;



#ifdef USE_ERRORS_STRING
//! Errors string table
static const char* const ERR_ErrorStrings[] =
{
#  define X(a, b, c) [a]=c,
  ERRORS_TABLE
#  undef X
};
#endif





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* ERRORSDEF_H_ */