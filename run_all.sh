#!/bin/bash
make

./build/main data_1111.txt out_1111.txt
./build/main data_2222.txt out_2222.txt
./build/main data_3333.txt out_3333.txt
./build/main data_4444.txt out_4444.txt
./build/main data_5555.txt out_5555.txt

./base data_1111.txt ref_1111.txt
./base data_2222.txt ref_2222.txt
./base data_3333.txt ref_3333.txt
./base data_4444.txt ref_4444.txt
./base data_5555.txt ref_5555.txt

printf "\n  data_1111\n"
printf "my\n"
./stats data_1111.txt out_1111.txt
printf "\nref\n"
./stats data_1111.txt ref_1111.txt

printf "\n  data_2222\n"
printf "my\n"
./stats data_2222.txt out_2222.txt
printf "\nref\n"
./stats data_2222.txt ref_2222.txt

printf "\n  data_3333\n"
printf "my\n"
./stats data_3333.txt out_3333.txt
printf "\nref\n"
./stats data_3333.txt ref_3333.txt

printf "\n  data_4444\n"
printf "my\n"
./stats data_4444.txt out_4444.txt
printf "\nref\n"
./stats data_4444.txt ref_4444.txt

printf "\n  data_5555\n"
printf "my\n"
./stats data_5555.txt out_5555.txt
printf "\nref\n"
./stats data_5555.txt ref_5555.txt

rm out_1111.txt out_2222.txt out_3333.txt out_4444.txt out_5555.txt
rm ref_1111.txt ref_2222.txt ref_3333.txt ref_4444.txt ref_5555.txt