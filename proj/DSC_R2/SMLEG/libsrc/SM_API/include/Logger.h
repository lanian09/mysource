#ifndef _LOGGER_H
#define _LOGGER_H

/**
 * this class is used to specify an interface of a general logger.
 * the class implementation should convert the general interface into 
 * the interface of the specific logger used by the system.
 * 
 * @author YoavG
 */
class OSAL_DllExport Logger
{
public:

	typedef enum
	{
		TRACE_HIGH,
		TRACE_MED,
		TRACE_LOW
	}TraceLevel;

	/**
	 * CTOR
	 */
	Logger() {};

	/**
	 * DTOR
	 */
	virtual ~Logger(){};

	/**
	 * used for error messages - 
	 * the system has encountered a problem, and it cannot handle it or might not 
	 * be able to handle it.
	 * 
	 * @param argMsg message to be logged
	 * @return success or fail
	 */
	virtual bool error(char* argMsg) = 0;

	/**
	 * used for warning messages - 
	 * the system has encountered a problem, but it is able to handle it.
	 * 
	 * @param argMsg message to be logged
	 * @return success or fail
	 */
	virtual bool warn(char* argMsg) = 0;

	/**
	 * used for informational messages.
	 * 
	 * @param argMsg message to be logged
	 * @return success or fail
	 */
	virtual bool info(char* argMsg) = 0;

	/**
	 * generaly used for debug operations.
	 * use the trace level to difrentiate the debug level you want
	 * 
	 * @param argLevel @see TraceLevel
	 * @param argMsg   message to be logged
	 * @return success or fail
	 */
	virtual bool trace(Logger::TraceLevel argLevel, char* argMsg) = 0;
};

#endif
