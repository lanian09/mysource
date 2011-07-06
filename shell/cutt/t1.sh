#!/bin/bash
a=12345
echo $a | sed -e s/[0-9]$//
