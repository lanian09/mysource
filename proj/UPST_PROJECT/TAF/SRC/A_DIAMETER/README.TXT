
HOWTO DIAMETER SIMULATION TEST...

A_SCTP를 컴파일 하기 전에 아래의 라이브러리를 컴파일 한다. 


### DIAMETER 헤더 및 메세지 정의 

../../../COMLIB/SBPARSING/

### SCTP가 사용하는 공유메모리 정의 

../../LIBSRC/SCTPSTACK_LIB/


#
### /IMS/CONF/conf-new.DAT
#

( MN
<K>IP=211.246.101.21
<K>ifrange=IFRANGE_CSCF
Mask=255.255.255.255
) MN

( LPREA
<K>ServerIP=211.246.101.113
<K>ServerPort=0
<K>PROTOCOL=PROTOCOL_SCTP
<K>ifrange=IFRANGE_CSCF
AppCode=S_MSGQ_A_DIAMETER
RANGE=RANGE_CSCF
) LPREA

#
### SIM TEST
#

./dagconvert -i DATA/Location_Info_Transaction.cap -o a.cap -T PCAP:ERF -X 1 -Z 10000 -N 172.31.247.179 -P 2009 -M 2 -C 211.246.101.21 -K 255.255.255.255
./dagconvert -i DATA/diameter1000.cap -o a.cap -T PCAP:ERF -X 1 -Z 10000 -N 172.31.247.179 -P 2009 -M 2 -C 211.246.101.21 -K 255.255.255.255


