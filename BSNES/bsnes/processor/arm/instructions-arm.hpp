void arm_step();

void arm_opcode(uint32 rm);
void arm_move_to_status(uint32 rm);

void arm_op_multiply();
void arm_op_multiply_long();
void arm_op_memory_swap();
void arm_op_move_half_register();
void arm_op_move_half_immediate();
void arm_op_load_register();
void arm_op_load_immediate();

void arm_op_move_to_register_from_status();
void arm_op_move_to_status_from_register();
void arm_op_branch_exchange_register();

void arm_op_move_to_status_from_immediate();
void arm_op_data_immediate_shift();
void arm_op_data_register_shift();
void arm_op_data_immediate();
void arm_op_move_immediate_offset();
void arm_op_move_register_offset();
void arm_op_move_multiple();
void arm_op_branch();
void arm_op_software_interrupt();
