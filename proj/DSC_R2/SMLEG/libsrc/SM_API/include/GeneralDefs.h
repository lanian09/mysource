#ifndef GENERAL_DEFS
#define GENERAL_DEFS

/**
 * used to specify the mapping type in logon methods.
 * the IP_RANGE works for IP ranges and IP addresses
 */
OSAL_DllExport typedef enum
{
	IP_RANGE, 
	VLAN
}MappingType;


/* ********* return code structure internals ********* */

OSAL_DllExport typedef enum
{
	ERROR_CODE_UNKNOWN                       = 0,
	ERROR_CODE_OBJECT_NOT_FOUND              = 1,
	ERROR_CODE_OPERATION_NOT_FOUND           = 2,
	ERROR_CODE_ATTRIBUTE_NOT_FOUND           = 3,
 
	ERROR_CODE_NULL_POINTER                  = 33000,
	ERROR_CODE_CLASS_CAST                    = 33001,
	ERROR_CODE_RUNTIME                       = 33002,
	ERROR_CODE_OUT_OF_MEMORY                 = 33003,
	ERROR_CODE_CLASS_NOT_FOUND               = 33004,
	ERROR_CODE_ILLEGAL_STATE                 = 33005,
	ERROR_CODE_ILLEGAL_ARGUMENT              = 33006,
	ERROR_CODE_ARRAY_ACCESS                  = 33007,
	ERROR_CODE_NUMBER_FORMAT                 = 33008,
 
	ERROR_CODE_DOMAIN_NOT_FOUND              = 100000,
	ERROR_CODE_NOT_A_SUSBCRIBER_DOMAIN       = 100001,
	ERROR_CODE_SE_ERROR                      = 100002,
	ERROR_CODE_SUSBSCRIBER_ALREADY_EXISTS    = 100003,
	ERROR_CODE_SUBSCRIBER_DOES_NOT_EXIST     = 100004,
	ERROR_CODE_SUBSCRIBER_DOMAIN_ASSOCIATION = 100005,
	ERROR_CODE_SUBSCRIBER_MAPPING_CONGESTION = 100006,
	ERROR_CODE_BAD_SUBSCRIBER_MAPPING        = 100007,
	ERROR_CODE_ILLEGAL_SUBSCRIBER_NAME       = 100008,
	
	ERROR_CODE_CLIENT_OUT_OF_THREADS         = 8000000,
	ERROR_CODE_CLIENT_INTERNAL_ERROR         = 8000001,
	ERROR_CODE_CLIENT_OPERATION_TIMEOUT      = 8000002
}ErrorCodeType;

OSAL_DllExport typedef struct ErrorCode_t
{
	ErrorCodeType type; /* type of the error see enumeration */
	char* message;		/* error message */
	char* stackTrace;	 		/* */
}ErrorCode;

OSAL_DllExport typedef enum
{
	NULL_T,				/* no union in the ReturnCode */
	VOID_T,	 			/* no union in the ReturnCode */
	BOOL_T,				
	INT8_T,             /* use the ReturnCode short union */
	INT16_T,
	INT32_T,
	INT64_T,
	STRING_T,
	BOOL_ARRAY_T, 		
	INT8_ARRAY_T,
	INT16_ARRAY_T,
	INT32_ARRAY_T,
	INT64_ARRAY_T,
	STRING_ARRAY_T,
	ERROR_T,			/* use the ReturnCode errorCode union */
	OBJECT_ARRAY_T		/* use the ReturnCode objectArray union - for future use */
}ReturnCodeType;

OSAL_DllExport typedef struct ReturnCode_t
{
	ReturnCodeType type;
	int size;		/* number of elements in the union element (for example: stringVal will have size=1) */
	union {			/* use union value according to the type value */
		bool boolVal;
		Uint8 byteVal;
		short shortVal;
		int intVal;		
		long longVal;
		char* stringVal;
		bool* boolArrayVal;
		short* shortArrayVal;
		Uint8* byteArrayVal;
		int* intArrayVal;
		long* longArrayVal;	
		char** stringArrayVal;
		ErrorCode* errorCode;
		struct ReturnCode_t** objectArray;
	} u;	
}ReturnCode;

OSAL_DllExport typedef ReturnCode Object;
OSAL_DllExport typedef ReturnCodeType ObjectType;

/* *********** utility function to handle ReturnCode structures *********** */
OSAL_EXTERN OSAL_DllExport void freeReturnCode(ReturnCode* argReturnCode);

OSAL_EXTERN OSAL_DllExport void printReturnCode(ReturnCode* argReturnCode);

OSAL_EXTERN OSAL_DllExport bool isReturnCodeError(ReturnCode* argReturnCode);

OSAL_EXTERN OSAL_DllExport int sprintfReturnCode(char *argToFill, ReturnCode* argReturnCode);


/* *********** callback function definition *********** */
/* connection down callback specification */
typedef void (*ConnectionIsDownCallBackFunc)();

/* connection up callback specification */
typedef void (*ConnectionIsUpCallBackFunc)();

/* operation failure callback specification */
typedef void (*OperationFailCallBackFunc)(Uint32 argHandle, ReturnCode *argReturnCode);

/* operation success callback specification */
typedef void (*OperationSuccessCallBackFunc)(Uint32 argHandle, ReturnCode *argReturnCode);


/* *********** API constants ************** */
enum
{
	DEFAULT_BUFFER_SIZE = 2000000
};

// in seconds
enum
{
	NO_RECONNECT = 0,
	DEFAULT_KEEP_ALIVE_DURATION = 10,
	DEFAULT_CONNECTION_TIMEOUT = 20
};

#endif
