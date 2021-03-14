/******************************************************************************
 * @file    ErrorsDef.h
 * @author  Fabien MAILLY
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   Errors definitions
 *
 * These errors definitions are compatibles with all the libraries realized by
 * the author
 *****************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020 Fabien MAILLY
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
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
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
/*  // Device mode                                                                              */\
    X(ERR__NOT_IN_SLEEP_MODE   , =  90, "Operation impossible in sleep mode"                    ) \
    X(ERR__ALREADY_IN_SLEEP    ,      , "Already in sleep mode"                                 ) \
    X(ERR__NOT_CONFIG_MODE     ,      , "Operation impossible in configuration mode"            ) \
    X(ERR__NEED_CONFIG_MODE    ,      , "Device not in configuration mode"                      ) \
                                                                                                  \
/*  //--- Devices errors ---                                                                    */\
/*  // MCP251XFD errors                                                                         */\
    X(ERR__RAM_TEST_FAIL       , = 100, "RAM test fail"                                         ) \
    X(ERR__BITTIME_ERROR       ,      , "Can't calculate a good Bit Time"                       ) \
    X(ERR__TOO_MANY_TEF        ,      , "Too many TEF to configure"                             ) \
    X(ERR__TOO_MANY_TXQ        ,      , "Too many TXQ to configure"                             ) \
    X(ERR__TOO_MANY_FIFO       ,      , "Too many FIFO to configure"                            ) \
    X(ERR__SID11_NOT_AVAILABLE ,      , "SID11 not available in CAN2.0 mode"                    ) \
    X(ERR__FILTER_CONSISTENCY  ,      , "Filter inconsistency between Mask and filter"          ) \
    X(ERR__FILTER_TOO_LARGE    ,      , "Filter too large between filter and config"            ) \
    X(ERR__BYTE_COUNT_MODULO_4 ,      , "Byte count should be modulo 4"                         ) \
                                                                                                  \
/*  //--- Interfaces errors ---                                                                 */\
/*  // SPI errors                                                                               */\
    X(ERR__SPI_PARAMETER_ERROR , = 200, "SPI parameter error"                                   ) \
    X(ERR__SPI_COMM_ERROR      ,      , "SPI communication error"                               ) \
    X(ERR__SPI_CONFIG_ERROR    ,      , "SPI configuration error"                               ) \
    X(ERR__SPI_TIMEOUT         ,      , "SPI communication timeout"                             ) \
    X(ERR__SPI_FREQUENCY_ERROR ,      , "SPI frequency error"                                   ) \
/*  // I2C errors                                                                               */\
    X(ERR__I2C_NACK            , = 210, "Received a I2C not acknowledge"                        ) \
    X(ERR__I2C_NACK_DATA       ,      , "Received a I2C not acknowledge while transferring data") \
    X(ERR__I2C_PARAMETER_ERROR ,      , "I2C parameter error"                                   ) \
    X(ERR__I2C_COMM_ERROR      ,      , "I2C communication error"                               ) \
    X(ERR__I2C_CONFIG_ERROR    ,      , "I2C configuration error"                               ) \
    X(ERR__I2C_TIMEOUT         ,      , "I2C communication timeout"                             ) \
    X(ERR__I2C_DEVICE_NOT_READY,      , "I2C device not ready"                                  ) \
    X(ERR__I2C_INVALID_ADDRESS ,      , "I2C invalid address"                                   ) \
    X(ERR__I2C_INVALID_COMMAND ,      , "I2C invalid command"                                   ) \
    X(ERR__I2C_FREQUENCY_ERROR ,      , "I2C frequency error"                                   ) \
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
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* ERRORSDEF_H_ */