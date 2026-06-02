#!/bin/bash

for contax in {1..1000}
do
  R=$(($RANDOM%8000-4000))
  C=`echo "$R"| bc -l`
  echo  "$contax of 1000 $C posizione "
  ./exe_scinti 1 15 random15 25 $C 0
done

