.global flush_uppermem_cache @ void *start_address, void *end_address, int flags

flush_uppermem_cache:
    swi #0x9f0002
    bx lr
