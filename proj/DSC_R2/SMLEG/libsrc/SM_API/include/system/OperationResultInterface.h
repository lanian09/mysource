#ifndef _OPERATION_RESULT_INTERFACE
#define _OPERATION_RESULT_INTERFACE

#include "Common.h"
#include "GeneralDefs.h"

/**
 * class for API internal use only !!!
 * this class is used to expose an interface that receives a detailed
 * callback about each operation result.
 * 
 * @author YoavG
 */
class OperationResultInterface
{
public:
	/**
	 * a callback called for each operation result.
	 * 
	 * @param argThreadNumber
	 *                  the thread number that invoked the operation
	 * @param argHandle operation handle allocated by the API
	 * @param argIsSuccessful
	 *                  whether the operation was successful or whether it failed
	 * @param argReturnCode
	 *                  return code structure.
	 *                  if null => there was an internal error.
	 */
	virtual void onOperationResult(Uint32 argThreadNumber,
									Uint32 argHandle,
									bool   argIsSuccessful,
									ReturnCode *argReturnCode) = 0;

	/**
	 * an API internal error occured and all operations had failed.
	 */
	virtual void onAllOperationsFailed(const char *errorMessage) = 0;

};

#endif
