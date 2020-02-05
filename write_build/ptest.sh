#!/bin/bash
for ((c=1;c<=32;c*=2))
do
	#./runmc.sh $c
	echo "threads: $c"
	for ((t=1;t<=$c;t++))
	do
		LD_PRELOAD=`jemalloc-config --libdir`/libjemalloc.so.`jemalloc-config --revision` ./memcached_client &> test-${c}-${t}.log &
	done
	wait
	sum=0
	for f in test-${c}-*.log
	do
		runtime=`cat $f | awk {'print $1'}`
		#echo "$runtime"
		sum=`expr $sum + $runtime`
	done
	echo "$sum $c `expr $sum / $c`"
done
