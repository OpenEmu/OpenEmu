void thumb_step();

void thumb_opcode(uint4 opcode, uint4 d, uint4 s);

void thumb_op_adjust_register();
void thumb_op_adjust_immediate();
void thumb_op_shift_immediate();
void thumb_op_immediate();
void thumb_op_alu();
void thumb_op_branch_exchange();
void thumb_op_alu_hi();
void thumb_op_load_literal();
void thumb_op_move_register_offset();
void thumb_op_move_word_immediate();
void thumb_op_move_byte_immediate();
void thumb_op_move_half_immediate();
void thumb_op_move_stack();
void thumb_op_add_register_hi();
void thumb_op_adjust_stack();
void thumb_op_stack_multiple();
void thumb_op_move_multiple();
void thumb_op_software_interrupt();
void thumb_op_branch_conditional();
void thumb_op_branch_short();
void thumb_op_branch_long_prefix();
void thumb_op_branch_long_suffix();
