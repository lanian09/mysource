#/usr/bin/sh
/DSC/SCRIPT/dis-scm.ex $1 | /usr/bin/tr "[:lower:]" "[:upper:]"  | /usr/bin/egrep '^PS|^FT0' | sed 's/OK/1/g' | /usr/bin/awk '
{ 
    printf "%s ", $2;
} 
END { 
    printf "\n"; 
}'
