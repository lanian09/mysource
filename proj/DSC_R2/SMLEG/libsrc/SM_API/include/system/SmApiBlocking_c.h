#ifndef _SM_API_BLOCKING_C_H
#define _SM_API_BLOCKING_C_H

#include "Common.h"
#include "GeneralDefs.h"

/**
 * this API supports blocking operations that are performed on the SM.
 * the API supports one connection per SMB_HANDLE, multiple SMB_HANDLE's should be used to work
 * with several SMs.
 * when using the API the user must first call the SMB_init() method.
 * the user should implement the connection-down callback to be able to identify
 * a connection failure and reconnect to the SM.
 * note that the returned values (ReturnCode structures) MUST be freed by the
 * API user. you can use the freeReturnCode	function to handle the memory release.
 * 										    
 * please read the API methods thoroughly before integratin the API into your
 * system.
 * 
 * @author YoavG
 */
 
typedef void* SMB_HANDLE;

/**
 * initializes the API internal tasks and objects.
 * this method must be called before any other method
 * of the API is called.
 * 
 * @param argSupportedThreads
 *               the number of thereds the API must support (optional)
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
OSAL_EXTERN OSAL_DllExport SMB_HANDLE SMB_init(int argSupportedThreads,
										 int argThreadPriority,
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
 *               handle to the blocking API
 */
OSAL_EXTERN OSAL_DllExport void SMB_setReconnectTimeout(SMB_HANDLE argApiHandle, Uint32 reconnectTimeout);


/**
 * set the name prefix used to expose to the peer who is this client
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName
 */
OSAL_EXTERN OSAL_DllExport void SMB_setName(SMB_HANDLE argApiHandle,char *argName);



/**
 * release the resources allocated for the specified handle.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return 
 */
OSAL_EXTERN OSAL_DllExport void SMB_release(SMB_HANDLE argApiHandle);

/**
 * set the destination to send the events to.
 * (SM address) and connect to it using the
 * PRPC protocol stack (over TCP).
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argHostName
 *                host name or IP address of the SM
 * @param argPort TCP port (default 14374)
 * @return success\fail
 */
OSAL_EXTERN OSAL_DllExport bool SMB_connect(SMB_HANDLE argApiHandle,char* argHostName, Uint16 argPort);

/**
 * close the connection with the SM.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return success\fail
 */
OSAL_EXTERN OSAL_DllExport bool SMB_disconnect(SMB_HANDLE argApiHandle);

/**
 * check whether the API is in connected mode
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return true - connected 
 *         false - disconnected
 */
OSAL_EXTERN OSAL_DllExport bool SMB_isConnected(SMB_HANDLE argApiHandle);

/**
 * retreive the SM version string.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return string or NULL if disconnected
 */
OSAL_EXTERN OSAL_DllExport const char*  SMB_getSMVersion(SMB_HANDLE argApiHandle);

/**
 * retreive the API version string.
 * 
 * @return string
 */
OSAL_EXTERN OSAL_DllExport const char*  SMB_getApiVersion(SMB_HANDLE argApiHandle);

/**
 * ****************** debug method *****************
 * this method sends the SM version string to the
 * SM and receives it back. it could be used for testing
 * the API in your system.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return ReturnCode pointer structure or NULL if an internal error occured.
 *         the return value type is string.
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_echo(SMB_HANDLE argApiHandle);



/**
 * performs a login of a CPE or CM to the SM in a cable environment.
 * 
 * @param argApiHandle
 *               handle to the blocking API
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
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is void
 * @see login for further detail
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_loginCable(SMB_HANDLE argApiHandle,char* argCpe,
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
 *               handle to the blocking API
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
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is void
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_login(SMB_HANDLE argApiHandle,char* argName,
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
 *               handle to the blocking API
 * @param argCpe    CPE Id or CM Id for CM logout
 * @param argCm     CM Id or NULL if does not exist.
 * @param argIp     the allocated IP address in dotted notation
 * @param argDomain domain hint string (can be the relay-agent IP)
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True/False if subscriber was Found/Not found
 * @see logoutByName for further information
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_logoutCable(SMB_HANDLE argApiHandle,char* argCpe,
					   char* argCm,
					   char* argIp,
					   char* argDomain);
/**
 * logout the subscriber from the SM.
 * the operation removes the specified subscriber's mappings from the
 * subscriber, but does not remove the subscriber from the SM.
 * 
 * @param argApiHandle
 *               handle to the blocking API
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
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport ReturnCode*  SMB_logoutByName(SMB_HANDLE argApiHandle,char* argName,
						  char** argMappings,
						  MappingType* argMappingTypes,
						  int argMappingsSize);

/**
 * logout the subscriber from the SM.
 * the operation removes the specified subscriber's mappings from the
 * subscriber, but does not remove the subscriber from the SM.
 * 
 * @param argApiHandle
 *               handle to the blocking API
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
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport ReturnCode*  SMB_logoutByNameFromDomain(SMB_HANDLE argApiHandle,char* argName,
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
 *               handle to the blocking API
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
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport ReturnCode*  SMB_logoutByMapping(SMB_HANDLE argApiHandle,char* argMapping,
							 MappingType argMappingType,
							 char* argDomain);


/**
 * Creates a subscriber according to the given data.
 * If a subscriber by this name already exists, it is
 * removed before adding the new one.
 * 
 * @param argApiHandle
 *               handle to the blocking API
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
 * @param argCustomPropertyKeys
 * @param argCustomPropertyValues
 * @param argCustomPropertySize
 * @param argDomain domain name.
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is void
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_addSubscriber(SMB_HANDLE argApiHandle,char* argName,
				  char** argMappings,
				  MappingType* argMappingTypes,
				  int argMappingsSize,
				  char** argPropertyKeys,
				  char** argPropertyValues,
				  int argPropertySize,
				  char** argCustomPropertyKeys,
				  char** argCustomPropertyValues,
				  int argCustomPropertySize,
				  char* argDomain);

/**
 * Removes a subscriber from the SM DataBase
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName subscriber Id
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_removeSubscriber(SMB_HANDLE argApiHandle,char* argName);

/**
 * retreive the subsriber information formatted as an
 * array of strings and string arrays.
 * This mehtod is intended for easy subscriber info presentation.
 * 
 * no array elements are null
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName subscriber Id
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is object-array
 *         each object in the array represent a subscriber parameter:
 *         index 0: subscriber name (String)
 *         index 1: array of mappings (String[])
 *         index 2: array of mapping types (short[])
 *         index 3: domain name (String)
 *         index 4: array of property names (String[])
 *         index 5: array of property values (String[])
 *         index 6: array of custom property names (String[])
 *         index 7: array of custom property values (String[])
 *         index 8: auto-logout time, seconds from now or -1 if not set (Integer)
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getSubscriber(SMB_HANDLE argApiHandle,char* argName);

/**
 * Check whether 'argName' exists in SM's database.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName subscriber Id
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True/False if subscriber was Found/Not found
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_subscriberExists(SMB_HANDLE argApiHandle,char* argName);


/**
 * finds the subscriber that has the mapping in a domain
 * 
 * @param argApiHandle
 *               handle to the blocking API
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
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is string:
 *         subscriber name or null if does not exist
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getSubscriberNameByMapping(SMB_HANDLE argApiHandle,char* argMapping,
							 MappingType argMappingType,
							 char* argDomain);

/**
 * Gets subscribers in SM's database, following <code>argLastName</code> up to
 * <code>argAmount</code> subscribers (alphabetically).<p>
 * 
 * If <code>argLastName</code> is <code>null</code>, the names will begin with the first subscriber
 * name that exists will be used.<p>
 * 
 * The method will return as many subscribers as found in SM's database, starting at
 * the requested subscriber. The array size if limited by the minimum between
 * <code>argAmount</code> and the SM's internal limit.<P>
 * 
 * Note that there is no guarantee that the total number of subscribers (in all bulks) equals
 * to the value returned from <CODE>getNumberOfSubscribers</CODE> at any time (they may differ, for
 * example, if some subscribers are added/removed while bulks are being retrieved).<p>
 * 
 * @param argApiHandle
 *                  handle to the blocking API
 * @param argLastName
 *                  The last name returned in the previous call.
 *                  The first call to this method should use NULL.
 * @param argAmount max number of subscriber names to return
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is string-array in alphabet order
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getSubscriberNames(SMB_HANDLE argApiHandle,char* argLastName,
									   int argAmount);

/**
 * Gets subscribers in SMM's database, that are associated with the specified domain<p>
 * 
 * the method retreives the subscriber names following <code>argLastName</code> up to
 * <code>argAmount</code> subscribers (alphabetically).<p>
 * 
 * If <code>argLastName</code> is <code>null</code>, the names will begin with the first subscriber
 * name that exists will be used.<p>
 * 
 * The method will return as many subscribers as found in SM's database, starting at
 * the requested subscriber. The array size if limited by the minimum between
 * <code>argAmount</code> and the SM's internal limit.<P>
 * 
 * Note that there is no guarantee that the total number of subscribers (in all bulks) equals
 * to the value returned from <CODE>getNumberOfSubscribers</CODE> at any time (they may differ, for
 * example, if some subscribers are added/removed while bulks are being retrieved).<p>
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argLastName
 *                  The last name returned in the previous call.
 *                  The first call to this method should use NULL.
 * @param argAmount max number of subscriber names to return
 * @param argDomain domain name or NULL if not known
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is string-array in alphabet order
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getSubscriberNamesInDomain(SMB_HANDLE argApiHandle,char* argLastName,
											int argAmount,
											char* argDomain);

/**
 * Gets subscribers in SMM's database, that begin with the specified prefix<p>
 * 
 * This method returns up to <code>argAmount</code> subscribers, that
 * are alphabetically after <code>argLastName</code>.<br>
 * 
 * If <code>argLastName</code> is <code>null</code>, the names will begin with the first subscriber
 * name that exists will be used.<p>
 * 
 * The method will return as many subscribers as found in SM's database, starting at
 * the requested subscriber. The array size if limited by the minimum between
 * <code>argAmount</code> and the SM's internal limit.<P>
 * 
 * Note that there is no guarantee that the total number of subscribers (in all bulks) equals
 * to the value returned from <CODE>getNumberOfSubscribers</CODE> at any time (they may differ, for
 * example, if some subscribers are added/removed while bulks are being retrieved).<p>
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argLastName
 *                  The last name returned in the previous call.
 *                  The first call to this method should use NULL.
 * @param argAmount max number of subscriber names to return
 * @param argPrefix
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is string-array in alphabet order
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getSubscriberNamesWithPrefix(SMB_HANDLE argApiHandle,char* argLastName,
											int argAmount,
											char* argPrefix);


/**
 * Gets subscribers in SMM's database, that end with the specified suffix<p>
 * 
 * This method returns up to <code>argAmount</code> subscribers, that have the same suffix
 * and are alphabetically after <code>argLastName</code>.<br>
 * 
 * If <code>argLastName</code> is <code>null</code>, the names will begin with the first subscriber
 * name that exists will be used.<p>
 * 
 * The method will return as many subscribers as found in SM's database, starting at
 * the requested subscriber. The array size if limited by the minimum between
 * <code>argAmount</code> and the SM's internal limit.<P>
 * 
 * Note that there is no guarantee that the total number of subscribers (in all bulks) equals
 * to the value returned from <CODE>getNumberOfSubscribers</CODE> at any time (they may differ, for
 * example, if some subscribers are added/removed while bulks are being retrieved).<p>
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argLastName
 *                  The last name returned in the previous call.
 *                  The first call to this method should use NULL.
 * @param argAmount max number of subscriber names to return
 * @param argSuffix
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is string-array in alphabet order
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getSubscriberNamesWithSuffix(SMB_HANDLE argApiHandle,char* argLastName,
											int argAmount,
											char* argSuffix);


/**
 * Reset the specified application properties of a subscriber.
 * 
 * If an application is loaded, then the relevant
 * application properties are set the default value of
 * the properties according to the currently loaded
 * application info.
 * 
 * If an application is not loaded, the method will fail
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName subscriber Id
 * @param argPropertyKeys
 *                subscriber property keys
 * @param argPropertySize
 *                the size of the property arrays
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is	void
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_setPropertiesToDefault(SMB_HANDLE argApiHandle,char* argName,
				  char** argPropertyKeys,
				  int argPropertySize);

/**
 * removes the specified custom properties of a subscriber.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName subscriber Id
 * @param argCustomPropertyKeys
 *                subscriber property keys
 * @param argCustomPropertySize
 *                the size of the custom property array
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is void
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_removeCustomProperties(SMB_HANDLE argApiHandle,char* argName,
				  char** argCustomPropertyKeys,
				  int argCustomPropertySize);

/**
 * Remove all subscribers from the DB
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is void
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_removeAllSubscribers(SMB_HANDLE argApiHandle);

/**
 * removes all of the subscribers from the specified domain.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argDomain subscriber domain name
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_removeAllSubscribersInDomain(SMB_HANDLE argApiHandle,char* argDomain);

/**
 * Check whether <code>argName</code> exists and is currently logged-in
 * i.e. has mappings and domain.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argName Subscriber Id
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is boolean:
 *         True - logged in to a domain.
 *         False - not logged in to a domain
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_subscriberLoggedIn(SMB_HANDLE argApiHandle,char* argName);

/**
 * retreives the number of subscribers in the SM's database
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is int
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getNumberOfSubscribers(SMB_HANDLE argApiHandle);

/**
 * retreives the number of subscribers in the specified domain
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argDomain subscriber domain name
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is int
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getNumberOfSubscribersInDomain(SMB_HANDLE argApiHandle,char* argDomain);

/**
 * retreives the subscriber domain names
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @return ReturnCode pointer structure or NULL if an internal error occured
 *         the return value type is string-array
 */
OSAL_EXTERN OSAL_DllExport ReturnCode* SMB_getDomains(SMB_HANDLE argApiHandle);


/**
 * specifies the connection down callback function.
 * this method removes the callback function by setting
 * NULL as the callback function.
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param callback callback function pointer
 */
OSAL_EXTERN OSAL_DllExport void SMB_setDisconnectListener(SMB_HANDLE argApiHandle,ConnectionIsDownCallBackFunc callback);


/**
 * set the maximal blocking time
 * 
 * @param argApiHandle
 *               handle to the blocking API
 * @param argTimeout Timeout in milliseconds:
 *                   Negative values - forbidden
 *                   Zero - wait forever
 *                   Positive values
 */
OSAL_EXTERN OSAL_DllExport void SMB_setReplyTimeout(SMB_HANDLE argApiHandle,int argTimeout);




  
#endif
