/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * System-wide definitions for an OpTiMSoC system instance.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

// Number of virtual channels
`define VCHANNELS 3

// Assign virtual channels to services
`define VCHANNEL_LSU_REQ   0
`define VCHANNEL_LSU_RESP  1
`define VCHANNEL_DMA_REQ  'bx
`define VCHANNEL_DMA_RESP 'bx
`define VCHANNEL_MPSIMPLE 2

`define OPTIMSOC_XDIM 1
`define OPTIMSOC_YDIM 1

`define OPTIMSOC_MEMORYID 1

`define OPTIMSOC_CLOCK 24000000 // 24 MHz

`define OPTIMSOC_DEBUG_NO_CLOCK_GATING
`define OPTIMSOC_DEBUG_ENABLE_ITM
`define OPTIMSOC_DEBUG_ENABLE_STM
//`define OPTIMSOC_DEBUG_ENABLE_NRM
`define OPTIMSOC_DEBUG_ENABLE_NCM

`define OPTIMSOC_CLOCKDOMAINS
`define OPTIMSOC_CDC_DYNAMIC
`define OPTIMSOC_CDC_DYN_DEFAULT 0
//`define OPTIMSOC_UART_LCD_ENABLE 0