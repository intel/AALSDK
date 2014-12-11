import os, re, commands, sys

# Open Transactions file
if (len(sys.argv) < 2):
    sys.exit("python check_transactions.py <transactions log>")

filename = sys.argv[1]
fd_lines = open(filename).readlines()

csr_write_cnt = 0
rdreq_meta_list = []
rdresp_meta_list = []
wrreq_meta_list = []
wrresp_meta_list = []

for line in fd_lines:
    line_tokens = str(line).split('\t')
    if line_tokens[1] == "CSRWrite":
        csr_write_cnt = csr_write_cnt + 1
    elif "RdLineReq_S" in line_tokens[1]:
        rdreq_meta_list.append(line_tokens[3])
    elif  "RdResp"  in line_tokens[1]:
        rdresp_meta_list.append(line_tokens[3])
    elif  "WrLineReq" in line_tokens[1]:
        wrreq_meta_list.append(line_tokens[3])
    elif  "WrResp" in line_tokens[1]:
        wrresp_meta_list.append(line_tokens[3])

print "# CSR_write", csr_write_cnt
print "# RdReq    ", len(rdreq_meta_list)
print "# RdResp   ", len(rdresp_meta_list)
print "# WrReq    ", len(wrreq_meta_list)
print "# WrResp   ", len(wrresp_meta_list)

# Checking read responses
print [x for x in rdreq_meta_list if x not in rdresp_meta_list]
