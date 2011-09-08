#
# lex 설명 
#
Filename: 
	re_lex.l
	
	19 Line : SELECT=OK 구문을 원하는 구문으로 변경 후 재컴파일 

How to Testing Compile :

	1. flex -P parsedata -i re_lex.l
	2. gcc -g -Wall -DTEST -L../../lib -I../../include -I./  -DPRINT -DYY_NO_UNPUT lex.parsedata.c
	3. Create a.out

