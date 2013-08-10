#ifndef _SM_API_NON_BLOCKING_C_H
#define _SM_API_NON_BLOCKING_C_H

#include "Common.h"
#include "GeneralDefs.h"

/**
 * this API supports non-blocking operations that are performed on the SM.
 * the API supports one connection per SMNB_HANDLE, multiple SMNB_HANDLE's should be used to work 
 * with several SMs.
 * when using the API the user must first call the SMNB_init() method.
 * the user should implement the connection-down callback to be able to identify
 * a connection failure and reconnect to the SM. 
 * the user should implement the operation-fail callback to be able to support 
 * failure handling.
 * each function in the non-blocking API returns a unique handle of the invoked 
 * operation. this handle can be used to correlate the invoked operation to the 
 * result returned to the reply handling functions. if the handle has a negative 
 * value it means there was an error in inserting the operation to the operation 
 * buffer, and the user should define a bigger buffer in the init function.
 * 
 * please read the API methods thoroughly before integratin the API into your
 * system. 
 * 
 * @author YoavG
 */
typedef void* SMNB_HANDLE;

/**
 * initializes the API internal tasks and objects.
 * this method must be called before any other method
 * of the API is called.
 * 
 * @param argThreadPriority
 *               internal thread priority value (optional)
 * @param argBufferSize
 *               API internal operation buffer size (optional)
 * @param argKeepAliveDuration
 *               duration in seconds between keepalive messages sent
 *               between the client and the server  (optional)
 * @param argConnectionTimeout
 *               the time in seconds till an idle connectin must be closed (optional)
 * @return non-NULL API handle if the initialization succeded
 */
OSAL_EXTERN OSAL_DllExport SMNB_HANDLE SMNB_init(int argThreadPriority,
                                         Uint32 argBufferSize,
                                         Uint32 argKeepAliveDuration,
										 Uint32 argConnectionTimeout);

/**
 * sets the reconnection timeout
 * when the connection to the SM is down the API will try to
 * re-establish the connection after reconnection timeout
 * 
 * this function should be called after init
 * if not called, the API will not reconnect at all
 * 
 * @param reconnectionTimeout
 *               the timeout in seconds
 * @param argApiHandle
 *               handle to the non-blocking API
 */
OSAL_EXTERN OSAL_DllExport void SMNB_setReconnectTimeout(SMNB_HANDLE argApiHandle, Uint32 reconnectTimeout);

/**
 * set the name prefix used to expose to the peer who is this client
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argName
 */
OSAL_EXTERN OSAL_DllExport void SMNB_setName(SMNB_HANDLE argApiHandle,char *argName);

/**
 * release the resources allocated for the specified handle.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @return 
 */
OSAL_EXTERN OSAL_DllExport void SMNB_release(SMNB_HANDLE argApiHandle);

/**
 * set the destination to send the events to.
 * (SM address) and connect to it using the
 * PRPC protocol stack (over TCP).
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argHostName
 *                host name or IP address of the SM
 * @param argPort TCP port (default 14374)
 * @return success\fail
 */
OSAL_EXTERN OSAL_DllExport bool SMNB_connect(SMNB_HANDLE argApiHandle,char* argHostName, Uint16 argPort);

/**
 * close the connection with the SM.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @return success\fail
 */
OSAL_EXTERN OSAL_DllExport bool SMNB_disconnect(SMNB_HANDLE argApiHandle);

/**
 * check whether the API is in connected mode
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @return true - connected 
 *         false - disconnected
 */
OSAL_EXTERN OSAL_DllExport bool SMNB_isConnected(SMNB_HANDLE argApiHandle);

/**
 * retreive the SM version string.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @return string or NULL if disconnected
 */
OSAL_EXTERN OSAL_DllExport const char*  SMNB_getSMVersion(SMNB_HANDLE argApiHandle);

/**
 * retreive the API version string.
 * 
 * @return string
 */
OSAL_EXTERN OSAL_DllExport const char*  SMNB_getApiVersion(SMNB_HANDLE argApiHandle);

/**
 * ****************** debug method *****************
 * this method sends the SM version string to the
 * SM and receives it back. it could be used for testing
 * the API in your system.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @return operation handle or a negative value if failed to pass the operation to the SM.
 *         the expected return value (result handler) type is string.
 */
OSAL_EXTERN OSAL_DllExport int SMNB_echo(SMNB_HANDLE argApiHandle);



/**
 * performs a login of a CPE or CM to the SM in a cable environment.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argCpe    CPE Id or CM Id for CM login
 * @param argCm     CM Id or NULL if does not exist.
 * @param argIp     the allocated IP address in dotted notation
 * @param argLease  lease time in seconds
 * @param argDomain domain hint string (SMNB_HANDLE argApiHandle,can be the relay-agent IP)
 * @param argPropertyKeys
 *                  subscriber property keys
 * @param argPropertyValues
 *                  subscriber property values assosiated with the keys array elements
 * @param argPropertySize
 *                  the size of the property arrays
 * @return operation handle or a negative value if failed to pass the operation to the SM
 *         the expected return value (result handler) type is void
 * @see login for further detail
 */
OSAL_EXTERN OSAL_DllExport int SMNB_loginCable(SMNB_HANDLE argApiHandle,char* argCpe,
					   char* argCm,
					   char* argIp,
					   int argLease,
					   char* argDomain,
					   char** argPropertyKeys,
					   char** argPropertyValues,
					   int argPropertySize);

/**
 * performs a login of a subscriber to the SM.
 * if the subscriber does not exist in the SM the subscriber
 * will be added with the specified parameters.
 * if the subscriber exists in the SM the subscriber's mappings
 * will be added\replaced according to the argIsAdditive flag,
 * the specified subscriber properties will replace only the
 * properties that are supplied and the other properties if
 * exist will remain as is.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argName   subscriber Id
 * @param argMappings
 *                  array of mapping (SMNB_HANDLE argApiHandle,IP address\IP range\Vlan ...) strings.
 *                  the IP address format is dotted notation.
 *                  the IP range format is as folows: a.b.c.d/r when r is the number of
 *                  bits in the mask (SMNB_HANDLE argApiHandle,0-32).  16 <=> 255.255.0.0
 *                  the Vlan format is an integer value representation.
 * @param argMappingTypes
 *                  @see MappingType enumeration
 *                  IP = 0
 *                  Vlan = 1
 *                  each element in this array is assosiated to the relevant element
 *                  in the the mapping array.
 * @param argMappingsSize
 *                  the size of the mapping arrays
 * @param argPropertyKeys
 *                  subscriber property keys
 * @param argPropertyValues
 *                  subscriber property values assosiated with the keys array elements
 * @param argPropertySize
 *                  the size of the property arrays
 * @param argDomain domain name.
 * @param argIsAdditive
 *                  whether the specified mappings should be added to the
 *                  subscriber's existing mappings (SMNB_HANDLE argApiHandle,true) or should they
 *                  replace them (SMNB_HANDLE argApiHandle,false)
 * @param argAutoLogoutTime
 *                  time in senconds till the subscriber should be logout
 *                  automatically.
 *                  0  - no autologout.
 *                  -1 - cancel auto logout if exists
 * @return operation handle or a negative value if failed to pass the operation to the SM
 *         the expected return value (result handler) type is void
 */
OSAL_EXTERN OSAL_DllExport int SMNB_login(SMNB_HANDLE argApiHandle,char* argName,
				  char** argMappings,
				  MappingType* argMappingTypes,
				  int argMappingsSize,
				  char** argPropertyKeys,
				  char** argPropertyValues,
				  int argPropertySize,
				  char* argDomain,
				  bool argIsAdditive,
				  int argAutoLogoutTime);

/**
 * perform a logout operation in a cable environment
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argCpe    CPE Id or CM Id for CM logout
 * @param argCm     CM Id or NULL if does not exist.
 * @param argIp     the allocated IP address in dotted notation
 * @param argDomain domain hint string (SMNB_HANDLE argApiHandle,can be the relay-agent IP)
 * @return operation handle or a negative value if failed to pass the operation to the SM
 *         the expected return value (result handler) type is boolean:
 *         True/False if subscriber was Found/Not found
 * @see logoutByName for further information
 */
OSAL_EXTERN OSAL_DllExport int SMNB_logoutCable(SMNB_HANDLE argApiHandle,char* argCpe,
					   char* argCm,
					   char* argIp,
					   char* argDomain);
/**
 * logout the subscriber from the SM.
 * the operation removes the specified subscriber's mappings from the
 * subscriber, but does not remove the subscriber from the SM.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argName   subscriber Id
 * @param argMappings
 *                  array of mapping (SMNB_HANDLE argApiHandle,IP address\IP range\Vlan ...) strings.
 *                  the IP address format is dotted notation.
 *                  the IP range format is as folows: a.b.c.d/r when r is the number of
 *                  bits in the mask (SMNB_HANDLE argApiHandle,0-32).  16 <=> 255.255.0.0
 *                  the Vlan format is an integer value representation.
 * @param argMappingTypes
 *                  @see MappingType enumeration
 *                  IP = 0
 *                  Vlan = 1
 *                  each element in this array is assosiated to the relevant element
 *                  in the the mapping array.
 * @param argMappingsSize
 *                  the size of the mapping arrays
 * @return operation handle or a negative value if failed to pass the operation to the SM
 *         the expected return value (result handler) type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport int  SMNB_logoutByName(SMNB_HANDLE argApiHandle,char* argName,
						  char** argMappings,
						  MappingType* argMappingTypes,
						  int argMappingsSize);
/**
 * logout the subscriber from the SM.
 * the operation removes the specified subscriber's mappings from the
 * subscriber, but does not remove the subscriber from the SM.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argName   subscriber Id
 * @param argMappings
 *                  array of mapping (SMNB_HANDLE argApiHandle,IP address\IP range\Vlan ...) strings.
 *                  the IP address format is dotted notation.
 *                  the IP range format is as folows: a.b.c.d/r when r is the number of
 *                  bits in the mask (SMNB_HANDLE argApiHandle,0-32).  16 <=> 255.255.0.0
 *                  the Vlan format is an integer value representation.
 * @param argMappingTypes
 *                  @see MappingType enumeration
 *                  IP = 0
 *                  Vlan = 1
 *                  each element in this array is assosiated to the relevant element
 *                  in the the mapping array.
 * @param argMappingsSize
 *                  the size of the mapping arrays
 * @param argDomain domain name or NULL if domain-less
 * @return operation handle or a negative value if failed to pass the operation to the SM
 *         the expected return value (result handler) type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport int  SMNB_logoutByNameFromDomain(SMNB_HANDLE argApiHandle,char* argName,
						  char** argMappings,
						  MappingType* argMappingTypes,
						  int argMappingsSize,
						  char* argDomain);

/**
 * finds the subscriber that holds the mapping and logouts
 * the subscriber from the SM.
 * the operation removes the specified subscriber's mappings from the
 * subscriber, but does not remove the subscriber from the SM.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param argMapping mapping (SMNB_HANDLE argApiHandle,IP address\IP range\Vlan ...) string.
 *                   the IP address format is dotted notation.
 *                   the IP range format is as folows: a.b.c.d/r when r is the number of
 *                   bits in the mask (SMNB_HANDLE argApiHandle,0-32).  16 <=> 255.255.0.0
 *                   the Vlan format is an integer value representation.
 * @param argMappingType
 *                   @see MappingType enumeration
 *                   IP = 0
 *                   Vlan = 1
 * @param argDomain  domain name or NULL if not known
 * @return operation handle or a negative value if failed to pass the operation to the SM
 *         the expected return value (result handler) type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport int  SMNB_logoutByMapping(SMNB_HANDLE argApiHandle,char* argMapping,
							 MappingType argMappingType,
							 char* argDomain);




/**
 * specifies the connection down callback function.
 * this method removes the callback function by setting
 * NULL as the callback function.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param callback callback function pointer
 */
OSAL_EXTERN OSAL_DllExport void SMNB_setDisconnectListener(SMNB_HANDLE argApiHandle,ConnectionIsDownCallBackFunc callback);



/**
 * specifies the "operation failed" callback function.
 * this method removes the callback function by setting
 * NULL as the callback function.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param callback callback function pointer
 */
OSAL_EXTERN OSAL_DllExport  void SMNB_setReplyFailCallBack(SMNB_HANDLE argApiHandle,OperationFailCallBackFunc callback);

/**
 * specifies the "operation succeeded" callback function.
 * this method removes the callback function by setting
 * NULL as the callback function.
 * 
 * @param argApiHandle
 *               handle to the non-blocking API
 * @param callback callback function pointer
 */
OSAL_EXTERN OSAL_DllExport  void SMNB_setReplySuccessCallBack(SMNB_HANDLE argApiHandle,OperationSuccessCallBackFunc callback);



  
#endif
