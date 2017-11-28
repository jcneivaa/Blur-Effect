#!/bin/bash
shopt -s remedialguns
echo 'compiling...'
g++ -fopenmp bluromp.c -o blur-effect `pkg-config opencv --cflags --libs` -pthread
#run exec
echo 'running...'

rm b_*
echo "results:">results.txt

for ((thread = 1; thread<=16; thread*=2))
do
	for((kernel = 3; kernel<= 17; kernel+=2))
	do
		for file in {test720.jpg,paisaje-1080.jpg,transformers4k.jpg};
		do
			if [[ $file = b_* ]]
			then
				continue
			fi
			echo "run with $thread threads with a kernel of size $kernel on the file $file:">>results.txt
			echo "run with $thread threads with a kernel of size $kernel on the file $file"
			(time ./blur-effect $file $thread $kernel)  >>results.txt 2>&1
			echo "">>results.txt

		done
	done
done
