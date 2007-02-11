;;; -*- asm -*-
;;;
;;;-----------------------------------------------------------------------------
;;; Product ID and name
;;;-----------------------------------------------------------------------------
;;; Copyright (C) 2007 Kolja Waschk, ixo.de
;;;-----------------------------------------------------------------------------
;;; This code is part of usbjtag. usbjtag is free software; you can redistribute
;;; it and/or modify it under the terms of the GNU General Public License as
;;; published by the Free Software Foundation; either version 2 of the License,
;;; or (at your option) any later version. usbjtag is distributed in the hope
;;; that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
;;; warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.  You should have received a
;;; copy of the GNU General Public License along with this program in the file
;;; COPYING; if not, write to the Free Software Foundation, Inc., 51 Franklin
;;; St, Fifth Floor, Boston, MA  02110-1301  USA
;;;-----------------------------------------------------------------------------

        VID              = 0x16C0         ; Vendor ID 0x16C0  
        PID              = 0x06AD         ; Product ID 0x06AD 
        VERSION          = 0x0004         ; Product Version (4 indicates *BM device) 
        USB_VER          = 0x0110         ; Support USB version 1.10 
        USB_ATTR         = 0x80           ; Bus powered, not self-powered, no remote wakeup
        FTD_ATTR         = 0x001C         ; Set USB version, use version string, enable suspend PD
        MAX_POWER        = 75             ; need 2*75 mA max

