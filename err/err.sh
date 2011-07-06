no=1
while [ 1 ]
do
	perror $no
	#echo $no
	no=$((no+1))
	if [ $((no % 100)) -eq 0 ]
	then
		sleep 3
		echo "Sleep $no"
	fi
done
