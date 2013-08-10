#/usr/bin/sh
/DSC/SCRIPT/dis-sce.ex $1 | /usr/bin/tr "[:lower:]" "[:upper:]"  | /usr/bin/awk '
{ 
	idx=index($1,"SYSTEM"); 
	if(idx>0) { 
		printf "%s ", $8; 
	} 
	else 
	{	idx=index($5,"ACTIVE"); 
		if(idx>0) printf "%s ",  $8; 
		idx=index($5,"FAILURE"); 
		if(idx>0) printf "%s ",  $7; 
	} 
} 
END { 
	printf "\n"; 
}'
