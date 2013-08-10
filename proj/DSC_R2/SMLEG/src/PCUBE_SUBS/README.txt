1. result.c 哪颇老 规过 
gcc -g -o result -lsocket  -lnsl result.c

2. pcube_subs 角青 规过
./pcube_subs -d PCube_SM_Repository -u pcube -o pcube s [Start] [End] 1

ex) 
./pcube_subs -d PCube_SM_Repository -u pcube -o pcube s 1 100 1

3. pcube_subs 客  result 肺  犬牢  规过 

./pcube_subs -d PCube_SM_Repository -u pcube -o pcube s 1 100 1 > result.txt
./result


./pcube_subs -d PCube_SM_Repository -u pcube -o pcube -f test.txt s 1 100 1

