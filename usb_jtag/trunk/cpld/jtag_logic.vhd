-------------------------------------------------------------------------------
-- Serial/Parallel converter, interfacing JTAG chain with FTDI FT245BM
-------------------------------------------------------------------------------
-- Copyright (C) 2005,2006 Kolja Waschk, ixo.de
-------------------------------------------------------------------------------
-- This code is part of usbjtag. usbjtag is free software; you can redistribute
-- it and/or modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 2 of the License,
-- or (at your option) any later version. usbjtag is distributed in the hope
-- that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
-- warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.  You should have received a
-- copy of the GNU General Public License along with this program in the file
-- COPYING; if not, write to the Free Software Foundation, Inc., 51 Franklin
-- St, Fifth Floor, Boston, MA  02110-1301  USA
-------------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_unsigned.all;

ENTITY jtag_logic IS
	PORT
	(
		CLK : IN STD_LOGIC;        -- external 24/25 MHz oscillator
		nRXF : IN STD_LOGIC;       -- FT245BM nRXF
		nTXE : IN STD_LOGIC;       -- FT245BM nTXE
		B_TDO : IN STD_LOGIC;      -- JTAG input: TDO of last device in chain
		nRD : OUT STD_LOGIC;       -- FT245BM nRD
		WR : OUT STD_LOGIC;        -- FT245BM WR
		B_TCK : BUFFER STD_LOGIC;  -- JTAG output: TCK to chain
		B_TMS : BUFFER STD_LOGIC;  -- JTAG output: TMS to chain
		B_TDI : BUFFER STD_LOGIC;  -- JTAG output: to TDI of first device in chain
		D : INOUT STD_LOGIC_VECTOR(7 downto 0) -- FT245BM D[7..0]
	);
END jtag_logic;

ARCHITECTURE spec OF jtag_logic IS
	TYPE states IS
	(
		wait_for_nRXF_low,
		set_nRD_low,
		keep_nRD_low,
		latch_data_from_host,
		set_nRD_high,
		bits_set_pins_from_data,
		bytes_set_bitcount,
		bytes_get_tdo_set_tdi,
		bytes_clock_high_and_shift,
		bytes_keep_clock_high,
		bytes_clock_finish,
		wait_for_nTXE_low,
		set_WR_high,
		output_enable,
		set_WR_low,
		output_disable
	);
	
	SIGNAL carry: STD_LOGIC;
	SIGNAL do_output: STD_LOGIC;
	SIGNAL ioshifter: STD_LOGIC_VECTOR(7 DOWNTO 0);
	SIGNAL bitcount: STD_LOGIC_VECTOR(8 DOWNTO 0);
	SIGNAL state, next_state: states;
	
BEGIN
	-- sm: PROCESS(nRXF, nTXE, state, D, bitcount, ioshifter, do_output)
	sm: PROCESS(nRXF, nTXE, state, bitcount, ioshifter, do_output)

	BEGIN
		CASE state IS
		
			-- ============================ INPUT
		
			WHEN wait_for_nRXF_low =>
				IF nRXF='0' THEN
					next_state <= set_nRD_low;
				ELSE
					next_state <= wait_for_nRXF_low;
				END IF;
				
			WHEN set_nRD_low =>
				next_state <= keep_nRD_low;
			
			WHEN keep_nRD_low => 
				next_state <= latch_data_from_host;
				
			WHEN latch_data_from_host =>
				next_state <= set_nRD_high;
			
			WHEN set_nRD_high =>
				IF NOT (bitcount(8 DOWNTO 3) = "000000") THEN
					next_state <= bytes_get_tdo_set_tdi;
				ELSIF ioshifter(7) = '1' THEN
					next_state <= bytes_set_bitcount;
				ELSE
					next_state <= bits_set_pins_from_data;
				END IF;
			
			WHEN bytes_set_bitcount =>
				next_state <= wait_for_nRXF_low;
			
			-- ============================ BIT BANGING
				
			WHEN bits_set_pins_from_data =>
				IF ioshifter(6) = '0' THEN
					next_state <= wait_for_nRXF_low; -- read next byte from host
				ELSE
					next_state <= wait_for_nTXE_low; -- output byte to host
				END IF;
				
			-- ============================ BYTE OUTPUT (SHIFT OUT 8 BITS)
			
			WHEN bytes_get_tdo_set_tdi =>
				next_state <= bytes_clock_high_and_shift;
			
			WHEN bytes_clock_high_and_shift =>
				next_state <= bytes_keep_clock_high;
				
			WHEN bytes_keep_clock_high =>
				next_state <= bytes_clock_finish;
				
			WHEN bytes_clock_finish =>
				IF NOT (bitcount(2 DOWNTO 0) = "111") THEN
					next_state <= bytes_get_tdo_set_tdi; -- clock next bit
				ELSIF do_output = '1' THEN
					next_state <= wait_for_nTXE_low; -- output byte to host
				ELSE
					next_state <= wait_for_nRXF_low; -- read next byte from host
				END IF;
			
			-- ============================ OUTPUT BYTE TO HOST
			
			WHEN wait_for_nTXE_low =>
				IF nTXE = '0' THEN
					next_state <= set_WR_high;
				ELSE
					next_state <= wait_for_nTXE_low;
				END IF;
				
			WHEN set_WR_high =>
				next_state <= output_enable;
				
			WHEN output_enable =>
				next_state <= set_WR_low;
				
			WHEN set_WR_low =>
				next_state <= output_disable;
			
			WHEN output_disable =>
				next_state <= wait_for_nRXF_low; -- read next byte from host
				
			WHEN OTHERS => 
				next_state <= wait_for_nRXF_low;
				
		END CASE;
	END PROCESS sm;

	out_sm: PROCESS(CLK, state, ioshifter, B_TDO, bitcount, carry)

	BEGIN
		IF CLK = '1' AND CLK'event THEN
			
			IF state = set_nRD_low OR state = keep_nRD_low OR state = latch_data_from_host THEN
				nRD <= '0';
			ELSE
				nRD <= '1';
			END IF;
			
			IF state = latch_data_from_host THEN
				ioshifter(7 DOWNTO 0) <= D;
			END IF;
			
			IF state = set_WR_high OR state = output_enable THEN
				WR <= '1';
			ELSE
				WR <= '0';
			END IF;
			
			IF state = output_enable OR state = set_WR_low THEN
				D <= ioshifter(7 DOWNTO 0);
			ELSE
				D <= "ZZZZZZZZ";	
			END IF;
			
			IF state = bits_set_pins_from_data THEN
				B_TDI <= ioshifter(4);
				B_TMS <= ioshifter(1);
				B_TCK <= ioshifter(0);
				ioshifter <= "0000001" & B_TDO;
			END IF;
			
			IF state = bytes_set_bitcount THEN
				bitcount <= ioshifter(5 DOWNTO 0) & "111";
				do_output <= ioshifter(6);
			END IF;
			
			IF state = bytes_get_tdo_set_tdi THEN
				carry <= B_TDO;
				B_TDI <= ioshifter(0);
				bitcount <= bitcount - 1;
			END IF;
			
			IF state = bytes_clock_high_and_shift OR state = bytes_keep_clock_high THEN
				B_TCK <= '1';
			END IF;
			
			IF state = bytes_clock_high_and_shift THEN
				ioshifter <= carry & ioshifter(7 DOWNTO 1);
			END IF;
			
			IF state = bytes_clock_finish THEN
				B_TCK <= '0';
			END IF;
		
			state <= next_state;
			
		END IF;
	END PROCESS out_sm;
	
END spec;
