rm -f *.o rlencode rldecode

make

echo "Please wait for at least a few minutes for your tests to be finished."
echo "The total waiting time depends on how fast your program runs."
echo "There are totally 40 tests (1 means it's passed, 0 means failed)."
echo -ne "Your test results are: "

rm -f *.out

for ((i=1;i<9;++i))
do
  ./rlencode ~cs9319/a2/simple$i.bwt > test$i-a.out 
  ./rlencode ~cs9319/a2/simple$i.bwt test$i-b.out
  touch test$i-b.out
  ls -l test$i-b.out |cut -d' ' -f5 > test$i-c.out
  ./rldecode test$i-b.out > test$i-d.out
  ./rldecode test$i-b.out test$i-e.out
  touch test$i-e.out
done


for ((i=1;i<9;++i))
do
  correct0=`eval diff -q ~cs9319/a2/simple${i}.bwt test${i}-e.out`
  if [ -z "$correct0" ]; then
    echo -ne "1" 

    correct=`eval diff -q ~cs9319/a1/autotest/test${i}-a.out test${i}-a.out`
    if [ -z "$correct" ]; then
      echo -ne "1" 
    else
      echo -ne "0" 
    fi
    correct=`eval diff -q ~cs9319/a1/autotest/test${i}-b.out test${i}-b.out`
    if [ -z "$correct" ]; then
      echo -ne "1" 
    else
      echo -ne "0" 
    fi
    correct=`eval diff -q ~cs9319/a1/autotest/test${i}-c.out test${i}-c.out`
    if [ -z "$correct" ]; then
      echo -ne "1" 
    else
      echo -ne "0" 
    fi
    correct=`eval diff -q ~cs9319/a1/autotest/test${i}-d.out test${i}-d.out`
    if [ -z "$correct" ]; then
      echo -ne "1" 
    else
      echo -ne "0" 
    fi

  else
    echo -ne "00000"
  fi
done
echo " "  

rm -f *.out
