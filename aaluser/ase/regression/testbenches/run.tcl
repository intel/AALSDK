dump -depth 0 

# dump -add /tb_latency_scoreboard/buffer/records
# dump -add /tb_latency_scoreboard/buffer/infifo/my_queue
# dump -add /tb_latency_scoreboard/buffer/infifo_nowrfence/my_queue
# dump -add /tb_latency_scoreboard/buffer/outfifo/my_queue
# dump -add /tb_latency_scoreboard/buffer/slot_lookup

# dump -add /tb_ase_fifo/inst_fifo/RAM_i/ram

# dump -add /tb_ase_fifo/inst_fifo/my_queue

run 
quit
