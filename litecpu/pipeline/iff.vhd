library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.consts.ALL;
use IEEE.NUMERIC_STD.ALL;


entity IFF is
    port (
		clk_i: in std_logic;
		rst_i: in std_logic;

		halt_i: in std_logic;
		
		active_o: out std_logic;

		advance_i: in std_logic;

		jb_pc_i: in mem_addr_t; -- jump/branch
		jb_en_i: in std_logic;

		pc_o: out mem_addr_t;
		
		pc_i: in mem_addr_t;
		pc_we_i: in std_logic;
		
		pcTEST: out std_logic
		
/*		reg_halt_i: in std_logic;
		reg_active_i: in std_logic;
		recover_i: in std_logic;
		intRdata_i: in dword*/
	);
end IFF;

architecture behave of IFF is
	signal pc: std_logic_vector(31 downto 0);
begin
	pc_o <= pc;

	process (clk_i)
 	begin
		if (rising_edge(clk_i)) then
			active_o <= '0';
 			if (rst_i = '1') then
				pc <= BOOT_PC;
				active_o <= '1';
				pcTEST <= '0';
         elsif (halt_i = '0') then
--				if (((advance_i = '1') and (reg_halt_i = '0')) or (reg_active_i = '1')) then
			
				if (advance_i = '1') then 
/*					step <= '1';
					jb_en <= jb_en_i;
					jb_pc <= jb_pc_i;
				elsif (step = '1') then*/
				
/*					if (recover_i = '1') then 
						pc <= intRdata_i;
					els*/
					if (pc_we_i = '1') then 
						pc <= pc_i;
					elsif (jb_en_i = '1') then	
						pc <= jb_pc_i;
					else
						pc <= std_logic_vector(unsigned(pc) + 4);
					end if;
					active_o <= '1';
				end if;
			end if;
			if (pc = x"00000000") then
				pcTEST <= '1';
			end if;
			-- else: pc should not change
		end if;
	end process;

end behave;
