#!/bin/bash

for numberx in {-5..5}
do
  for contax in {1..2}
  do
    echo  "$contax of $numberx "
    n=$(($numberx * 800))
    nome="simu"
    tutto=$nome$n
    ./exe_scinti 1 15 $tutto 25 $n 0
  done
done

