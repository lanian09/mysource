1. result.c ������ ��� 
gcc -g -o result -lsocket  -lnsl result.c

2. pcube_subs ���� ���
./pcube_subs -d PCube_SM_Repository -u pcube -o pcube s [Start] [End] 1

ex) 
./pcube_subs -d PCube_SM_Repository -u pcube -o pcube s 1 100 1

3. pcube_subs ��  result ��  Ȯ��  ��� 

./pcube_subs -d PCube_SM_Repository -u pcube -o pcube s 1 100 1 > result.txt
./result


./pcube_subs -d PCube_SM_Repository -u pcube -o pcube -f test.txt s 1 100 1

