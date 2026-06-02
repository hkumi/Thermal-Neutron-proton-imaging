for i in {1..2}
do
   echo "##############################################################" 
   echo "Computation $i starts"
   echo "##############################################################" 
   ./exe_scinti 1 10 data.txt 25 0 0 >> data/vedi$i.txt
   echo "Computation $i Completed"
   echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" 
done
