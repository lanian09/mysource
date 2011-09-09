#ifndef _SM_API_NON_BLOCKING_H
#define _SM_API_NON_BLOCKING_H

#include "Common.h"
#include "Logger.h"
#include "GeneralDefs.h"
#include "PrintLogger.h"

class PrpcClient;

/**
 * this class supports non-blocking operations that are performed on the SM.
 * the API supports one connection, multiple API's should be used to work
 * with several SMs.
 * when using the API the user must first call the init() method.
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
class OSAL_DllExport SmApiNonBlocking
{
public:

	/**
	 * CTOR
	 * 
	 */
	SmApiNonBlocking();


	/**
	 * DTOR
	 */
	virtual ~SmApiNonBlocking();

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
	 * @return success\fail of initialization
	 */
	virtual bool init(int argThreadPriority = 0,
					 Uint32 argBufferSize = DEFAULT_BUFFER_SIZE,
					 Uint32 argKeepAliveDuration = DEFAULT_KEEP_ALIVE_DURATION,
					 Uint32 argConnectionTimeout= DEFAULT_CONNECTION_TIMEOUT,
					 Uint32 argReconnectTimeout = NO_RECONNECT);

	/**
	 * sets the reconnection timeout
	 * when the connection to the SM is down the API will try to
	 * re-establish the connection after reconnection timeout
	 * 
	 * the parameter can also be set during init
	 *
	 * @param reconnectionTimeout
	 *               the timeout in seconds
	 */
	virtual void setReconnectTimeout(Uint32 reconnectTimeout);

	/**
	 * set the name prefix used to expose to the peer who is this client
	 * 
	 * @param argName
	 */
	virtual void setName(char *argName);

	/**
	 * set the destination to send the events to.
	 * (SM address) and connect to it using the
	 * PRPC protocol stack (over TCP).
	 * this method is blocking.
	 * 
	 * @param argHostName
	 *                host name or IP address of the SM
	 * @param argPort TCP port (default 14374)
	 * @return success\fail
	 */
	virtual bool connect(char* argHostName, Uint16 argPort = 14374);

	/**
	 * close the connection with the SM.
	 * this method is blocking.
	 * 
	 * @return success\fail
	 */
	virtual bool disconnect();

	/**
	 * check whether the API is in connected mode
	 * 
	 * @return true - connected 
	 *         false - disconnected
	 */
	virtual bool isConnected();

	/**
	 * retreive the SM version string.
	 * 
	 * @return string or NULL if disconnected
	 */
	virtual const char* getSMVersion();

	/**
	 * retreive the API version string.
	 * 
	 * @return string
	 */
	virtual const char* getApiVersion();

	/**
	 * set a logging object of your own.
	 * this method should be used to integrate the SM API
	 * log messages with the system log.
	 * 
	 * @param argLogger Logger class implementation
	 */
	virtual void setLogger(Logger *argLogger);


	/**
	 * ****************** debug method ******************
	 * this method sends the SM version string to the
	 * SM and receives it back. it could be used for testing
	 * the API in your system.
	 * 
	 * @return operation handle or a negative value if failed to pass the operation to the SM
	 */
	virtual int echo();


	// ************************* logon methods ********************
	
	/**
	 * performs a login of a CPE or CM to the SM in a cable environment.
	 * @see login for further detail
	 * @param argCpe    CPE Id or CM Id for CM login
	 * @param argCm     CM Id or NULL if does not exist.
	 * @param argIp     the allocated IP address in dotted notation
	 * @param argLease  lease time in seconds
	 * @param argDomain domain hint string (can be the relay-agent IP)
	 * @param argPropertyKeys
	 *                  subscriber property keys
	 * @param argPropertyValues
	 *                  subscriber property values assosiated with the keys array elements
	 * @param argPropertySize
	 *                  the size of the property arrays
	 * @return operation handle or a negative value if failed to pass the operation to the SM
	 */
	virtual int loginCable(char* argCpe,
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
	 * @param argName   subscriber Id
	 * @param argMappings
	 *                  array of mapping (IP address\IP range\Vlan ...) strings.
	 *                  the IP address format is dotted notation.
	 *                  the IP range format is as folows: a.b.c.d/r when r is the number of
	 *                  bits in the mask (0-32).  16 <=> 255.255.0.0
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
	 *                  subscriber's existing mappings (true) or should they
	 *                  replace them (false)
	 * @param argAutoLogoutTime
	 *                  time in senconds till the subscriber should be logout
	 *                  automatically.
	 *                  0  - no autologout.
	 *                  -1 - cancel auto logout if exists
	 * @return operation handle or a negative value if failed to pass the operation to the SM
	 */
	virtual int login(char* argName,
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
	 * @see logoutByName for further information
	 * @param argCpe    CPE Id or CM Id for CM logout
	 * @param argCm     CM Id or NULL if does not exist.
	 * @param argIp     the allocated IP address in dotted notation
	 * @param argDomain domain hint string (can be the relay-agent IP)
	 * @return operation handle or a negative value if failed to pass the operation to the SM
	 */
	virtual int logoutCable(char* argCpe,
						   char* argCm,
						   char* argIp,
						   char* argDomain);
	/**
	 * logout the subscriber from the SM.
	 * the operation removes the specified subscriber's mappings from the
	 * subscriber, but does not remove the subscriber from the SM. 
	 * 
	 * @param argName   subscriber Id
	 * @param argMappings
	 *                  array of mapping (IP address\IP range\Vlan ...) strings.
	 *                  the IP address format is dotted notation.
	 *                  the IP range format is as folows: a.b.c.d/r when r is the number of
	 *                  bits in the mask (0-32).  16 <=> 255.255.0.0
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
	 */
	virtual int  logoutByName(char* argName,
							  char** argMappings,
							  MappingType* argMappingTypes,
							  int argMappingsSize);

	/**
	 * logout the subscriber from the SM.
	 * the operation removes the specified subscriber's mappings from the
	 * subscriber, but does not remove the subscriber from the SM. 
	 * 
	 * @param argName   subscriber Id
	 * @param argMappings
	 *                  array of mapping (IP address\IP range\Vlan ...) strings.
	 *                  the IP address format is dotted notation.
	 *                  the IP range format is as folows: a.b.c.d/r when r is the number of
	 *                  bits in the mask (0-32).  16 <=> 255.255.0.0
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
	 */
	virtual int  logoutByNameFromDomain(char* argName,
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
	 * @param argMapping mapping (IP address\IP range\Vlan ...) string.
	 *                   the IP address format is dotted notation.
	 *                   the IP range format is as folows: a.b.c.d/r when r is the number of
	 *                   bits in the mask (0-32).  16 <=> 255.255.0.0
	 *                   the Vlan format is an integer value representation.
	 * @param argMappingType
	 *                   @see MappingType enumeration
	 *                   IP = 0
	 *                   Vlan = 1
	 * @param argDomain  domain name or NULL if not known
	 * @return operation handle or a negative value if failed to pass the operation to the SM
	 */
	virtual int  logoutByMapping(char* argMapping,
								 MappingType argMappingType,
								 char* argDomain);



	/**
	 * specifies the connection down callback function.
	 * this method removes the callback function by setting
	 * NULL as the callback function.
	 * 
	 * @param callback callback function pointer
	 */
	virtual void setDisconnectListener(ConnectionIsDownCallBackFunc callback);

	/**
	 * specifies the "operation failed" callback function.
	 * this method removes the callback function by setting
	 * NULL as the callback function.
	 * 
	 * @param callback callback function pointer
	 */
	virtual void setReplyFailCallBack(OperationFailCallBackFunc callback);
	
	/**
	 * specifies the "operation succeeded" callback function.
	 * this method removes the callback function by setting
	 * NULL as the callback function.
	 * 
	 * @param callback callback function pointer
	 */
	virtual void setReplySuccessCallBack(OperationSuccessCallBackFunc callback);

		virtual void setLoggerLevel(int logLevel);
		virtual int clearClientBuffer();
		virtual bool isFull();
		virtual Uint32* getQueueSize();

private:

protected:
	PrpcClient *myPrpcc;
	PrintLogger log;
};
	  
#endif
