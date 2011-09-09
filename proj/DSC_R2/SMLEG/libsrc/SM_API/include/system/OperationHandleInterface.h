#ifndef _OPERATION_HANDLE_INTERFACE
#define _OPERATION_HANDLE_INTERFACE

#include "Common.h"
#include "GeneralDefs.h"

/**
 * class for API internal use only !!!
 * this class is used to expose an interface that receives the operation handle
 * before the operation is forwarded to the server.
 * 
 * @author YoavG
 */
class OperationHandleInterface
{
public:
	/**
	 * this method returns the operation handle allocated for the relevant
	 * thread number that invoked this method.
	 * 
	 * @param argThreadNumber
	 * @param argHandle
	 */
	virtual void onOperationHandleCreation(Uint32 argThreadNumber,
									Uint32 argHandle) = 0;
};

#endif
