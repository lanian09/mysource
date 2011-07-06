TL=`cat stat.log | wc -l`
CL=1

while [ 1 ]
do
	#CURR=`cat stat.log | head -$((CL+1)) |tail -1 | awk '{print $3" "$4}'`
	PROC=`cat stat.log | head -$((CL+1)) |tail -1 | awk '{print $3}'`
	CURR=`cat stat.log | head -$((CL+1)) |tail -1 | awk '{print $3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$11" "$12" "$13" "$14" "$15" "$16" "$17" "$18" "$19" "$20" "$21" "$22" "$23" "$24" "$25" "$26" "$27" "$28" "$29" "$30" "$31" "$32" "$33" "$34" "$35" "$36" "$37" "$38" "$39" "$40" "$41" "$42" "$43}'` 
	echo $CURR > $PROC
	CL=$((CL+1))

	if [ $CL -eq $TL ]
	then
		exit
	fi
done

