all:stats base main

stats:compute_stats.cpp
	g++ -o stats -Wall -g -ggdb -O0 compute_stats.cpp
base:baseline.cpp
	g++ -o base -Wall -g -ggdb -O0 baseline.cpp
main:main.cpp
	g++ -o main -Wall -g -ggdb -O0 main.cpp