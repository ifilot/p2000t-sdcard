;-------------------------------------------------------------------------------
;                                                                       
;   Author: Ivo Filot <ivo@ivofilot.nl>                                 
;                                                                       
;   P2000T-SDCARD is free software:                                     
;   you can redistribute it and/or modify it under the terms of the     
;   GNU General Public License as published by the Free Software        
;   Foundation, either version 3 of the License, or (at your option)    
;   any later version.                                                  
;                                                                       
;   P2000T-SDCARD is distributed in the hope that it will be useful,    
;   but WITHOUT ANY WARRANTY; without even the implied warranty         
;   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.             
;   See the GNU General Public License for more details.                
;                                                                       
;   You should have received a copy of the GNU General Public License   
;   along with this program.  If not, see http://www.gnu.org/licenses/. 
;                                                                       
;-------------------------------------------------------------------------------

; signature, byte count, CRC16 checksum
;
; * signature needs to be 0x50 to indicate that this is a runnable PRG file
; * byte count corresponds to the number of bytes of the program
; * checksum is the CRC-16 checksum
; * the byte count and CRC16 checksum need to be set later
DB 0x50,0x00,0x00,0x00,0x00

; name of the program (8+3 bytes)
DB 'H','E','L','L','O','W','O','R','P','R','G'

; first address to call is thus $A210
jp __Start