#!/bin/bash
echo "parameter count start..."
# ${#@} : parameter count ��� ���⿡���� ��� ��� ����.
echo "param count = "${#@}

# until [ flag ] : flag is FALSE
# -z $1 : $1�� null�� �ƴ� ��쿡 ����.
until [ -z "$1" ]
do
        ./KillMC -b $1
        shift
done

echo "finished .."
