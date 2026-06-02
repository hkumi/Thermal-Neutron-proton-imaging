#!/bin/bash

for numberx in {1..8}
do
  for contax in {1..1000}
  do
    echo  "$contax of $numberx "
    n=$(($numberx * 5))
    nome="colli"
    tutto=$nome$n
    ./exe_scinti 1 $n $tutto 25 0 0
  done
done

