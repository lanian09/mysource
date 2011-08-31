#!/usr/bin/python

import re

proc = {}
cifo = []
gifo = []
write_matrix = []
read_matrix = []

uiShmKey = 10314
uiHeadRoomSize = 5242880

b_cellCnt = 1000000
cellSize = 8
wBufCnt = 1
b_wSemFlag = 0
b_rSemFlag = 0

def getCellCnt(value):
	p_cellCnt = re.compile('cellCnt(\s*)=(\s*)(\d+)')
	a = p_cellCnt.search(value)
	if a:
		return a.group(3)
	else:
		return b_cellCnt

def getWSemFlag(value):
	p_wSemFlag = re.compile('wSemFlag(\s*)=(\s*)(\d+)')
	a = p_wSemFlag.search(value)
	if a:
		return a.group(3)
	else:
		return b_wSemFlag

def getRSemFlag(value):
	p_rSemFlag = re.compile('rSemFlag(\s*)=(\s*)(\d+)')
	a = p_rSemFlag.search(value)
	if a:
		return a.group(3)
	else:
		return b_rSemFlag

def readProcessList(path):
	p = re.compile('(\S+)(\s*)=(\s*)(\d+)')
	f = open(path, 'r')
	for s in f:
		a = p.search(s)
		if a:
			proc[a.group(1)] = a.group(4)
	f.close()

def readFlowList(path):
	chid = 0
	grid = 0
	wSemKey = 20000
	rSemKey = 10000
	p = re.compile('([0-9a-zA-Z=_(), \t]+)(\s*)->(\s*)(\S+)')
	f = open(path, 'r')
	for s in f:
		a = p.search(s)
		if a:
			temp_write = a.group(1).split(',')
			rBufCnt = len(temp_write)
			read_process = a.group(4)
			sub_p = re.compile('([0-9a-zA-Z_]+)')
			gifo_str = "%d/" % grid
			idx = 0
			for x in temp_write:
				cellCnt = getCellCnt(x)
				wSemFlag = getWSemFlag(x)
				rSemFlag = getRSemFlag(x)
				sub_a = sub_p.search(x)
				write_process = sub_a.group(1)
#				print write_process + "\t" + read_process
				str = "%s/%s/%s/%s/%s/%s/%s/%s/%s/\t# %s - %s #" % (chid, cellCnt, cellSize, wBufCnt, rBufCnt, wSemFlag, wSemKey, rSemFlag, rSemKey, write_process, read_process)
#				print str
				cifo.insert(chid, str)
				if idx == rBufCnt - 1:
					gifo_str = gifo_str + "%d/\t# %s #" % (chid, read_process)
				else:
					gifo_str = gifo_str + "%d," % chid
				write_matrix.append("%s/%s/%s/\t # %s/%s/ #" % (proc[write_process], proc[read_process], chid, write_process, read_process))
				chid += 1
				wSemKey += 1
				rSemKey += 1
				idx += 1
#			print gifo_str
			gifo.insert(grid, gifo_str)
			read_matrix.append("%s/%s/\t # %s #" % (proc[read_process], grid, read_process))
			grid += 1
	f.close()

def printCIFO(path):
	f = open(path, 'w')
	f.write("#### st_CIFOCONF setting ####\n")
	f.write("uiShmKey = %s\t# INC/ipaf_define.h S_SSHM_CIFO_MEM 0x284A #\n" % uiShmKey)
	f.write("uiHeadRoomSize = %s\t# gifo size #\n" % uiHeadRoomSize)
	f.write("\n")
	f.write("#### st_CHANCONF setting ####\n")
	f.write("#chId/cellCnt/cellSize/wBufCnt/rBufCnt/wSemflag/wSemKey/rSemFlag/rSemKey/#\n")
	for a in cifo:
		f.write(a + "\n")	
	f.close()

def printGIFO(path):
	f = open(path, 'w')
	f.write("#### group setting ####\n")
	f.write("#grId/chId(, chId)/#\n")
	for a in gifo:
		f.write(a + "\n")
	f.write("END_GROUP#\n")
	f.write("#### write matrix setting ####\n")
	f.write("#wProcSeq/rProcSeq/chId/#\n")
	for b in write_matrix:
		f.write(b + "\n")
	f.write("END_WRITE_MATRIX#\n")
	f.write("#### read matrix setting ####\n")
	f.write("#procSeq/grId/#\n")
	for c in read_matrix:
		f.write(c + "\n")
	f.write("END_READ_MATRIX#\n")
	f.close()

readProcessList('./process_list.conf')
#print proc
readFlowList('./flow_list.conf')
#print cifo
#print gifo
#print write_matrix
#print read_matrix
printCIFO('./cifo.conf')
printGIFO('./gifo.conf')
