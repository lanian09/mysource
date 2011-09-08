#include <svctype.h>

char *ERRTABLE[] = { "Wrong Delimiter, each column of Command line has to be delimited by comma(\',\')",
			"Command type NOT defined, Command types are A(ADD), D(DEL), C(CHG)",
			"Unknown layer, a layer's value has to be one of TCP, UDP or IP",
			"Wring IPv4 Address",
			"Invalid netmask value, a netmask value has to be less than 33",
			NULL
};
