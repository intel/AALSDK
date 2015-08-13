Hello_AAL/SW/helloaal >OUTPUT.txt
awk '{print "///"$0}' OUTPUT.txt >OUTPUT1.txt
cp doc/output/OUTPUT.dox doc/output/final.dox 
sed -i '/HelloAAL OUTPUT/r OUTPUT1.txt' doc/output/final.dox 
Hello_CCI_NLB/SW/helloCCInlb >OUTPUT.txt
awk '{print "///"$0}' OUTPUT.txt >OUTPUT1.txt
sed -i '/HelloCCINLB OUTPUT/r OUTPUT1.txt' doc/output/final.dox 
Hello_SPL_LB/SW/helloSPLlb >OUTPUT.txt
awk '{print "///"$0}' OUTPUT.txt >OUTPUT1.txt
sed -i '/HelloSPLLB OUTPUT/r OUTPUT1.txt' doc/output/final.dox  
rm OUTPUT.txt OUTPUT1.txt

