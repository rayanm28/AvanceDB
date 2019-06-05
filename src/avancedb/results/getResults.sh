#!/bin/bash


file1=$1
file2=$2
from1=$3
to1=$4

./Merge $file1 $file2
fileName=$(echo $file1 | cut -d'.' -f 1)
mCsvStr="Merged.csv"
fixedStr="Fixed.csv"
mergedFile=$fileName$mCsvStr
fixedFile=$fileName$fixedStr
printf "$mergedFile\n"

for i in `seq $from1 $to1`;
do
      ./fixedSL $i $mergedFile
      ./mkhist $fixedFile
done


