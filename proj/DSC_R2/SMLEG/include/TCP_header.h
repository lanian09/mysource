#ifndef _PROTOCOL_TCP_HEADER_ETHERNET
#define _PROTOCOL_TCP_HEADER_ETHERNET

#pragma pack(1)

#define TRANSPROTOS_MAX    256

typedef struct tagTRANSPORT_PROTOCOLS {
	UCHAR	Protocol;
	TCHAR	szName[20];
	TCHAR	szDescription[MAX_PATH];
} TRANSPORT_PROTOCOLS, *LPTRANSPORT_PROTOCOLS;

#define TRANSPROTOS_MAX2   5

typedef struct tagTRANSPORT_PROTOCOLS2 {
	int	    Protocol;
	TCHAR	szName[20];
	TCHAR	szDescription[MAX_PATH];
} TRANSPORT_PROTOCOLS2, *LPTRANSPORT_PROTOCOLS2;


/* see rfc1700.txt */
TRANSPORT_PROTOCOLS transProtos[TRANSPROTOS_MAX] = {
	{  0, "",            "Reserved"                            },
	{  1, "ICMP",        "Internet Control Message"            },
	{  2, "IGMP",        "Internet Group Management"           },
	{  3, "GGP",         "Gateway-to-Gateway"                  },
	{  4, "IP",          "IP in IP (encasulation)"             },
	{  5, "ST",          "Stream"                              },
	{  6, "TCP",         "Transmission Control"                },
	{  7, "UCL",         "UCL"                                 },
	{  8, "EGP",         "Exterior Gateway Protocol"           },
	{  9, "IGP",         "any private interior gateway"        },
	{ 10, "BBN-RCC-MON", "BBN RCC Monitoring"                  },
	{ 11, "NVP-II",      "Network Voice Protocol"              },
	{ 12, "PUP",         "PUP"                                 },
	{ 13, "ARGUS",       "ARGUS"                               },
	{ 14, "EMCON",       "EMCON"                               },
	{ 15, "XNET",        "Cross Net Debugger"                  },
	{ 16, "CHAOS",       "Chaos"                               },
	{ 17, "UDP",         "User Datagram"                       },
	{ 18, "MUX",         "Multiplexing"                        },
	{ 19, "DCN-MEAS",    "DCN Measurement Subsystems"          },
	{ 20, "HMP",         "Host Monitoring"                     },
	{ 21, "PRM",         "Packet Radio Measurement"            },
	{ 22, "XNS-IDP",     "XEROX NS IDP"                        },
	{ 23, "TRUNK-1",     "Trunk-1"                             },
	{ 24, "TRUNK-2",     "Trunk-2"                             },
	{ 25, "LEAF-1",      "Leaf-1"                              },
	{ 26, "LEAF-2",      "Leaf-2"                              },
	{ 27, "RDP",         "Reliable Data Protocol"              },
	{ 28, "IRTP",        "Internet Reliable Transaction"       },
	{ 29, "ISO-TP4",     "ISO Transport Protocol Class 4"      },
	{ 30, "NETBLT",      "Bulk Data Transfer Protocol"         },
	{ 31, "MFE-NSP",     "MFE Network Services Protocol"       },
	{ 32, "MERIT-INP",   "MERIT Internodal Protocol"           },
	{ 33, "SEP",         "Sequential Exchange Protocol"        },
	{ 34, "3PC",         "Third Party Connect Protocol"        },
	{ 35, "IDPR",        "Inter-Domain Policy Routing Protocol"},
	{ 36, "XTP",         "XTP"                                 },
	{ 37, "DDP",         "Datagram Delivery Protocol"          },
	{ 38, "IDPR-CMTP",   "IDPR Control Message Transport Proto"},
	{ 39, "TP++",        "TP++ Transport Protocol"             },
	{ 40, "IL",          "IL Transport Protocol"               },
	{ 41, "SIP",         "Simple Internet Protocol"            },
	{ 42, "SDRP",        "Source Demand Routing Protocol"      },
	{ 43, "SIP-SR",      "SIP Source Route"                    },
	{ 44, "SIP-FRAG",    "SIP Fragment"                        },
	{ 45, "IDRP",        "Inter-Domain Routing Protocol"       },
	{ 46, "RSVP",        "Reservation Protocol"                },
	{ 47, "GRE",         "General Routing Encapsulation"       },
	{ 48, "MHRP",        "Mobile Host Routing Protocol"        },
	{ 49, "BNA",         "BNA"                                 },
	{ 50, "SIPP-ESP",    "SIPP Encap Security Payload"         },
	{ 51, "SIPP-AH",     "SIPP Authentication Header"          },
	{ 52, "I-NLSP",      "Integrated Net Layer Security"       },
	{ 53, "SWIPE",       "IP with Encryption"                  },
	{ 54, "NHRP",        "NBMA Next Hop Resolution Protocol"   },
	{ 55, "",            "Unassigned"                          },
	{ 56, "",            "Unassigned"                          },
	{ 57, "",            "Unassigned"                          },
	{ 58, "",            "Unassigned"                          },
	{ 59, "",            "Unassigned"                          },
	{ 60, "",            "Unassigned"                          },
	{ 61, "",            "any host internal protocol"          },
	{ 62, "CFTP",        "CFTP"                                },
	{ 63, "",            "any local network"                   },
	{ 64, "SAT-EXPAK",   "SATNET and Backroom EXPAK"           },
	{ 65, "KRYPTOLAN",   "Kryptolan"                           },
	{ 66, "RVD",         "MIT Remote Virtual Disk Protocol"    },
	{ 67, "IPPC",        "Internet Pluribus Packet Core"       },
	{ 68, "",            "any distributed file system"         },
	{ 69, "SAT-MON",     "SATNET Monitoring"                   },
	{ 70, "VISA",        "VISA Protocol"                       },
	{ 71, "IPCV",        "Internet Packet Core Utility"        },
	{ 72, "CPNX",        "Computer Protocol Network Executive" },
	{ 73, "CPHB",        "Computer Protocol Heart Beat"        },
	{ 74, "WSN",         "Wang Span Network"                   },
	{ 75, "PVP",         "Packet Video Protocol"               },
	{ 76, "BR-SAT-MON",  "Backroom SATNET Monitoring"          },
	{ 77, "SUN-ND",      "SUN ND PROTOCOL-Temporary"           },
	{ 78, "WB-MON",      "WIDEBAND Monitoring"                 },
	{ 79, "WB-EXPAK",    "WIDEBAND EXPAK"                      },
	{ 80, "ISO-IP",      "ISO Internet Protocol"               },
	{ 81, "VMTP",        "VMTP"                                },
	{ 82, "SECURE-VMTP", "SECURE-VMTP"                         },
	{ 83, "VINES",       "VINES"                               },
	{ 84, "TTP",         "TTP"                                 },
	{ 85, "NSFNET-IGP",  "NSFNET-IGP"                          },
	{ 86, "DGP",         "Dissimilar Gateway Protocol"         },
	{ 87, "TCF",         "TCF"                                 },
	{ 88, "IGRP",        "IGRP"                                },
	{ 89, "OSPFIGP",     "OSPFIGP"                             },
	{ 90, "Sprite-RPC",  "Sprite RPC Protocol"                 },
	{ 91, "LARP",        "Locus Address Resolution Protocol"   },
	{ 92, "MTP",         "Multicast Transport Protocol"        },
	{ 93, "AX.25",       "AX.25 Frames"                        },
	{ 94, "IPIP",        "IP-within-IP Encapsulation Protocol" },
	{ 95, "MICP",        "Mobile Internetworking Control Pro." },
	{ 96, "SCC-SP",      "Semaphore Communications Sec. Pro."  },
	{ 97, "ETHERIP",     "Ethernet-within-IP Encapsulation"    },
	{ 98, "ENCAP",       "Encapsulation Header"                },
	{ 99, "",            "any private encryption scheme"       },
	{100, "GMTP",        "GMTP"                                },
	{101, "",            "Unassigned"                          },
	{102, "",            "Unassigned"                          },
	{103, "",            "Unassigned"                          },
	{104, "",            "Unassigned"                          },
	{105, "",            "Unassigned"                          },
	{106, "",            "Unassigned"                          },
	{107, "",            "Unassigned"                          },
	{108, "",            "Unassigned"                          },
	{109, "",            "Unassigned"                          },
	{110, "",            "Unassigned"                          },
	{111, "",            "Unassigned"                          },
	{112, "",            "Unassigned"                          },
	{113, "",            "Unassigned"                          },
	{114, "",            "Unassigned"                          },
	{115, "",            "Unassigned"                          },
	{116, "",            "Unassigned"                          },
	{117, "",            "Unassigned"                          },
	{118, "",            "Unassigned"                          },
	{119, "",            "Unassigned"                          },
	{120, "",            "Unassigned"                          },
	{121, "",            "Unassigned"                          },
	{122, "",            "Unassigned"                          },
	{123, "",            "Unassigned"                          },
	{124, "",            "Unassigned"                          },
	{125, "",            "Unassigned"                          },
	{126, "",            "Unassigned"                          },
	{127, "",            "Unassigned"                          },
	{128, "",            "Unassigned"                          },
	{129, "",            "Unassigned"                          },
	{130, "",            "Unassigned"                          },
	{131, "",            "Unassigned"                          },
	{132, "",            "Unassigned"                          },
	{133, "",            "Unassigned"                          },
	{134, "",            "Unassigned"                          },
	{135, "",            "Unassigned"                          },
	{136, "",            "Unassigned"                          },
	{137, "",            "Unassigned"                          },
	{138, "",            "Unassigned"                          },
	{139, "",            "Unassigned"                          },
	{140, "",            "Unassigned"                          },
	{141, "",            "Unassigned"                          },
	{142, "",            "Unassigned"                          },
	{143, "",            "Unassigned"                          },
	{144, "",            "Unassigned"                          },
	{145, "",            "Unassigned"                          },
	{146, "",            "Unassigned"                          },
	{147, "",            "Unassigned"                          },
	{148, "",            "Unassigned"                          },
	{149, "",            "Unassigned"                          },
	{150, "",            "Unassigned"                          },
	{151, "",            "Unassigned"                          },
	{152, "",            "Unassigned"                          },
	{153, "",            "Unassigned"                          },
	{154, "",            "Unassigned"                          },
	{155, "",            "Unassigned"                          },
	{156, "",            "Unassigned"                          },
	{157, "",            "Unassigned"                          },
	{158, "",            "Unassigned"                          },
	{159, "",            "Unassigned"                          },
	{160, "",            "Unassigned"                          },
	{161, "",            "Unassigned"                          },
	{162, "",            "Unassigned"                          },
	{163, "",            "Unassigned"                          },
	{164, "",            "Unassigned"                          },
	{165, "",            "Unassigned"                          },
	{166, "",            "Unassigned"                          },
	{167, "",            "Unassigned"                          },
	{168, "",            "Unassigned"                          },
	{169, "",            "Unassigned"                          },
	{170, "",            "Unassigned"                          },
	{171, "",            "Unassigned"                          },
	{172, "",            "Unassigned"                          },
	{173, "",            "Unassigned"                          },
	{174, "",            "Unassigned"                          },
	{175, "",            "Unassigned"                          },
	{176, "",            "Unassigned"                          },
	{177, "",            "Unassigned"                          },
	{178, "",            "Unassigned"                          },
	{179, "",            "Unassigned"                          },
	{180, "",            "Unassigned"                          },
	{181, "",            "Unassigned"                          },
	{182, "",            "Unassigned"                          },
	{183, "",            "Unassigned"                          },
	{184, "",            "Unassigned"                          },
	{185, "",            "Unassigned"                          },
	{186, "",            "Unassigned"                          },
	{187, "",            "Unassigned"                          },
	{188, "",            "Unassigned"                          },
	{189, "",            "Unassigned"                          },
	{190, "",            "Unassigned"                          },
	{191, "",            "Unassigned"                          },
	{192, "",            "Unassigned"                          },
	{193, "",            "Unassigned"                          },
	{194, "",            "Unassigned"                          },
	{195, "",            "Unassigned"                          },
	{196, "",            "Unassigned"                          },
	{197, "",            "Unassigned"                          },
	{198, "",            "Unassigned"                          },
	{199, "",            "Unassigned"                          },
	{200, "",            "Unassigned"                          },
	{201, "",            "Unassigned"                          },
	{202, "",            "Unassigned"                          },
	{203, "",            "Unassigned"                          },
	{204, "",            "Unassigned"                          },
	{205, "",            "Unassigned"                          },
	{206, "",            "Unassigned"                          },
	{207, "",            "Unassigned"                          },
	{208, "",            "Unassigned"                          },
	{209, "",            "Unassigned"                          },
	{210, "",            "Unassigned"                          },
	{211, "",            "Unassigned"                          },
	{212, "",            "Unassigned"                          },
	{213, "",            "Unassigned"                          },
	{214, "",            "Unassigned"                          },
	{215, "",            "Unassigned"                          },
	{216, "",            "Unassigned"                          },
	{217, "",            "Unassigned"                          },
	{218, "",            "Unassigned"                          },
	{219, "",            "Unassigned"                          },
	{220, "",            "Unassigned"                          },
	{221, "",            "Unassigned"                          },
	{222, "",            "Unassigned"                          },
	{223, "",            "Unassigned"                          },
	{224, "",            "Unassigned"                          },
	{225, "",            "Unassigned"                          },
	{226, "",            "Unassigned"                          },
	{227, "",            "Unassigned"                          },
	{228, "",            "Unassigned"                          },
	{229, "",            "Unassigned"                          },
	{230, "",            "Unassigned"                          },
	{231, "",            "Unassigned"                          },
	{232, "",            "Unassigned"                          },
	{233, "",            "Unassigned"                          },
	{234, "",            "Unassigned"                          },
	{235, "",            "Unassigned"                          },
	{236, "",            "Unassigned"                          },
	{237, "",            "Unassigned"                          },
	{238, "",            "Unassigned"                          },
	{239, "",            "Unassigned"                          },
	{240, "",            "Unassigned"                          },
	{241, "",            "Unassigned"                          },
	{242, "",            "Unassigned"                          },
	{243, "",            "Unassigned"                          },
	{244, "",            "Unassigned"                          },
	{245, "",            "Unassigned"                          },
	{246, "",            "Unassigned"                          },
	{247, "",            "Unassigned"                          },
	{248, "",            "Unassigned"                          },
	{249, "",            "Unassigned"                          },
	{250, "",            "Unassigned"                          },
	{251, "",            "Unassigned"                          },
	{252, "",            "Unassigned"                          },
	{253, "",            "Unassigned"                          },
	{254, "",            "Unassigned"                          },
	{255, "",            "Reserved"                            },
};

TRANSPORT_PROTOCOLS2 transProtos2[TRANSPROTOS_MAX2] = {
	{    1, "All Protocols", "All Protocols"     },
	{    1, "A11",           "A11"               },
	{ 6100, "RADIUS",        "RADIUS"            },
	{ 6200, "MIP",           "MIP"               },
	{ 8101, "GRE",           "GRE"               }
};

#define MAX_ICMP_TYPE 16
struct _icmp_type {
	UCHAR Type;
	char Desc[30];
} icmp_type[MAX_ICMP_TYPE] =
	{
		{  0, "echo reply"             },
		{  3, "destination unreachable"},
		{  4, "source quench"          },
		{  5, "redirect"               },
		{  6, "alternate host address" },
		{  8, "echo request"           },
		{  9, "router advertisement"   },
		{ 10, "router selection"       },
		{ 11, "time exceeded"          },
		{ 12, "parameter problem"      },
		{ 13, "timestamp"              },
		{ 14, "timestamp reply"        },
		{ 15, "information request"    },
		{ 16, "information reply"      },
		{ 17, "address mask request"   },
		{ 18, "address mask reply"     },
	};

#define MAX_ICMP_03_CODE 13
struct _icmp_03_code {
	UCHAR Code;
	char  Desc[80];
} icmp_03_code[MAX_ICMP_03_CODE] =
	{
		{ 0x00, "net unreachable"                                                      },
		{ 0x01, "host unreachable"                                                     },
		{ 0x02, "protocol unreachable"                                                 },
		{ 0x03, "port unreachable"                                                     },
		{ 0x04, "fragmentation needed and don't fragment was Set"                      },
		{ 0x05, "source route failed"                                                  },
		{ 0x06, "destination network unknown"                                          },
		{ 0x07, "destination host unknown"                                             },
		{ 0x08, "source host isolated"                                                 },
		{ 0x09, "communication with destination network is administratively prohibited"},
		{ 0x0a, "communication with destination host is administratively prohibited"   },
		{ 0x0b, "destination network unreachable for type of service"                  },
		{ 0x0c, "destination host unreachable for type of service"                     }
	};

#define MAX_ICMP_05_CODE 4
struct _icmp_05_code {
	UCHAR Code;
	char  Desc[60];
} icmp_05_code[MAX_ICMP_05_CODE] =
	{
		{ 0, "redirect datagram for the network for subnet"         },
		{ 1, "redirect datagram for the host"                       },
		{ 2, "redirect datagram for the type of service and network"},
		{ 3, "redirect datagram for the type of service and host"   }
	};

#define MAX_ICMP_11_CODE 2
struct _icmp_11_code {
	UCHAR Code;
	char Desc[40];
} icmp_11_code[MAX_ICMP_11_CODE] =
	{
		{ 0, "time to live exceeded in transit" },
		{ 1, "fragment reassembly time exceeded"}
	};

#define MAX_ICMP_12_CODE 2
struct _icmp_12_code {
	UCHAR Code;
	char Desc[40];
} icmp_12_code[MAX_ICMP_12_CODE] =
	{
		{ 0, "pointer indicates the error"      },
		{ 1, "fragment reassembly time exceeded"}
	};


/* TCP Header, RFC793 */
typedef	struct	_TCP_RHDR {
	UCHAR	Source[2];
	UCHAR	Destination[2];
	UCHAR	Seq[4];
	UCHAR	Ack[4];
#if LINUX
	UCHAR	Rsvd0:4;
	UCHAR	Offset:4;

	UCHAR	Flags:6;
	UCHAR	Rsvd1:2;
#else
	UCHAR	Offset:4;
	UCHAR	Rsvd0:4;

	UCHAR	Rsvd1:2;
	UCHAR	Flags:6;
#endif
	UCHAR	Window[2];
	UCHAR	Checksum[2];
	UCHAR	UrgPoint[2];
	UCHAR	Data[1];
} TCP_RHDR, *PTCP_RHDR;

typedef struct _st_tcpoption {
	UCHAR   Kind;
	short   Length;
	UCHAR   Desc[40];
} st_tcpoption;

#define MAX_TCPOPTION 26

#define	TCP_FLAG_FIN	0x01
#define	TCP_FLAG_SYN	0x02
#define	TCP_FLAG_RST	0x04
#define	TCP_FLAG_PSH	0x08
#define	TCP_FLAG_ACK	0x10
#define	TCP_FLAG_URG	0x20


/* NBT Session : 2001.03.05 by joonhk */
typedef struct _NBT_S {
	UCHAR Type;
	UCHAR Flags;
	UCHAR Length[2];
} NBT_S, *PNBT_S;

st_tcpoption TcpOption[MAX_TCPOPTION] = {
	{  0,  1, "End of option list"                 },
	{  1,  1, "No operation"                       },
	{  2,  4, "Maximum Segment Size"               },
	{  3,  3, "Window scale factor"                },
	{  4,  2, "SACK permitted"                     },
	{  5, -1, "SACK"                               },	/* length : variable */
	{  6,  6, "Echo"                               },
	{  7,  6, "Echo reply"                         },
	{  8, 10, "Timestamp"                          },
	{  9,  2, "Partial Order Connection Permitted" },
	{ 10,  3, "Partial Order Service Profile"      },
	{ 11,  6, "Connection Count"                   },
	{ 12,  6, "Connection Count NEW"               },
	{ 13,  6, "Connection Count ECHO"              },
	{ 14,  3, "TCP Alternate Checksum Request"     },
	{ 15, -1, "TCP Alternate Checksum Data"        },	/* length : variable */
	{ 16,  1, "Skeeter"                            },	/* length ? */
	{ 17,  1, "Bubba"                              },	/* length ? */
	{ 18,  3, "Trailer Checksum Option"            },
	{ 19, 18, "MD5 sigature"                       },
	{ 20,  1, "SCPS Capabilities"                  },	/* length ? */
	{ 21,  1, "Selective Negative Acknowledgements"},	/* length ? */
	{ 22,  1, "Record Boundaries"                  },	/* length ? */
	{ 23,  1, "Corruption experienced"             },	/* length ? */
	{ 24,  1, "SNAP"                               },	/* length ? */
	{ 25,  1, "RDMA"                               }	/* length ? */
	};

#pragma pack(0)
#endif

