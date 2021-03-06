/*
 * Copyright (c) 2002-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dagtoken.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAGTOKEN_H
#define DAGTOKEN_H
enum
{
	T_ERROR = -1,

	T_ATM = 1,
	T_ATM_ADD_HEC,
	T_ATM_FIX_HEC,
	T_ATM_LCELL,
	T_ATM_NCELLS,
	T_ATM_PASS_HEC,
	T_ATM_PASS_IDLE,
	T_ATM_PLOAD_SCRAMBLE,
	T_DEFAULT,
	T_ETH,
	T_ETH_10,
	T_ETH_100,
	T_ETH_1000_STATUS,
	T_ETH_1000,
	T_ETH_AUTO,
	T_ETH_NIC,
	T_ETH_WAN,
	T_GPP_ALIGN64,
	T_GPP_ENABLE_A,
	T_GPP_ENABLE_B,
	T_GPP_SLEN,
	T_GPP_VARLEN,
	T_INTERFACE_SWAP,
	T_LINK_ADM,
	T_LINK_AMI,
	T_LINK_ASYNC1,
	T_LINK_B8ZS,
	T_LINK_BIT_SYNC,
	T_LINK_BYTE_SYNC1,
	T_LINK_BYTE_SYNC2,
	T_LINK_CLEAR,
	T_LINK_CORE_ON,
	T_LINK_CRC,
	T_LINK_DCR,
	T_LINK_DISCARD,
	T_LINK_E1_CRC,
	T_LINK_E1_UNFRAMED,
	T_LINK_E1,
	T_LINK_EQL,
	T_LINK_FCL,
	T_LINK_HGAIN,
	T_LINK_LASER,
	T_LINK_LSEQL,
	T_LINK_LSFCL,
	T_LINK_LT0,
	T_LINK_LT1,
	T_LINK_M23,
	T_LINK_MODE,
	T_LINK_OC1,
	T_LINK_OC3,
	T_LINK_OC12,
	T_LINK_OC48,
	T_LINK_OC192,
	T_LINK_RESET,
	T_LINK_RX_TERMEXT,
	T_LINK_SFPPWR,
	T_LINK_T1_ESF,
	T_LINK_T1_SF,
	T_LINK_T1_UNFRAMED,
	T_LINK_TERM75,
	T_LINK_TERM100,
	T_LINK_TERM120,
	T_LINK_TERMEXT,
	T_LINK_TU11,
	T_LINK_TU12,
	T_LINK_TUNOMAP,
	T_LINK_TX_TERMEXT,
	T_LINK_VC3,
	T_LINK_VC4,
	T_LINK_VC4C,
	T_LINK_WHICH,
	T_MASTER,
	T_MUX_CONFIG,
	T_MUX_RXMERGE,
	T_MUX_RXSPLIT,
	T_PBM_MEMCONFIG,
	T_PBM_OVERLAP,
	T_PBM_RXONLY,
	T_PBM_RXTX,
	T_PBM_TXONLY,
	T_POS_CRC16,
	T_POS_CRC32,
	T_POS_CRCSTRIP,
	T_POS_MAXCHECK,
	T_POS_MAXLEN,
	T_POS_MINCHECK,
	T_POS_MINLEN,
	T_POS_RXPKTS,
	T_POS_SCRAMBLE,
	T_POS_TXPKTS,
	T_POS,
	T_RAW,
	T_SAR_ENABLE,
	T_SAR_NNI,
	T_SAR_UNI,
	T_SAR_CELLS,
	T_SAR_FRAMES,
	T_SLAVE,
	T_SONET_SCRAMBLE,

	T_MAX
};

#endif /* DAGTOKEN_H */
