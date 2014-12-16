dump -depth 0 

dump -add /tb_latency_scoreboard/buffer/records
dump -add /tb_latency_scoreboard/buffer/infifo/RAM_i/ram
dump -add /tb_latency_scoreboard/buffer/infifo_nowrfence/RAM_i/ram
dump -add /tb_latency_scoreboard/buffer/outfifo/RAM_i/ram
dump -add /tb_latency_scoreboard/buffer/slot_lookup

# dump -add /tb_ase_fifo/inst_fifo/RAM_i/ram

run 
quit
