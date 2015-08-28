#!/bin/bash

rm doc/output/final.dox >>/dev/null 2>&1
cp doc/output/OUTPUT.dox doc/output/final.dox

if  [ -a Hello_AAL/SW/helloaal ]; then
   timeout 2s Hello_AAL/SW/helloaal >OUTPUT.txt 2>&1
   temp=`grep -c "Error" OUTPUT.txt`
   if [ $temp -ne 0 ];
   then
      sed -n 16,26p doc/output/err.txt >>errtemp.txt
      sed -i '/HelloAAL OUTPUT/r errtemp.txt' doc/output/final.dox 
      rm errtemp.txt
   else
      awk '{print "///"$0}' OUTPUT.txt >OUTPUT1.txt
      sed -i '/HelloAAL OUTPUT/r OUTPUT1.txt' doc/output/final.dox 
      rm OUTPUT.txt OUTPUT1.txt
   fi
else
   sed -n 3,13p doc/output/err.txt >>errtemp.txt
   sed -i '/HelloAAL OUTPUT/r errtemp.txt' doc/output/final.dox
   rm errtemp.txt
fi

if  [ -a Hello_CCI_NLB/SW/helloCCInlb ]; then
   timeout 2s Hello_CCI_NLB/SW/helloCCInlb >OUTPUT.txt 2>&1
   temp=`grep -c "Error" OUTPUT.txt`
   if [ $temp -ne 0 ];
   then
      temp=0;
      sed -n 29,40p doc/output/err.txt >>errtemp.txt
      sed -i '/HelloCCINLB OUTPUT/r errtemp.txt' doc/output/final.dox 
      rm errtemp.txt
   else
      awk '{print "///"$0}' OUTPUT.txt >OUTPUT1.txt
      sed -i '/HelloCCINLB OUTPUT/r OUTPUT1.txt' doc/output/final.dox 
      rm OUTPUT.txt OUTPUT1.txt
   fi
else
   sed -n 3,13p doc/output/err.txt >>errtemp.txt
   sed -i '/HelloCCINLB OUTPUT/r errtemp.txt' doc/output/final.dox
   rm errtemp.txt
fi

if  [ -a Hello_SPL_LB/SW/helloSPLlb ]; then
   timeout 2s Hello_SPL_LB/SW/helloSPLlb >OUTPUT.txt 2>&1
   temp=`grep -c "Error" OUTPUT.txt`
   if [ $temp -ne 0 ];
   then
      sed -n 29,40p doc/output/err.txt >>errtemp.txt
      sed -i '/HelloSPLLB OUTPUT/r errtemp.txt' doc/output/final.dox 
      rm errtemp.txt
   else
      awk '{print "///"$0}' OUTPUT.txt >OUTPUT1.txt
      sed -i '/HelloSPLLB OUTPUT/r OUTPUT1.txt' doc/output/final.dox 
      rm OUTPUT.txt OUTPUT1.txt
   fi
else
   sed -n 3,13p doc/output/err.txt >>errtemp.txt
   sed -i '/HelloSPLLB OUTPUT/r errtemp.txt' doc/output/final.dox
   rm errtemp.txt
fi

