library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

use IEEE.NUMERIC_STD.ALL;

library hal_asos_v4_00_a;
use hal_asos_v4_00_a.hal_asos_configs_pkg.all;
use hal_asos_v4_00_a.hal_kernel_pkg.all;
use hal_asos_v4_00_a.hal_asos_user_pkg.all;
use hal_asos_v4_00_a.hal_asos_utils_pkg.all;

library hw_fast_v1_00_a;
use hw_fast_v1_00_a.fast_pkg.RUN_BIT;
use hw_fast_v1_00_a.fast_pkg.THRESHOLD_OFFSET;
use hw_fast_v1_00_a.fast_pkg.CONTROL_OFFSET;
use hw_fast_v1_00_a.fast_pkg.array_slv_32_t;
use hw_fast_v1_00_a.fast9_16;
use hw_fast_v1_00_a.LOGICAL_FIFO32;
use hw_fast_v1_00_a.word_to_pixel;
entity hal_task_fast_sa is
		generic(
				TASK_ID				: string		:= "FastSP";
				C_DATA_IN_NWORDS	: natural 	:=4;
				C_DATA_OUT_NWORDS	: natural	:=1;
				--INTERNAL  PARAMETERS
				C_PARAMETER_WIDTH               :natural := (C_MACHINE_WIDTH*2);
				C_KERNEL_PROGRESS_WIDTH         :natural := C_KERNEL_PROGRESS_WIDTH;
				C_KERNEL_SCHED_PROGRESS_WIDTH    :natural := C_SCHED_PROGRESS_WIDTH;
				C_KERNEL_INDEX_WIDTH            :natural := C_KERNEL_INDEX_WIDTH;
				C_SYSCALL_ID_WIDTH: natural:=  CSYS_CALL_LEN;
				C_SYSRAM_PAGES		: natural	:=1;
				C_SYSRAM_BUFFER	: BOOLEAN:=false);
				
    Port ( clock		: in  STD_LOGIC;
           resetn		: in  STD_LOGIC;
			  ------------------------------------------------
			  -- Custom ports
			  
			    i_ready_to_receive  : in std_logic;
                o_fast_done: out std_logic;
                o_score_valid: out std_logic;
                o_is_key_point: out std_logic;
                o_pixel_score: out std_logic_vector(15 downto 0);
			  ------------------------------------------------
			    s00_kernel_swrst                    : in std_logic;
                s00_kernel_run                      : in std_logic;
                s00_kernel_rxdata					: in std_logic_vector(32*C_DATA_IN_NWORDS-1 downto 0);
                
                s00_kernel_sys_call_id      : in std_logic_vector(C_SYSCALL_ID_WIDTH-1 downto 0);
                s00_kernel_return	        : in std_logic_vector (C_PARAMETER_WIDTH-1 downto 0); -- return sys_call data
                s00_kernel_valid            : in std_logic; -- trigger task to valid return
                s00_kernel_syscall_progress : in std_logic_vector(C_KERNEL_PROGRESS_WIDTH-1 downto 0);
                s00_kernel_sched_progress    : in std_logic_vector(C_KERNEL_SCHED_PROGRESS_WIDTH-1 downto 0);
                s00_kernel_index            : in std_logic_vector(C_KERNEL_INDEX_WIDTH-1 downto 0);-- CMAXINDEX; -- handle objects INDEX 
                s00_kernel_delayed_index         : in std_logic_vector(C_KERNEL_INDEX_WIDTH-1 downto 0);-- CMAXINDEX; -- handle objects INDEX 
                s00_kernel_block_task       : in  std_ulogic;-- axuiliar signal to block task
                s00_kernel_sleep_task       : in  std_ulogic;-- axuiliar signal to sleep task
                s00_kernel_error_flag       : in std_ulogic; -- signal task with error
                
                m00_task_sys_call_id            : out std_logic_vector(C_SYSCALL_ID_WIDTH-1 downto 0);
                m00_task_parameters	            :  out std_logic_vector (C_PARAMETER_WIDTH-1 downto 0); --field for syscall arguments
                m00_task_this_call              :  out std_logic;-- trigger os sys_call
                m00_task_enable_scheduler   :  out std_logic;
                m00_task_reschedule  :  out std_logic;
                m00_task_enable_index	        :  out std_logic;
                m00_task_increment_index        :  out std_logic;
                
                
                
                m00_task_txdata				     : out 	std_logic_vector(32*C_DATA_OUT_NWORDS-1 downto 0);
                m00_task_state                  : out   std_logic_vector(C_TASK_STATE_NBITS-1 downto 0);
                m00_task_done 				    : out  	STD_LOGIC);
end hal_task_fast_sa;

architecture Behavioral of hal_task_fast_sa is

constant C_BLOCK_LEN: natural := 131072;
constant C_FIFO_IN_DEPTH: NATURAL :=512;
constant BURST_MAX: natural:=256;
CONSTANT C_CONF_LEN: NATURAL RANGE 0 TO 65535 :=12;
CONSTANT C_COORD_RESOLUTION_WIDTH: natural := 16;

constant fmessage: string := "finished...(%d,%d)\n";
constant m_len: natural := fmessage'high + 8;

type t_state is (s0_ready,s1_query_ifile,s2_query_ofile,s3_open_ofile,s4_eval_ofile,s5_query_conf,s6_read_config,s7_config_run,s8_read_file,s9_eval_fread,
s10_async_fstream_read,s11_push_pixels,s12_check_pixel_target ,s13_wait_space,s14_fin_fstream_read,s15_eval_fstream_read, 
s16_exhausted_size,s17_stop_fast_nms,s18_write_message,
s90_print_stdio,s99_exit);


signal ifile_d	:fstream_descriptor_t;
signal ifile_q	:fstream_descriptor_t;

signal blen_param_q: natural;


signal kernel_response: kernel_output_t;    					  
signal kernel_call : kernel_input_t;
signal fast_state: t_state; --reg
signal task_state_next: t_state; -- signal
signal Done_d: boolean;
signal Done_q: boolean;

-------------------------------------------------------------------------------------------------------
-- read pixels signals, counters
-------------------------------------------------------------------------------------------------------
signal ce_pixel_block_counter: std_logic;
signal inc_pixel_block_counter:std_logic;
signal pixel_block_counter: integer;

signal total_pixel_counter_q: integer;
signal pixel_counter_d: integer;
signal pixel_counter_q: integer;
signal pixels_target_d: integer;
signal pixels_target_q: integer;
signal remaining_words_d: integer;
signal remaining_words_q: integer;

signal clr_pixel_counter_i: std_logic;
signal inc_rindex_i: std_logic;
signal load_rindex_i: std_logic;

signal index_read_d : unsigned(POW2(C_BLOCK_LEN/4) downto 0);
signal index_read_q:integer;

signal w_address_offset_d:integer;
signal w_address_offset_q:integer;
signal reset_i: std_logic;


signal data_out_i: t_array_slv_32(0 to C_DATA_OUT_NWORDS-1);
signal data_in_i: t_array_slv_32(0 to C_DATA_IN_NWORDS-1);

signal config_d : t_array_slv_32(0 to 3);
signal task_done_i  : STD_LOGIC;
signal resetn_i          : std_logic;

-----------------------------------------------------------------------------------------------------------------------------
--control channel
signal WR_CE_i :  STD_LOGIC;
signal OFFSET_i:  std_logic_vector(4 downto 2);
signal control_data_i :  std_logic_vector(31 downto 0);
signal pixels_word_d :  STD_LOGIC_VECTOR(31 DOWNTO 0);

---------------------------------------------------------------------------------------------------------------
--
---------------------------------------------------------------------------------------------------------------
signal fifo_in_word_space:  STD_LOGIC_VECTOR(POW2(C_FIFO_IN_DEPTH) downto 0);
signal fifo_in_space_q  :  STD_LOGIC_VECTOR(POW2(C_FIFO_IN_DEPTH) downto 0);
signal fifo_in_size_Q  :  STD_LOGIC_VECTOR(POW2(C_FIFO_IN_DEPTH) downto 0);

SIGNAL o_word_valid 	:   STD_LOGIC;
SIGNAL fifo_pop_word_i 	:   STD_LOGIC;
SIGNAL write_pixel_word_i 	:   STD_LOGIC;
SIGNAL fifo_pop_data_pixel_i 	    :   STD_LOGIC;
SIGNAL i_fifo_pixels 	:   array_slv_32_t(0 to 0);
SIGNAL o_fifo_pixel 	:  array_slv_32_t(0 to 0);
SIGNaL o_ready_for_pixels 	:   STD_LOGIC;
signal pixel_i         :  std_logic_vector(7 downto 0);
--output

signal pixel_x_i:  std_logic_vector(C_COORD_RESOLUTION_WIDTH-1 downto 0);
signal pixel_y_i:  std_logic_vector(C_COORD_RESOLUTION_WIDTH-1 downto 0);
signal pixel_valid_i:  std_logic;
signal fast_done_i:  std_logic;


signal hw_fast_done_d: std_logic;
signal hw_fast_done_q: std_logic;



signal burst_target_d: integer;
signal burst_target_q: integer;
signal space_available_d: std_logic;
signal space_available_q: std_logic;
signal is_event_d: boolean;
signal is_event_q: boolean;

signal score_valid: std_logic_vector(0 to 0);
signal is_keypoint: std_logic_vector(0 to 0);
signal treshold_q: std_logic_vector(7 downto 0);

signal score_write          :  std_logic;
signal run_q          :  std_logic;

signal handshake_clks: integer;

begin




data_out_i<=(others=>(others=>'0'));

import_kernel_response( s00_kernel_sys_call_id,
                        s00_kernel_return,
                        s00_kernel_valid,
                        s00_kernel_syscall_progress,
                        s00_kernel_sched_progress,
                        s00_kernel_index, 
                        s00_kernel_delayed_index, 
                        s00_kernel_block_task,
                        s00_kernel_sleep_task,
                        s00_kernel_error_flag,
                        kernel_response);
                        
export_kernel_call(kernel_call,
                    m00_task_sys_call_id,
                    m00_task_parameters,
                    m00_task_this_call,
                    m00_task_enable_scheduler,
                    m00_task_reschedule,
                    m00_task_enable_index,
                    m00_task_increment_index);                        

reset_i <= not resetn_i;
resetn_i <= resetn and(not s00_kernel_swrst);
--------------------------------------------------------------------------------------------
FSM_STATES: process( clock)
--------------------------------------------------------------------------------------------

begin


	if rising_edge(clock) then

		if resetn_i = '0' then
			fast_state 		<= s0_ready;
		else
			if kernel_response.block_task = '0' then
				fast_state 		<= task_state_next;
			end if;
		end if;
		
	end if;
	
end process FSM_STATES;

------------------------------------------------------------------------------------------
m00_task_done <= task_done_i;
m00_task_txdata <= array_32_to_slv(data_out_i);
m00_task_state <= STD_LOGIC_VECTOR(to_unsigned(t_state'pos(fast_state), C_TASK_STATE_NBITS));
------------------------------------------------------------------------------------------
data_in_i<= slv_to_array_32(s00_kernel_rxdata);

----------------------------------------------------------------------------------------
CONTROL_FSM: process(fast_state,s00_kernel_run ,blen_param_q, kernel_response, fifo_in_size_Q,
	w_address_offset_q, space_available_q,	pixels_target_q, pixel_counter_q,hw_fast_done_q)
--------------------------------------------------------------------------------------------------------------------
begin
task_done_i	<= '0';
task_state_next <= fast_state;
w_address_offset_d <= w_address_offset_q;	
WR_CE_i <= '0';
load_rindex_i<='0';
inc_pixel_block_counter<='0';
clr_pixel_counter_i <= '0';
hw_fast_done_d <= hw_fast_done_q;
	
    case fast_state is
        when s0_ready=>
            hw_fast_done_d <= '0';
            task_state_next <= s0_ready;
            if s00_kernel_run = '1' then
                task_state_next <= s1_query_ifile;
            end if;
        when s1_query_ifile=>
            task_state_next <= s5_query_conf;
      
        when s5_query_conf=>
            task_state_next <= s6_read_config;
            
        when s6_read_config=>
            task_state_next <=s6_read_config;
            WR_CE_i <= '1';
            if kernel_response.index = C_CONF_LEN/4-1 then
                   task_state_next <=s7_config_run;
            end if;
                
        when s7_config_run=>
            WR_CE_i <= '1';
            inc_pixel_block_counter<= '1';
            task_state_next <=s8_read_file;		
            
        when s8_read_file=>
            task_state_next <= s9_eval_fread;
       
        when s9_eval_fread=>
            w_address_offset_d <= blen_param_q;
            task_state_next <= s18_write_message;
            load_rindex_i<='1';
            inc_pixel_block_counter<= '1';
            if(pixels_target_q > 0) then
                task_state_next <= s10_async_fstream_read;
            end if;
        
         when s10_async_fstream_read=>
            task_state_next <=s13_wait_space;
            if space_available_q = '1' then
                task_state_next <=s11_push_pixels;
            end if;
            
        when s11_push_pixels=>
            task_state_next <= s12_check_pixel_target;
      
        when s12_check_pixel_target =>
            task_state_next <=s13_wait_space;
            if(pixel_counter_q >= pixels_target_q) then
                task_state_next <= s14_fin_fstream_read;
            elsif  space_available_q = '1' then
                task_state_next <=s11_push_pixels;
            end if;
            
        when s13_wait_space=>
            task_state_next <=s11_push_pixels;

        when s14_fin_fstream_read=>
            load_rindex_i<= '1';
            clr_pixel_counter_i <='1';
            task_state_next <= s15_eval_fstream_read;
       
        when s15_eval_fstream_read =>
            inc_pixel_block_counter<= '1';
            task_state_next <=s10_async_fstream_read;
            if (pixels_target_q = 0) then
                task_state_next <=s16_exhausted_size;
            end if;
            
            if(w_address_offset_q = blen_param_q)then
               w_address_offset_d <=0;
            else
                w_address_offset_d <= blen_param_q;
            end if;  
       
        when s16_exhausted_size=>
            if(to_integer(unsigned(fifo_in_size_Q)) = 0) then
                task_state_next <= s17_stop_fast_nms;
            end if;
       
        when s17_stop_fast_nms =>
            hw_fast_done_d <= '1';
            task_state_next <= s18_write_message;
        
        when s18_write_message=>
            task_state_next <= s90_print_stdio;
        
        when s90_print_stdio=>
            task_state_next <= s99_exit;
       
        when s99_exit=>
            task_done_i<= '1';
            task_state_next <=s99_exit;
        
        when others=> null;
    end case;
		
end process CONTROL_FSM;
--------------------------------------------------------------------------


----------------------------------------------------------------------------------------
EXTENEDED_FEATURES: process(resetn_i,fast_state,  data_in_i, blen_param_q,  ifile_q, kernel_response,
	  w_address_offset_q, burst_target_q, space_available_q,pixel_block_counter, 
	  pixels_target_q, index_read_q, total_pixel_counter_q,	kernel_call)
--------------------------------------------------------------------------------------------------------------------
VARIABLE RAM_DATA:STD_LOGIC_VECTOR(31 DOWNTO 0);
begin
RAM_DATA:=(OTHERS=>'0');
is_event_d <= false;
pixels_target_d	  <= pixels_target_q;
config_d     <= (others=>(others=>'0'));
pixels_word_d <= (OTHERS=>'0');
OFFSET_i<= (others=>'0');
inc_rindex_i<='0';
write_pixel_word_i <= '0';
ifile_d<=ifile_q;
if resetn_i = '0' then
reset_sys_call(kernel_call);
else
hal_asos_link_to_kernel(kernel_response,kernel_call);
case fast_state is
    when s1_query_ifile=>
        pooled_fstream_query(kernel_call,kernel_response,ifile_q, ifile_d);
    when s5_query_conf=>
        transfer_from_host_swfifo(kernel_call,kernel_response,C_CONF_LEN);
    when s6_read_config=>
        kernel_call.enable_index <= '1';
        kernel_call.increment_index <= '1';
        config_d(0)<=data_in_i(0);
        config_d(1)<=data_in_i(1);
        config_d(2)<=data_in_i(2);
        OFFSET_i<= std_logic_vector(to_unsigned(kernel_response.index,3));
        if kernel_response.index = C_CONF_LEN/4-1 then
             kernel_call.increment_index <= '0';
        end if;
            
    when s7_config_run=>
        config_d(0)<= x"00000080";
        OFFSET_i<= std_logic_vector(to_unsigned(CONTROL_OFFSET,OFFSET_i'length));
        
    when s8_read_file=>
        pooled_fstream_read_word32_sysram(kernel_call,kernel_response,ifile_q, blen_param_q/4,w_address_offset_q/4);
        pixels_target_d <= (cast_return_to_transfer_len(kernel_response))*4;

     when s10_async_fstream_read=>
        async_pooled_fstream_read_word32_sysram(kernel_call,kernel_response, ifile_q,blen_param_q/4,w_address_offset_q/4);
        
    when s11_push_pixels=>
        unsafe_read_sysram_word32_fifo_burst(kernel_call,kernel_response,pixels_word_d,burst_target_q,index_read_q);
        write_pixel_word_i <= cast_return_to_push_data(kernel_response);
        inc_rindex_i<=cast_return_to_push_data(kernel_response);
    
        
    when s13_wait_space=>
        wait_signal_event(kernel_call,kernel_response,space_available_q,is_event_d,0);

    when s14_fin_fstream_read=>
        async_finalize_pooled_fstream_read_word32_sysram(kernel_call,kernel_response);
        pixels_target_d <= (cast_return_to_transfer_len(kernel_response))*4;
                
    
    when s17_stop_fast_nms =>
        config_d(0)<=(others=>'0');
        OFFSET_i<= std_logic_vector(to_unsigned(CONTROL_OFFSET,OFFSET_i'length));
    when s18_write_message=>
        safe_write_lram(kernel_call,kernel_response,fmessage,std_logic_vector(to_unsigned(HANDSHAKE_CLKS,32) & to_unsigned(pixel_block_counter,32)),0);
    when s90_print_stdio=>
        write_stdio(kernel_call, kernel_response,fmessage'high,m_len,0);
    when s99_exit=>
        task_exit(kernel_call, kernel_response);
    when others=> null;
end case;
end if;
end process EXTENEDED_FEATURES;
--------------------------------------------------------------------------



ce_pixel_block_counter<= '1' when pixels_target_q /= 0  and inc_pixel_block_counter = '1' else '0';

--------------------------------------------------------------------------
MUX_INDEX:PROCESS(inc_rindex_i,load_rindex_i,index_read_q,w_address_offset_q)
--------------------------------------------------------------------------
VARIABLE SEL:unsigned(1 downto 0);
variable reload_index:integer ;
BEGIN
SEL := load_rindex_i & inc_rindex_i;
reload_index:=w_address_offset_q/4;
case sel is
    when "00" =>
       index_read_d <= to_unsigned(index_read_q,index_read_d'length);
    when "01" =>
        index_read_d <= to_unsigned(index_read_q,index_read_d'length) + 1; 
    when "10"=>
        index_read_d <= to_unsigned(reload_index,index_read_d'length);
    when "11"=>
        index_read_d <= to_unsigned(reload_index,index_read_d'length);
    when others=>
        index_read_d <= to_unsigned(index_read_q,index_read_d'length);
end case;

END PROCESS MUX_INDEX;
--------------------------------------------------------------------------

--------------------------------------------------------------------------
process(pixel_counter_q, pixels_target_q)
--------------------------------------------------------------------------
variable word_target_i: unsigned(pow2(C_BLOCK_LEN) downto 0); 
variable word_counter_i: unsigned(pow2(C_BLOCK_LEN) downto 0); 
begin
    word_target_i := to_unsigned(pixels_target_q, word_target_i'length);
    word_counter_i := to_unsigned(pixel_counter_q, word_counter_i'length);
    remaining_words_d <= to_integer(word_target_i(pow2(C_BLOCK_LEN) downto 2)) - to_integer(word_counter_i(pow2(C_BLOCK_LEN) downto 2));
end process;
--------------------------------------------------------------------------


burst_target_d <= BURST_MAX when remaining_words_q > BURST_MAX else remaining_words_q;
space_available_d <= '1' when burst_target_q <= to_integer('0' & unsigned(fifo_in_word_space)) else '0';
                    
--------------------------------------------------------------------------
INDEX_REGS:PROCESS(CLOCK)
--------------------------------------------------------------------------
BEGIN
    if rising_edge(clock) then
        if reset_i = '1' then
            index_read_q<= 0;
            total_pixel_counter_q<=0;
            pixel_counter_q<=0;
            burst_target_q <= 0;
            space_available_q <= '0';
            remaining_words_q <= 0;
            w_address_offset_q<=0;
            ifile_q <=(fstream_obj,(others=>'0'),0,false);
            blen_param_q <= 0;
            
        else
            if(WR_CE_i = '1') then
                blen_param_q <= to_integer(unsigned(data_in_i(3)));
            end if;
            space_available_q <= space_available_d;
            if  kernel_response.block_task = '1'then
                index_read_q <= to_integer(index_read_d);
                pixel_counter_q<= pixel_counter_d;
                
            end if;
            if inc_rindex_i = '1' then
                total_pixel_counter_q<= total_pixel_counter_q + 4;
            end if;
            
            if kernel_response.block_task = '0'then 
                burst_target_q <= burst_target_d;
               w_address_offset_q <=w_address_offset_d;
                remaining_words_q <= remaining_words_d;

               ifile_q <=ifile_d;
            end if;
        end if;
        
    end if;

END PROCESS INDEX_REGS;
--------------------------------------------------------------------------


--------------------------------------------------------------------------
MUX_PIXEL_CNTR:PROCESS(inc_rindex_i,clr_pixel_counter_i,pixel_counter_q)
--------------------------------------------------------------------------
VARIABLE SEL:unsigned(1 downto 0);
begin
sel:=clr_pixel_counter_i & inc_rindex_i;

case sel is
    when "00"=>
        pixel_counter_d<=pixel_counter_q;
    when "01"=>
        pixel_counter_d<=pixel_counter_q + 4;
    when "10"=>
        pixel_counter_d<=0;
    when "11"=>
        pixel_counter_d<=0;
    when others=>
        pixel_counter_d<=pixel_counter_q;
end case;
end process;
--------------------------------------------------------------------------


--------------------------------------------------------------------------
TASK_REGS:process(clock)
--------------------------------------------------------------------------
begin
	if rising_edge(clock) then
		
		if resetn_i = '0' then
			pixels_target_q <= 0;
			pixel_block_counter <= 0;
			hw_fast_done_q <= '0';
			is_event_q <= false;
		else
				if ce_pixel_block_counter = '1' then
					pixel_block_counter<= pixel_block_counter +1;
				end if;
                   
            
			if kernel_response.block_task = '0' then
				pixels_target_q <= pixels_target_d;
				hw_fast_done_q <= hw_fast_done_d;
				is_event_q <= is_event_d;
			end if;
			
		end if;
	end if;
end process TASK_REGS;
--------------------------------------------------------------------------
o_fast_done <= hw_fast_done_q;
--------------------------------------------------------------------------
--------------------------------------------------------------------------
i_fifo_pixels(0) <= pixels_word_d;
------------------------------------------------------------------------------------------------
FIFO_IN:   ENTITY LOGICAL_FIFO32
------------------------------------------------------------------------------------------------
GENERIC MAP(
        FIFO_DEPTH	    => C_FIFO_IN_DEPTH,
        FIFO_N_INPUTS   => 1,
        FIFO_N_OUTPUTS  => 1)
PORT MAP(
        i_clock 	=> CLOCK,
        i_reset 	=> reset_I,
        
        i_push_data 	=> write_pixel_word_i,
        i_pop_data 	    => fifo_pop_word_i,
        i_data_port 	=> i_fifo_pixels,
        o_data_port 	=> o_fifo_pixel,
        o_size		    => fifo_in_size_Q,
        o_space	        => fifo_in_space_Q,
        o_data_valid    => o_word_valid,
        
        o_ready_for_data 	=> o_ready_for_pixels);
------------------------------------------------------------------------------------------------
fifo_in_word_space <= fifo_in_space_Q;
------------------------------------------------------------------------------------------------
demux:entity word_to_pixel 
------------------------------------------------------------------------------------------------
    Port map( 
           clock => CLOCK,
           reset => reset_I,
           
           i_word =>  o_fifo_pixel(0),
           i_data_valid => o_word_valid,
           i_pop_pixel => fifo_pop_data_pixel_i,
           
           o_pixel => pixel_i,
           o_pixel_valid => pixel_valid_i,
           o_pop_fifo => fifo_pop_word_i);
------------------------------------------------------------------------------------------------
control_data_i <= config_d(to_integer(to_unsigned(kernel_response.index,2)));
----------------------------------------------------------------------------
ufast:entity fast9_16
port map(
   ap_clk 				=> clock 	,
    ap_rst 				=> reset_I	,
    ap_start 			=> run_q,
    ap_done 			=> fast_done_i,
    ap_idle 			=>  open,
    ap_ready 			=>  open,
    i_pixel_dout 			=> pixel_i,
    i_pixel_empty_n 		=> pixel_valid_i,
    i_pixel_read 			=> fifo_pop_data_pixel_i,
    threshold 			=> treshold_q,
    o_is_keypoint 		=> is_keypoint,
    o_is_keypoint_ap_vld 	=> open 	,
    o_score_din 			=> o_pixel_score,
    o_score_full_n 		=> i_ready_to_receive,
    o_score_write 		=> score_write,
    x_coord 			=> pixel_x_i,
    x_coord_ap_vld 		=> open	,
    y_coord 			=> pixel_y_i,
    y_coord_ap_vld 		=> open ,
    o_score_valid => score_valid,
    o_score_valid_ap_vld => open);
   
     o_score_valid <=  score_write and score_valid(0);
     o_is_key_point <= is_keypoint(0);

----------------------------------------------------------------------------
 FAST_CONTROL_REGS:process(clock)
----------------------------------------------------------------------------

 begin
    if rising_edge(clock) then
        if reset_i =  '1' then
            treshold_q <=(others=>'0');
            run_q <= '0';
        elsif wr_ce_i = '1' then
            if(to_integer(unsigned(offset_i)) = CONTROL_OFFSET ) then
                run_q <= control_data_i(RUN_BIT);
            end if;
            if(to_integer(unsigned(offset_i))= THRESHOLD_OFFSET ) then
                treshold_q <= control_data_i(treshold_q'range);
            end if;
        end if;
    end if;
 end process;
----------------------------------------------------------------------------
HAND_CNT:PROCESS(clock)
BEGIN
    IF rising_edge(clock) THEN
        IF RESET_I = '1' OR fast_state = s1_query_ifile THEN
            HANDSHAKE_CLKS <= 0;
        ELSIF fast_state = s14_fin_fstream_read THEN
            HANDSHAKE_CLKS <= HANDSHAKE_CLKS + 1;
        END IF;
    END IF;
END PROCESS HAND_CNT;
----------------------------------------------------------------------------
end Behavioral;
