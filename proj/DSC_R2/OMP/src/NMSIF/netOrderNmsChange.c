#include <sys/types.h>
#include <netinet/in.h>
#include "nmsif.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

void PKTHEAD_N2H(PktHead *pkthead);
void PKTHEAD_H2N(PktHead *pkthead);
void PACKET1_N2H(Packet1 *pk1);
void PACKET1_H2N(Packet1 *pk1);
void PACKET2_N2H(Packet2 *pk2);
void PACKET2_H2N(Packet2 *pk2);
void PACKET3_H2N(Packet3 *pk3);
void PACKET3_H2N(Packet3 *pk3);

//##########################################################################

void PKTHEAD_N2H(PktHead *pkthead)
{
	unsigned int p;
	memcpy(&p, &pkthead->chd, sizeof(pkthead->chd));
	p =  ntohl(p);
	memcpy(&pkthead->chd,&p, sizeof(pkthead->chd));
	
	pkthead->prmt = ntohs(pkthead->prmt);
	pkthead->len = ntohs(pkthead->len);
}

void PKTHEAD_H2N(PktHead *pkthead)
{
	unsigned int p;
	memcpy(&p, &pkthead->chd, sizeof(pkthead->chd));
	p =  htonl(p);
	memcpy(&pkthead->chd,&p, sizeof(pkthead->chd));
	pkthead->prmt = htons(pkthead->prmt);
	pkthead->len = htons(pkthead->len);
}

void PACKET1_N2H(Packet1 *pk1)
{
	PKTHEAD_N2H((PktHead*)&pk1->hdr);
}

void PACKET1_H2N(Packet1 *pk1)
{
	PKTHEAD_H2N((PktHead*)&pk1->hdr);
}

void PACKET2_N2H(Packet2 *pk2)
{
	PKTHEAD_N2H((PktHead*)&pk2->hdr);
	pk2->msgtype = ntohl(pk2->msgtype);
}

void PACKET2_H2N(Packet2 *pk2)
{
	PKTHEAD_H2N((PktHead*)&pk2->hdr);
	pk2->msgtype = htonl(pk2->msgtype);
}

void PACKET3_N2H(Packet3 *pk3)
{
	PKTHEAD_N2H((PktHead*)&pk3->hdr);
	pk3->msgtype = ntohl(pk3->msgtype);
	pk3->serno 	 = ntohs(pk3->serno);
	pk3->attrcnt = ntohs(pk3->attrcnt);
}

void PACKET3_H2N(Packet3 *pk3)
{
	PKTHEAD_H2N((PktHead*)&pk3->hdr);
	pk3->msgtype = htonl(pk3->msgtype);
	pk3->serno 	 = htons(pk3->serno);
	pk3->attrcnt = htons(pk3->attrcnt);
}

#ifdef _TEST_
int main()
{
	return 0;
}
#endif 
