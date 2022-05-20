----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/25/2021 02:26:58 PM
-- Design Name: 
-- Module Name: hal_task_nms_dpnet - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------

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
use hw_fast_v1_00_a.fast_pkg.CONTROL_OFFSET;
use hw_fast_v1_00_a.fast_pkg.array_slv_32_t;
use hw_fast_v1_00_a.fast_pkg.array_slv_16_t;
use hw_fast_v1_00_a.nms9_double;
use hw_fast_v1_00_a.LOGICAL_FIFO32;


entity hal_task_nms_dpnet is
	generic(
				TASK_ID				: string		:= "MNonmaxDPNet0";
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
			  i_wr_score          : in std_logic;
              i_is_keypoint0       : IN std_logic;
              i_is_keypoint1       : IN std_logic;
              i_pixel0_score             : IN std_logic_vector(15 downto 0);
              i_pixel1_score             : IN std_logic_vector(15 downto 0);
              i_fast_done         : IN std_logic;
              o_ready_for_data    : out std_logic;
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
end hal_task_nms_dpnet;

architecture Behavioral of hal_task_nms_dpnet is


constant C_BLOCK_LEN: natural :=131072;
constant C_TRGT_UPLOAD_B: natural :=C_BLOCK_LEN;
CONSTANT C_TRGT_WCRNRS_W: NATURAL :=256;
CONSTANT C_FIFO_OUT_DPTH: NATURAL :=2*C_TRGT_WCRNRS_W;

CONSTANT C_CONF_LEN: NATURAL RANGE 0 TO 65535 :=16;
CONSTANT C_RAM_WIDTH: NATURAL := 13; 
CONSTANT C_RESOLUTION_WIDTH: natural := 16;
CONSTANT C_CIRCLE_DIAMETER: NATURAL := 7;

type t_state is (s0_ready,s1_query_ofile,s2_query_conf,s3_read_config,s4_config_run, s5_check_crnrs_size,
s7_write_corners,s8_unlock_rsrc_mutex, s9_check_upld_tgt,s10_fstream_async_write,s11_fstream_finalz_write, s12_eval_fstream_write, 
s13_check_done, s14_wait_corners, s15_check_corners_last, s16_fstream_finwrite, s17_stop_fast_nms, s18_write_message,s90_print_stdio,s99_exit);




signal psocket_d	:socket_descriptor_t;
signal psocket_q	:socket_descriptor_t;


signal kernel_response: kernel_output_t;    					  
signal kernel_call : kernel_input_t;
signal nms_state: t_state; --reg
signal task_state_next: t_state; -- signal
signal Done_d: boolean;
signal Done_q: boolean;

    
-------------------------------------------------------------------------------------------------------
-- read pixels signals, counters
-------------------------------------------------------------------------------------------------------

signal status_ret_d:integer;
signal status_ret_q:integer;
-------------------------------------------------------------------------------------------------------
-- write corners signals, counters
-------------------------------------------------------------------------------------------------------

signal inc_last_windex_i: std_logic;
signal inc_windex_i: std_logic;
signal load_windex_i: std_logic;
signal index_write_d : unsigned(POW2(C_TRGT_UPLOAD_B)-2 downto 0);
signal index_write_q:integer;
signal remaining_space_d:integer;
signal remaining_space_q:integer;

signal blen_param_q: natural;

signal count_crnrs_q :integer;
signal count_crnrs_d : unsigned(31 downto 0);

signal psocket_len_d :integer;
signal psocket_len_q :integer;
signal count_crnr_bytes_q :integer;
signal count_crnr_bytes_d :UNSIGNED (31 DOWNTO 0);

signal fifo_out_burst_d  :  integer;
signal fifo_out_burst_q  :  integer;

signal r_address_offset_d:INTEGER;
signal r_address_offset_q:integer;
signal pending_transfer_d : integer range 0 to 3;
signal pending_transfer_q : integer range 0 to 3;
signal inc_pending: std_logic;
signal dec_pending: std_logic;

-------------------------------------------------------------------------------------------------------


signal reset_i: std_logic;
constant fmessage: string := "finished...(%d,%d)\n";
constant m_len: natural := fmessage'high + 8;

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
signal pixels_word_d :  STD_LOGIC_VECTOR(31 DOWNTO 0);--t_array_slv_32(0 to 3);

-----------------------------------------------------------------------------------------------------------------------------
--nms
----------------------------------------------------------------------------------------------------------------------------


SIGNAL x_coord_ap_vld  :  STD_LOGIC;
SIGNAL y_coord_ap_vld  :  STD_LOGIC;
SIGNAL is_crnr_i  :  STD_LOGIC;
SIGNAL nms_done_i :  STD_LOGIC;



			  
SIGNAL push_crnrs_i 	:  STD_LOGIC;
SIGNAL pop_crnrs_i 	:   STD_LOGIC;
SIGNAL fifo_input_i 	:  	array_slv_32_t(0 to 0);
SIGNAL fifo_output_i 	:   array_slv_32_t(0 to 0);
SIGNAL fifo_size		: STD_LOGIC_VECTOR(POW2(C_FIFO_OUT_DPTH) downto 0);
SIGNAL fifo_size_block_q		: integer;
SIGNAL fifo_data_valid : STD_LOGIC;
SIGNAL fifo_ready_for_data 	:   STD_LOGIC;
signal corners_word_i: std_logic_vector(31 downto 0);

signal wake_control_d: std_logic;
signal wake_control_q: std_logic;
signal is_event_d: boolean;
signal is_event_q: boolean;
signal low_space_q: std_logic;
signal low_space_d: std_logic;
signal upload_crnr_bytes_d :integer;
signal upload_crnr_bytes_q :integer;

signal i_ff1_data_port 	:   array_slv_32_t(0 to 1-1);
signal o_ff1_data_port 	:  array_slv_32_t(0 to 1-1);
signal o_ff1_data_valid    :   STD_LOGIC;
signal score0_dout 			:  STD_LOGIC_VECTOR (15 downto 0);
signal score1_dout 			:  STD_LOGIC_VECTOR (15 downto 0);
signal score_read 			:  STD_LOGIC;
signal run_q 			:  STD_LOGIC;

signal is_corner_full_n 			:  STD_LOGIC;
signal is_corner_din 			:  STD_LOGIC_VECTOR (0 downto 0);
signal is_corner_write 			:  STD_LOGIC;

signal x_coord:  STD_LOGIC_VECTOR (15 downto 0);
signal y_coord:  STD_LOGIC_VECTOR (15 downto 0);

signal handshake_hls: integer;

attribute mark_debug : string;
attribute mark_debug of nms_state: signal is "true";
attribute mark_debug of fifo_size: signal is "true";

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
			nms_state 		<= s0_ready;
		else
			if kernel_response.block_task = '0' then
				nms_state 		<= task_state_next;
			end if;
		end if;
		
	end if;
	
end process FSM_STATES;
------------------------------------------------------------------------------------------
--FSM_ASYNC_OUTPUTS: process(clock)
------------------------------------------------------------------------------------------

    m00_task_done <= task_done_i;
    m00_task_txdata <= array_32_to_slv(data_out_i);
    m00_task_state <= STD_LOGIC_VECTOR(to_unsigned(t_state'pos(nms_state), C_TASK_STATE_NBITS));


----------------------------------------------------------------------------------------
data_in_i<= slv_to_array_32(s00_kernel_rxdata);


----------------------------------------------------------------------------------------
CONTROL_STATE: process(nms_state,
					s00_kernel_run ,
					data_in_i,
					status_ret_q,
					resetn_i, 
					kernel_response, 
					remaining_space_q,
					count_crnr_bytes_q,
					index_write_q,
					nms_done_i,
					fifo_size_block_q,
					fifo_out_burst_q,
					count_crnrs_q,
                    corners_word_i,
					psocket_len_q,
					i_fast_done,
					r_address_offset_q,
					pending_transfer_q,
					wake_control_q,
					upload_crnr_bytes_q)
--------------------------------------------------------------------------------------------------------------------
VARIABLE RAM_DATA:STD_LOGIC_VECTOR(31 DOWNTO 0);
begin
task_done_i	<= '0';
config_d     <= (others=>(others=>'0'));
task_state_next <= nms_state;
WR_CE_i <= '0';
OFFSET_i<= (others=>'0');
pop_crnrs_i <= '0';
load_windex_i<='0';
inc_windex_i <='0';
inc_pending<='0';
dec_pending<='0';
	if resetn_i = '0' then
			load_windex_i <= '1';
	else
	
			case nms_state is
				when s0_ready=>
					task_state_next <= s0_ready;
				    if s00_kernel_run = '1' then
						task_state_next <= s1_query_ofile;
					end if;
				when s1_query_ofile=>
					task_state_next <=s2_query_conf;
				when s2_query_conf=>
				    task_state_next <= s3_read_config;
					
				when s3_read_config=>
				  
				    WR_CE_i <= '1';
				    config_d(0)<=data_in_i(0);
				    config_d(1)<=data_in_i(1);
				    config_d(2)<=data_in_i(2);
                    OFFSET_i<= std_logic_vector(to_unsigned(kernel_response.index,3));
                    if kernel_response.index = C_CONF_LEN/4-2 then
			             task_state_next <=s4_config_run;
			        end if;
				when s4_config_run=>
					config_d(0)<= x"00000080";
				    WR_CE_i <= '1';
                    OFFSET_i<= std_logic_vector(to_unsigned(CONTROL_OFFSET,OFFSET_i'length));
				    task_state_next <=s5_check_crnrs_size;		
				when s5_check_crnrs_size=>
				    task_state_next <= s9_check_upld_tgt;
					if fifo_size_block_q >= fifo_out_burst_q  and  fifo_out_burst_q > 0 then				
						task_state_next <= s7_write_corners;
					end if;		
				when s7_write_corners=>
				    pop_crnrs_i <= cast_return_to_pop_data(kernel_response);
					inc_windex_i <= cast_return_to_pop_data(kernel_response);
					task_state_next <= s9_check_upld_tgt;
  				when s9_check_upld_tgt=>
			        task_state_next <= s13_check_done;
                    if count_crnr_bytes_q >= upload_crnr_bytes_q then
						task_state_next <= s10_fstream_async_write;
					end if;	
				when s10_fstream_async_write=>
				    load_windex_i <= '1';	
				    inc_pending <= '1';
				    if pending_transfer_q = 1 then
						task_state_next <= s11_fstream_finalz_write;
					else
					   task_state_next<= s13_check_done;
					end if;	
				when s11_fstream_finalz_write=>
					task_state_next <= s12_eval_fstream_write; 
				when s12_eval_fstream_write=>
 					task_state_next <=  s13_check_done;
					dec_pending <= '1';
					if psocket_len_q = 0 then
						task_state_next <= s17_stop_fast_nms;
					end if; 
				when s13_check_done=>
					if i_fast_done = '1' and fifo_size_block_q = 0 then
					    task_state_next <= s15_check_corners_last;
					elsif wake_control_q = '1' then
					   task_state_next <= s5_check_crnrs_size;
					else
					   task_state_next <= s14_wait_corners;
					end if;
                when s14_wait_corners=>
                    task_state_next <= s5_check_crnrs_size;
			    when s15_check_corners_last=>
					task_state_next <= s17_stop_fast_nms;
					if pending_transfer_q = 1 then
						task_state_next <= s16_fstream_finwrite;
					elsif count_crnr_bytes_q > 0 then
						task_state_next <= s10_fstream_async_write;
					end if;	
				when s16_fstream_finwrite=>
					dec_pending<='1';
				    load_windex_i <= '1';	
					task_state_next<= s17_stop_fast_nms;
				when s17_stop_fast_nms =>
					config_d(0)<=(others=>'0');
					WR_CE_i <= '1';
					OFFSET_i<= std_logic_vector(to_unsigned(CONTROL_OFFSET,OFFSET_i'length));
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
		end if;
		
end process CONTROL_STATE;
--------------------------------------------------------------------------

----------------------------------------------------------------------------------------
EXTENEDED_FEATURES: process(resetn_i,nms_state, psocket_q, kernel_response, 
					index_write_q, 	fifo_out_burst_q,corners_word_i,
					psocket_len_q, r_address_offset_q, wake_control_q,
					upload_crnr_bytes_q, count_crnrs_q,handshake_hls)
--------------------------------------------------------------------------------------------------------------------
begin
psocket_len_d <= psocket_len_q;
psocket_d<=psocket_q;
is_event_d <= false;
	if resetn_i = '0' then
			reset_sys_call(kernel_call);
	else
	        hal_asos_link_to_kernel(kernel_response,kernel_call);
			case nms_state is
				when s1_query_ofile=>
				    pooled_socket_query(kernel_call,kernel_response,psocket_q, psocket_d);
				when s2_query_conf=>
				    transfer_from_host_swfifo(kernel_call,kernel_response,C_CONF_LEN);
				when s3_read_config=>
				    kernel_call.enable_index <= '1';
				    kernel_call.increment_index <= '1';
				    if kernel_response.index = C_CONF_LEN/4-2 then
			             kernel_call.increment_index <= '0';
			        end if;
				when s7_write_corners=>
				    unsafe_write_sysram_word32_burst_fifo(kernel_call,kernel_response,corners_word_i,fifo_out_burst_q,index_write_q);
				when s10_fstream_async_write=>
 					async_pooled_socket_write_word32_sysram(kernel_call, kernel_response,psocket_q, upload_crnr_bytes_q/4,r_address_offset_q/4);
				when s11_fstream_finalz_write=>
					async_finalize_pooled_fstream_write_sysram(kernel_call, kernel_response);
				    psocket_len_d <= cast_return_to_transfer_len(kernel_response);
                when s14_wait_corners=>
                    wait_signal_event(kernel_call,kernel_response,wake_control_q,is_event_d,0);
				when s16_fstream_finwrite=>
				    async_finalize_pooled_socket_write_sysram(kernel_call, kernel_response);
   				    psocket_len_d <= cast_return_to_transfer_len(kernel_response);
				when s18_write_message=>
					safe_write_lram(kernel_call,kernel_response,fmessage,std_logic_vector(to_unsigned(handshake_hls,32)) & std_logic_vector(to_unsigned(count_crnrs_q,32)),0);
				when s90_print_stdio=>
					write_stdio(kernel_call, kernel_response,fmessage'high,m_len,0);
				when s99_exit=>
					task_exit(kernel_call, kernel_response);
				when others=> null;
			end case;
	end if;	
end process EXTENEDED_FEATURES;
--------------------------------------------------------------------------




--------------------------------------------------------------------------
INDEX_REGS:PROCESS(CLOCK)
--------------------------------------------------------------------------
BEGIN
    if rising_edge(clock) then
        if reset_i = '1' then
            psocket_q <=(net_obj,(others=>'0'),0,false);
        else
            
            if kernel_response.block_task = '0'then 
               psocket_q <=psocket_d;
            end if;
        end if;
        
    end if;

END PROCESS INDEX_REGS;
--------------------------------------------------------------------------

PENDING_CNTR:PROCESS(pending_transfer_q,inc_pending,dec_pending)
variable sel: unsigned(1 downto 0);
BEGIN
sel := inc_pending & dec_pending;

case sel is
    when "00" =>
        pending_transfer_d <= pending_transfer_q;
    when "01" =>
         pending_transfer_d <= pending_transfer_q - 1;
    when "10" =>
         pending_transfer_d <= pending_transfer_q + 1;
    when "11" =>
        pending_transfer_d <= 0;
    when others =>
        pending_transfer_d <= pending_transfer_q;
end case;

END PROCESS PENDING_CNTR;
--------------------------------------------------------------------------


REM_SPACE:process(count_crnr_bytes_q,blen_param_q)
begin
remaining_space_d <= blen_param_q - count_crnr_bytes_q;
end process;
--------------------------------------------------------------------------
------------------------------------------------------------------------------------
FIFO_OUT_BRST:PROCESS(remaining_space_q,i_fast_done,fifo_size,low_space_q)
------------------------------------------------------------------------------------
variable sel: unsigned (1 downto 0);
BEGIN 

sel := i_fast_done & low_space_q;

case sel is
    when "00"=>
        fifo_out_burst_d <= C_TRGT_WCRNRS_W;
    when "01"=>
        fifo_out_burst_d <= (remaining_space_q/4);
    when "10"=>
        fifo_out_burst_d <= to_integer(unsigned(fifo_size));
    when "11"=>
        fifo_out_burst_d <= (remaining_space_q/4);
    when others=>
        fifo_out_burst_d <= C_TRGT_WCRNRS_W;
end case;
END PROCESS FIFO_OUT_BRST;
------------------------------------------------------------------------------------
low_space_d <= '1' when (remaining_space_q) < C_TRGT_WCRNRS_W else '0';
--------------------------------------------------------------------------
MUX_INDEX_OUT:PROCESS(inc_windex_i,index_write_q)
--------------------------------------------------------------------------
BEGIN
       index_write_d <= to_unsigned(index_write_q,index_write_d'length);
    if inc_windex_i = '1' then
        index_write_d <= to_unsigned(index_write_q,index_write_d'length) + 1; 
    end if;
END PROCESS MUX_INDEX_OUT;
--------------------------------------------------------------------------

wake_control_d <= '1' when to_integer(unsigned(fifo_size)) >=C_TRGT_WCRNRS_W or i_fast_done = '1' else '0';
--------------------------------------------------------------------------
MUX_ROFFSET:PROCESS(r_address_offset_q, blen_param_q)
--------------------------------------------------------------------------
BEGIN

if r_address_offset_q = 0 then
    r_address_offset_d <= blen_param_q;
else
    r_address_offset_d <=0;
end if;
END PROCESS MUX_ROFFSET;
--------------------------------------------------------------------------


--------------------------------------------------------------------------
MUX_CORNER_BYTES:PROCESS(inc_windex_i,load_windex_i,count_crnr_bytes_q,  psocket_len_q)
--------------------------------------------------------------------------
VARIABLE SEL:unsigned(1 downto 0);
BEGIN
SEL := load_windex_i & inc_windex_i;

case sel is
    when "00" =>
       count_crnr_bytes_d <= to_unsigned(count_crnr_bytes_q,count_crnr_bytes_d'length);  
    when "01" =>
       count_crnr_bytes_d <= to_unsigned(count_crnr_bytes_q,count_crnr_bytes_d'length) + 4; 
    when "10" =>
       count_crnr_bytes_d<=(others=>'0');
     when "11" =>
       count_crnr_bytes_d<=(others=>'0');
    when others=>
        count_crnr_bytes_d <= to_unsigned(count_crnr_bytes_q,count_crnr_bytes_d'length);
end case;
END PROCESS MUX_CORNER_BYTES;
--------------------------------------------------------------------------

--------------------------------------------------------------------------
MUX_CORNER_OUT:PROCESS(  inc_windex_i,count_crnrs_q)
--------------------------------------------------------------------------
BEGIN

case inc_windex_i is
    when '0' =>
       count_crnrs_d <= to_unsigned(count_crnrs_q,count_crnrs_d'length);  
    when '1' =>
        count_crnrs_d <= to_unsigned(count_crnrs_q,count_crnrs_d'length) + 1; 
    WHEN OTHERS=>
        count_crnrs_d <= to_unsigned(count_crnrs_q,count_crnrs_d'length);  
    end case;
END PROCESS MUX_CORNER_OUT;
--------------------------------------------------------------------------
upload_crnr_bytes_d <= to_integer(unsigned(data_in_i(3))) when i_fast_done = '0' else count_crnr_bytes_q;

--------------------------------------------------------------------------
TASK_REGS:process(clock)
--------------------------------------------------------------------------
begin
	if rising_edge(clock) then
		
		if resetn_i = '0' then
			count_crnr_bytes_q <= 0;
			psocket_len_q<=0;
            pending_transfer_q <= 0;
            index_write_q<= 0;
			count_crnrs_q <= 0;
			status_ret_q <= -1;
			remaining_space_q <=0;
			
			fifo_out_burst_q <= 0;
			r_address_offset_q <= 0;
			wake_control_q <= '0';
			is_event_q <= false;
			upload_crnr_bytes_q <= C_TRGT_UPLOAD_B;
		else
		
		    if(WR_CE_i = '1') then
                blen_param_q <= to_integer(unsigned(data_in_i(3)));
            end if;
            if(WR_CE_i = '1') or i_fast_done ='1' then
                upload_crnr_bytes_q <=upload_crnr_bytes_d;
            end if;
            
		    wake_control_q <= wake_control_d;
            if  kernel_response.block_task = '1' or load_windex_i <= '1' then


                index_write_q <= TO_INTEGER(index_write_d);
                count_crnr_bytes_q <=  TO_INTEGER(count_crnr_bytes_d);
                remaining_space_q<=remaining_space_d;
            end if;
            if  kernel_response.block_task = '1'then
                
                count_crnrs_q <= TO_INTEGER(count_crnrs_d);
            end if;
            
            if kernel_response.block_task = '0' and load_windex_i = '1' then
                r_address_offset_q <= r_address_offset_d;
            end if;
            
			if kernel_response.block_task = '0' then
			    is_event_q <= is_event_d;
			    low_space_q <= low_space_d;
			    pending_transfer_q <= pending_transfer_d;
			    status_ret_q <= status_ret_d;
			    fifo_out_burst_q <=  fifo_out_burst_d;
			
                fifo_size_block_q <= to_integer(unsigned(fifo_size));
				psocket_len_q <= psocket_len_d;

			end if;
			
		end if;
	end if;
end process TASK_REGS;
--------------------------------------------------------------------------
control_data_i <= config_d(to_integer(to_unsigned(kernel_response.index,2)));
----------------------------------------------------------------------------
     i_ff1_data_port(0) <= i_pixel1_score &i_pixel0_score;
------------------------------------------------------------------------------------------------
scores_ff:ENTITY logical_fifo32 
------------------------------------------------------------------------------------------------
generic map(
				FIFO_DEPTH	=> 8, 
				FIFO_N_INPUTS =>1,
				FIFO_N_OUTPUTS =>1)
    Port map ( 
                i_clock 	=> clock,
                i_reset 	=> reset_i,
                
                i_push_data 	=> i_wr_score,
                i_pop_data 	    => score_read,
                i_data_port 	=> i_ff1_data_port,
                o_data_port 	=> o_ff1_data_port,
                o_size		    => open,
                o_space	        => open,
                o_data_valid    => o_ff1_data_valid,
                o_ready_for_data=> o_ready_for_data);
------------------------------------------------------------------------------------------------
  score0_dout <= o_ff1_data_port(0)(15 downto 0);
  score1_dout <= o_ff1_data_port(0)(31 downto 16);
------------------------------------------------------------------------------------------------
unms:entity nms9_double 
port map(
    ap_clk 			=> clock 	  ,
    ap_rst 			=> reset_I 	  ,
    ap_start 		=> run_q  ,
    ap_done 		=> nms_done_i  ,
    ap_idle 		=> open 	  ,
    ap_ready 		=> open  ,
    score0_dout 		=>  score0_dout,
    score0_empty_n 	=> o_ff1_data_valid,
    score0_read 		=> score_read ,
    score1_dout 		=>  score1_dout,
    score1_empty_n 	=> o_ff1_data_valid,
    score1_read 		=> open ,
	is_corner_din 		=> is_corner_din,	
    is_corner_full_n 	=> fifo_ready_for_data,	
    is_corner_write 	=> is_corner_write,	
    x_coord 			=> x_coord,	
    x_coord_ap_vld 	    => x_coord_ap_vld,	
    y_coord 			=> y_coord,	
    y_coord_ap_vld 	    => y_coord_ap_vld);	
------------------------------------------------------------------------------------------------


is_crnr_i <= is_corner_din(0);
push_crnrs_i    <= is_crnr_i AND x_coord_ap_vld and y_coord_ap_vld;
fifo_input_i(0)    <= y_coord & x_coord;





------------------------------------------------------------------------------------------------
FIFO_OUT:   ENTITY LOGICAL_FIFO32
------------------------------------------------------------------------------------------------
GENERIC MAP(
        FIFO_DEPTH	    => C_FIFO_OUT_DPTH,
        FIFO_N_INPUTS   => 1,
        FIFO_N_OUTPUTS  => 1)
PORT MAP(
        i_clock 	=> CLOCK,
        i_reset 	=> reset_I,
        
        i_push_data 	=> push_crnrs_i,
        i_pop_data 	    => pop_crnrs_i,
        i_data_port 	=> fifo_input_i,
        o_data_port 	=> fifo_output_i,
        o_size		    => fifo_size,
        o_space	        => open,
        o_data_valid    => fifo_data_valid,
        
        o_ready_for_data 	=> fifo_ready_for_data);
------------------------------------------------------------------------------------------------
corners_word_i <= fifo_output_i(0);
----------------------------------------------------------------------------
 FAST_CONTROL_REGS:process(clock)
----------------------------------------------------------------------------

 begin
    if rising_edge(clock) then
        if reset_i =  '1' then
            run_q <= '0';
        elsif wr_ce_i = '1' then
            if(to_integer(unsigned(offset_i)) = CONTROL_OFFSET ) then
                run_q <= control_data_i(RUN_BIT);
            end if;
        end if;
    end if;
 end process;
----------------------------------------------------------------------------
------------------------------------------------------------------------
HAND_REG:process(clock)
------------------------------------------------------------------------
begin
  if reset_i =  '1' or nms_state = s1_query_ofile then
            handshake_hls <= 0;
        elsif nms_state = s16_fstream_finwrite and kernel_response.block_task = '1' then
            handshake_hls <= handshake_hls + 1;
        end if;
end process HAND_REG;
--------------------------------------------------------------------------
end Behavioral;
