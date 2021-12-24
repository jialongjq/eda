#!/bin/bash

counter=1
wc1=0
wc2=0
wc3=0
wc4=0

while [ $counter -le $5 ]
do

./Game $1 $2 $3 $4 -s $RANDOM -i default.cnf -o game$counter.res &> gameoutput$counter.txt

winnerline=$( tail -2 gameoutput$counter.txt | head -1 )
winner=$( echo $winnerline | cut -d " " -f 3 )


if [ $winner == $1 ]
then
((wc1++))
elif [ $winner == $2 ]
then
((wc2++))
elif [ $winner == $3 ]
then
((wc3++))
elif [ $winner == $4 ]
then
((wc4++))
fi

echo $counter: $winner   TOTAL[$1=$wc1, $2=$wc2, $3=$wc3, $4=$wc4]

((counter++))

done
echo post game results:
echo $1 won $wc1 
echo $2 won $wc2
echo $3 won $wc3
echo $4 won $wc4
echo ------------------------------------------------
rm *.txt *.res
