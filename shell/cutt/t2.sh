#!/bin/bash
a=12345
echo ${a:0:${#a}-1}
