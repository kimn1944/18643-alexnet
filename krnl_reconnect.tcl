proc reconnect_krnl {krnl} {
    set suffix "_m_axi_gmem"
    delete_bd_objs [get_bd_intf_nets "$krnl$suffix"]
    connect_bd_intf_net [get_bd_intf_pins $krnl/m_axi_gmem] -boundary_type upper [get_bd_intf_pins interconnect_axifull_2_user_slr1/S00_AXI]
    assign_bd_address -target_address_space /$krnl/Data_m_axi_gmem [get_bd_addr_segs interconnect_aximm_ddrmem3_M00_AXI/Reg] -force
    connect_bd_net [get_bd_pins /$krnl/ap_clk] [get_bd_pins interconnect_axifull_2_user_slr1/S00_ACLK]
    connect_bd_net [get_bd_pins interconnect_axifull_2_user_slr1/S00_ARESETN] [get_bd_pins /$krnl/ap_rst_n] -boundary_type upper
}

delete_bd_objs [get_bd_intf_nets axi_vip_1_M_AXI]
delete_bd_objs [get_bd_cells axi_vip_1]
set_property -dict [list CONFIG.NUM_SI {1} CONFIG.NUM_MI {1}] [get_bd_cells interconnect_axifull_2_user_slr1]
set_property -dict [list CONFIG.S00_HAS_DATA_FIFO {1}] [get_bd_cells interconnect_axifull_2_user_slr1]

disconnect_bd_net /clkwiz_kernel2_clk_out1_1 [get_bd_pins interconnect_axifull_2_user_slr1/S00_ACLK]
disconnect_bd_net /proc_sys_reset_0_interconnect_aresetn [get_bd_pins interconnect_axifull_2_user_slr1/S00_ARESETN]

if {[get_bd_intf_nets krnl_cnn_layer1_1_m_axi_gmem] != ""} {
    reconnect_krnl krnl_cnn_layer1_1
}

if {[get_bd_intf_nets krnl_cnn_layer0_1_m_axi_gmem] != ""} {
    reconnect_krnl krnl_cnn_layer0_1
}

if {[get_bd_intf_nets krnl_cnn_layer2_1_m_axi_gmem] != ""} {
    reconnect_krnl krnl_cnn_layer2_1
}

if {[get_bd_intf_nets krnl_cnn_layer3_1_m_axi_gmem] != ""} {
    reconnect_krnl krnl_cnn_layer3_1
}

if {[get_bd_intf_nets krnl_cnn_layer4_1_m_axi_gmem] != ""} {
    reconnect_krnl krnl_cnn_layer4_1
}

set_property offset 0x00000000 [get_bd_addr_segs {krnl_cnn_layer*_1/Data_m_axi_gmem/SEG_interconnect_aximm_ddrmem3_M00_AXI_Reg}]
set_property range 2G [get_bd_addr_segs {krnl_cnn_layer*_1/Data_m_axi_gmem/SEG_interconnect_aximm_ddrmem3_M00_AXI_Reg}]
