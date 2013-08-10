#ifndef _PRINT_LOGGER_H
#define _PRINT_LOGGER_H

#include "Common.h"
#include "Logger.h"

/**
 * this class is used to log events to the standard error.
 * 
 * @author YoavG
 */
class OSAL_DllExport PrintLogger : public Logger
{
public:
	typedef enum 
	{
		LOG_TRACE,
		LOG_INFO,
		LOG_WARN,
		LOG_ERROR
	}LoggingLevel;

	/**
	 * CTOR
	 */
	PrintLogger();

	/**
	 * DTOR
	 */
	virtual ~PrintLogger(){};

	/**
	 * used to define the level from which the object will actually log the message.
	 * 
	 * @param argLogLevel @see LoggingLevel
	 * @param argTraceLevel @see TraceLevel
	 * @return success or fail
	 */
	virtual void setLoggingLevels(LoggingLevel argLogLevel, Logger::TraceLevel argTraceLevel = Logger::TRACE_HIGH);

	/**
	 * used for error messages - 
	 * the system has encountered a problem, and it cannot handle it or might not 
	 * be able to handle it.
	 * 
	 * @param argMsg message to be logged
	 * @return success or fail
	 */
	virtual bool error(char* argMsg);

	/**
	 * used for warning messages - 
	 * the system has encountered a problem, but it is able to handle it.
	 * 
	 * @param argMsg message to be logged
	 * @return success or fail
	 */
	virtual bool warn(char* argMsg);

	/**
	 * used for informational messages.
	 * 
	 * @param argMsg message to be logged
	 * @return success or fail
	 */
	virtual bool info(char* argMsg);

	/**
	 * generaly used for debug operations.
	 * use the trace level to difrentiate the debug level you want
	 * 
	 * @param argLevel @see TraceLevel
	 * @param argMsg   message to be logged
	 * @return success or fail
	 */
	virtual bool trace(Logger::TraceLevel argLevel, char* argMsg);

protected:
	LoggingLevel myLogLevel;	// current logging level
	TraceLevel   myTraceLevel;	// current trace level (relevant only when myLogLevel ==  LOG_TRACE)

};

#endif
