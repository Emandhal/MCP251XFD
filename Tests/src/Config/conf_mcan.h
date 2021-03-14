/**
 * \file
 *
 * \brief SAM Control Area Network Driver Configuration Header
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#ifndef CONF_MCAN_H_INCLUDED
#define CONF_MCAN_H_INCLUDED

/*
 * Below is the message RAM setting, it will be stored in the system RAM.
 * Please adjust the message size according to your application.
 */
/** Range: 1..64 */ 
#define CONF_MCAN0_RX_FIFO_0_NUM                0     
/** Range: 1..64 */        
#define CONF_MCAN0_RX_FIFO_1_NUM                0      
/** Range: 1..64 */      
#define CONF_MCAN0_RX_BUFFER_NUM                0
/** Range: 1..16 */
#define CONF_MCAN0_TX_BUFFER_NUM                0   
/** Range: 1..16 */        
#define CONF_MCAN0_TX_FIFO_QUEUE_NUM            0     
/** Range: 1..32 */       
#define CONF_MCAN0_TX_EVENT_FIFO                0             
/** Range: 1..128 */
#define CONF_MCAN0_RX_STANDARD_ID_FILTER_NUM    0    
/** Range: 1..64 */
#define CONF_MCAN0_RX_EXTENDED_ID_FILTER_NUM    0

    
/** Range: 1..64 */
#define CONF_MCAN1_RX_FIFO_0_NUM               64             
/** Range: 1..64 */
#define CONF_MCAN1_RX_FIFO_1_NUM                0  
/** Range: 1..64 */          
#define CONF_MCAN1_RX_BUFFER_NUM                0      
/** Range: 1..16 */     
#define CONF_MCAN1_TX_BUFFER_NUM                0 
/** Range: 1..16 */     
#define CONF_MCAN1_TX_FIFO_QUEUE_NUM            8     
/** Range: 1..32 */        
#define CONF_MCAN1_TX_EVENT_FIFO               32             
/** Range: 1..128 */
#define CONF_MCAN1_RX_STANDARD_ID_FILTER_NUM    1    
/** Range: 1..64 */
#define CONF_MCAN1_RX_EXTENDED_ID_FILTER_NUM    1    

/** The value should be 8/12/16/20/24/32/48/64. */
#define CONF_MCAN_ELEMENT_DATA_SIZE            64

/**
 * The setting of the nominal bit rate is based on the PCK5 which is 30MHz which you can
 * change in the conf_clock.h. Below is the default configuration. The
 * time quanta is 30MHz / (0+1) = 30MHz. And each bit is (1 + NTSEG1 + 1 + NTSEG2 + 1) = 30 time
 * quanta which means the bit rate is 30MHz/30=1Mbps.
 */
/** Nominal bit Baud Rate Prescaler */
#define CONF_MCAN_NBTP_NBRP_VALUE    0
/** Nominal bit (Re)Synchronization Jump Width */
#define CONF_MCAN_NBTP_NSJW_VALUE    5
/** Nominal bit Time segment before sample point */
#define CONF_MCAN_NBTP_NTSEG1_VALUE  22
/** Nominal bit Time segment after sample point */
#define CONF_MCAN_NBTP_NTSEG2_VALUE  5

/*
 * The setting of the data bit rate is based on the PCK5 which is 30MHz which you can
 * change. Below is the default configuration. The
 * time quanta is 30MHz / (0+1) = 30MHz. And each bit is (1 + FTSEG1 + 1 + FTSEG2 + 1) = 6 time (> 4 min)
 * quanta which means the bit rate is 30MHz/6=5Mbps.
 */
/** Data bit Baud Rate Prescaler */
#define CONF_MCAN_FBTP_FBRP_VALUE    0
/** Data bit (Re)Synchronization Jump Width */
#define CONF_MCAN_FBTP_FSJW_VALUE    0
/** Data bit Time segment before sample point */
#define CONF_MCAN_FBTP_FTSEG1_VALUE  3
/** Data bit Time segment after sample point */
#define CONF_MCAN_FBTP_FTSEG2_VALUE  0









///*
 //* Below is the message RAM setting, it will be stored in the system RAM.
 //* Please adjust the message size according to your application.
 //*/
///** Range: 1..64 */ 
//#define CONF_MCAN0_RX_FIFO_0_NUM         16     
///** Range: 1..64 */        
//#define CONF_MCAN0_RX_FIFO_1_NUM         16      
///** Range: 1..64 */      
//#define CONF_MCAN0_RX_BUFFER_NUM         16
///** Range: 1..16 */
//#define CONF_MCAN0_TX_BUFFER_NUM         4   
///** Range: 1..16 */        
//#define CONF_MCAN0_TX_FIFO_QUEUE_NUM     4     
///** Range: 1..32 */       
//#define CONF_MCAN0_TX_EVENT_FIFO         8             
///** Range: 1..128 */
//#define CONF_MCAN0_RX_STANDARD_ID_FILTER_NUM     32    
///** Range: 1..64 */
//#define CONF_MCAN0_RX_EXTENDED_ID_FILTER_NUM     16    
///** Range: 1..64 */
//#define CONF_MCAN1_RX_FIFO_0_NUM         16             
///** Range: 1..64 */
//#define CONF_MCAN1_RX_FIFO_1_NUM         16  
///** Range: 1..64 */          
//#define CONF_MCAN1_RX_BUFFER_NUM         16      
///** Range: 1..16 */     
//#define CONF_MCAN1_TX_BUFFER_NUM         0 
///** Range: 1..16 */     
//#define CONF_MCAN1_TX_FIFO_QUEUE_NUM     8     
///** Range: 1..32 */        
//#define CONF_MCAN1_TX_EVENT_FIFO         8             
///** Range: 1..128 */
//#define CONF_MCAN1_RX_STANDARD_ID_FILTER_NUM     32    
///** Range: 1..64 */
//#define CONF_MCAN1_RX_EXTENDED_ID_FILTER_NUM     16    
//
///** The value should be 8/12/16/20/24/32/48/64. */
//#define CONF_MCAN_ELEMENT_DATA_SIZE         8
//
///**
 //* The setting of the nominal bit rate is based on the PCK5 which is 30M which you can
 //* change in the conf_clock.h. Below is the default configuration. The
 //* time quanta is 30MHz / (2+1) =  10MHz. And each bit is (1+5+3) = 10 time
 //* quanta which means the bit rate is 10MHz/10=1000KHz.
 //*/
///** Nominal bit Baud Rate Prescaler */
//#define CONF_MCAN_NBTP_NBRP_VALUE    2
///** Nominal bit (Re)Synchronization Jump Width */
//#define CONF_MCAN_NBTP_NSJW_VALUE    1
///** Nominal bit Time segment before sample point */
//#define CONF_MCAN_NBTP_NTSEG1_VALUE  5
///** Nominal bit Time segment after sample point */
//#define CONF_MCAN_NBTP_NTSEG2_VALUE  2
//
///*
 //* The setting of the data bit rate is based on the GCLK_MCAN is 48M which you can
 //* change in the conf_clock.h. Below is the default configuration. The
 //* time quanta is 48MHz / (2+1) =  16MHz. And each bit is (3+10+3) = 16 time
 //* quanta which means the bit rate is 16MHz/16=1000KHz.
 //*/
///** Data bit Baud Rate Prescaler */
//#define CONF_MCAN_FBTP_FBRP_VALUE    2
///** Data bit (Re)Synchronization Jump Width */
//#define CONF_MCAN_FBTP_FSJW_VALUE    3
///** Data bit Time segment before sample point */
//#define CONF_MCAN_FBTP_FTSEG1_VALUE  10
///** Data bit Time segment after sample point */
//#define CONF_MCAN_FBTP_FTSEG2_VALUE  3

#endif
