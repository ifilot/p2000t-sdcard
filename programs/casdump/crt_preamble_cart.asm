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

; signature, byte count, checksum
DB 0x5E,0x00,0x00,0x00,0x00

; name of the cartridge (11 bytes)
;DB 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
DB 'C' ,'A' ,'S' ,'S' ,'E' ,'T' ,'T', 'E', 'D', 'M', 'P'

jp __Start