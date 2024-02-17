	count_bin u0 (
		.button_pio_external_connection_export    (<connected-to-button_pio_external_connection_export>),    //    button_pio_external_connection.export
		.led_pio_external_connection_export       (<connected-to-led_pio_external_connection_export>),       //       led_pio_external_connection.export
		.new_sdram_controller_0_wire_addr         (<connected-to-new_sdram_controller_0_wire_addr>),         //       new_sdram_controller_0_wire.addr
		.new_sdram_controller_0_wire_ba           (<connected-to-new_sdram_controller_0_wire_ba>),           //                                  .ba
		.new_sdram_controller_0_wire_cas_n        (<connected-to-new_sdram_controller_0_wire_cas_n>),        //                                  .cas_n
		.new_sdram_controller_0_wire_cke          (<connected-to-new_sdram_controller_0_wire_cke>),          //                                  .cke
		.new_sdram_controller_0_wire_cs_n         (<connected-to-new_sdram_controller_0_wire_cs_n>),         //                                  .cs_n
		.new_sdram_controller_0_wire_dq           (<connected-to-new_sdram_controller_0_wire_dq>),           //                                  .dq
		.new_sdram_controller_0_wire_dqm          (<connected-to-new_sdram_controller_0_wire_dqm>),          //                                  .dqm
		.new_sdram_controller_0_wire_ras_n        (<connected-to-new_sdram_controller_0_wire_ras_n>),        //                                  .ras_n
		.new_sdram_controller_0_wire_we_n         (<connected-to-new_sdram_controller_0_wire_we_n>),         //                                  .we_n
		.seven_seg_pio_external_connection_export (<connected-to-seven_seg_pio_external_connection_export>), // seven_seg_pio_external_connection.export
		.sys_sdram_pll_0_ref_clk_clk              (<connected-to-sys_sdram_pll_0_ref_clk_clk>),              //           sys_sdram_pll_0_ref_clk.clk
		.sys_sdram_pll_0_ref_reset_reset          (<connected-to-sys_sdram_pll_0_ref_reset_reset>),          //         sys_sdram_pll_0_ref_reset.reset
		.sys_sdram_pll_0_sdram_clk_clk            (<connected-to-sys_sdram_pll_0_sdram_clk_clk>)             //         sys_sdram_pll_0_sdram_clk.clk
	);

