#!/bin/bash
echo "parameter count start..."
# ${#@} : parameter count 사실 여기에서는 없어도 상관 없다.
echo "param count = "${#@}

# until [ flag ] : flag is FALSE
# -z $1 : $1이 null이 아닌 경우에 수행.
until [ -z "$1" ]
do
        ./KillMC -b $1
        shift
done

echo "finished .."
