LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
-- USE ieee.std_logic_arith.ALL;
-- USE ieee.std_logic_unsigned.ALL;

-- ---------------------------------------------------------

ENTITY eb2 IS

    PORT
    (   
        -- Clock and Reset
        clk_brd        : IN STD_LOGIC;
        -- User Header A
        ha             : INOUT STD_LOGIC_VECTOR(19 DOWNTO 2);
        -- User Header B
        hb             : INOUT STD_LOGIC_VECTOR(19 DOWNTO 2)
     );

END eb2;

-- ---------------------------------------------------------

ARCHITECTURE behaviour OF eb2 IS

SIGNAL half_clk_brd : STD_LOGIC;
SIGNAL quarta_clk_brd : STD_LOGIC;

SIGNAL tms : STD_LOGIC;
SIGNAL tdo : STD_LOGIC;
SIGNAL tdi : STD_LOGIC;
SIGNAL tck : STD_LOGIC;
SIGNAL nce : STD_LOGIC;
SIGNAL ncs : STD_LOGIC;
SIGNAL nstatus : STD_LOGIC;

COMPONENT jtag_logic

	PORT
	(
		CLK : IN STD_LOGIC;        -- external 24/25 MHz oscillator
		nRXF : IN STD_LOGIC;       -- FT245BM nRXF
		nTXE : IN STD_LOGIC;       -- FT245BM nTXE
		B_TDO  : IN STD_LOGIC;     -- JTAG input: TDO, AS/PS input: CONF_DONE
		B_ASDO : IN STD_LOGIC;     -- AS input: DATAOUT, PS input: nSTATUS
		B_TCK  : BUFFER STD_LOGIC; -- JTAG output: TCK to chain, AS/PS DCLK
		B_TMS  : BUFFER STD_LOGIC; -- JTAG output: TMS to chain, AS/PS nCONFIG
		B_NCE  : BUFFER STD_LOGIC; -- AS output: nCE
		B_NCS  : BUFFER STD_LOGIC; -- AS output: nCS
		B_TDI  : BUFFER STD_LOGIC; -- JTAG output: TDI to chain, AS: ASDI, PS: DATA0
		B_OE   : BUFFER STD_LOGIC; -- LED output/output driver enable 
		nRD : OUT STD_LOGIC;       -- FT245BM nRD
		WR : OUT STD_LOGIC;        -- FT245BM WR
		D : INOUT STD_LOGIC_VECTOR(7 downto 0) -- FT245BM D[7..0]
	);
END COMPONENT;

BEGIN

    divide_clk_brd: PROCESS(clk_brd)
    BEGIN
		IF clk_brd = '1' AND clk_brd'event THEN
		    half_clk_brd <= not half_clk_brd;
		END IF;
	END PROCESS divide_clk_brd;

    divide_half_clk_brd: PROCESS(half_clk_brd)
    BEGIN
		IF half_clk_brd = '1' AND half_clk_brd'event THEN
		    quarta_clk_brd <= not quarta_clk_brd;
		END IF;
	END PROCESS divide_half_clk_brd;

    -- JTAG interface to target

    tdo <= hb(3);
    hb(4) <= tck;
    hb(5) <= tms;
    hb(6) <= nce;
    nstatus <= hb(7);
    hb(8) <= ncs;
    hb(9) <= tdi;

    -- JTAG interface to real USB-Blaster on same header

--    tck <= hb(11);
--    hb(13) <= tdo;
--    tms <= hb(15);
--    nce <= hb(16);
--    hb(17) <= nstatus;
--    ncs <= hb(18);
--    tdi <= hb(19);

   -- JTAG interface to FT245 via jtag_logic

   blaster: jtag_logic

   PORT MAP
   (
       -- Connection to UM245R:
       nRXF => ha(15),            -- FT245 nRXF / FT232 CB0
       nTXE => ha(13),            -- FT245 nTXE / FT232 CB1
       nRD  => ha(3),             -- FT245 nRD  / FT232 CB2
       WR   => ha(6),             -- FT245 WR   / FT232 CB3
       D(0) => ha(18),            -- FT245 DB0  / FT232 TXD
       D(1) => ha(10),            -- FT245 DB1  / FT232 RXD
       D(2) => ha(14),            -- FT245 DB2  / FT232 nRTS
       D(3) => ha(7),             -- FT245 DB3  / FT232 nCTS
       D(4) => ha(16),            -- FT245 DB4  / FT232 nDTR
       D(5) => ha(4),             -- FT245 DB5  / FT232 nDSR
       D(6) => ha(2),             -- FT245 DB6  / FT232 nDCD
       D(7) => ha(8),             -- FT245 DB7  / FT232 nRI

       -- JTAG interface:
--       B_ASDO => '0',
--       B_TDO => '0',
       B_TDO => tdo,              -- JTAG input: TDO, AS/PS input: CONF_DONE
       B_ASDO => nstatus,         -- AS input: DATAOUT, PS input: nSTATUS
       B_TCK => tck,              -- JTAG output: TCK to chain, AS/PS DCLK
       B_TMS => tms,              -- JTAG output: TMS to chain, AS/PS nCONFIG
       B_NCE => nce,              -- AS output: nCE
       B_NCS => ncs,              -- AS output: nCS
       B_TDI => tdi,              -- JTAG output: TDI to chain, AS: ASDI, PS: DATA0
       -- B_OE => led_gn,            -- LED output/output driver enable 

       -- Subsystem clock (~ 24 MHz)
       CLK => half_clk_brd        -- external 24/25 MHz oscillator
       --CLK => quarta_clk_brd        -- external 24/25 MHz oscillator
	);

    -- Unassigned & Input pins

    hb(2)  <= 'Z';
    hb(3)  <= 'Z';
    hb(7)  <= 'Z';
    hb(10) <= 'Z';
    hb(11) <= 'Z';
    hb(12) <= 'Z';
    hb(13) <= 'Z';
    hb(14) <= 'Z';
    hb(15) <= 'Z';
    hb(16) <= 'Z';
    hb(17) <= 'Z';
    hb(18) <= 'Z';
    hb(19) <= 'Z';

    ha(5)  <= 'Z'; -- FT245/FT232 nPWE
    ha(9)  <= 'Z'; -- FT245/FT232 nPWE
    ha(11) <= 'Z'; -- n.c.
    ha(12) <= 'Z'; -- VIO
    ha(13) <= 'Z'; -- FT245 nTXE
    ha(15) <= 'Z'; -- FT245 nRXF
    ha(17) <= 'Z'; -- n.c.
    ha(19) <= 'Z'; -- n.c.

END behaviour;
