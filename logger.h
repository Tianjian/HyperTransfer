#ifndef _LOGGER_H_
#define _LOGGER_H_

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

// see ref: http://www.delorie.com/gnu/docs/gcc/gcc_78.html
//  
// __PRETTY_FUNCTION__  complete function description
// __FUNCTION__ summary function description

#define APPEND_FUNCTION(MSG) "[" << __FUNCTION__ << "] ["<<__LINE__<<"]" << MSG



// define logger  interface macros   
#define LOG(TYPE,MSG) LOG_##TYPE( g_logger , APPEND_FUNCTION(MSG)) //main dispatcher

#define LOG_TRACE(a,b) LOG4CPLUS_TRACE(a,b) //for level TRACE
#define LOG_DEBUG(a,b) LOG4CPLUS_DEBUG(a,b) //for level DEBUG
#define LOG_INFO(a,b)  LOG4CPLUS_INFO(a,b)  //for level INFO
#define LOG_WARN(a,b)  LOG4CPLUS_WARN(a,b)  //for level WARN
#define LOG_ERROR(a,b) LOG4CPLUS_ERROR(a,b) //for level ERROR
#define LOG_FATAL(a,b) LOG4CPLUS_FATAL(a,b) //for level FATAL

/**
    Init Function For log4cplus
*/

// global variable 
extern log4cplus::Logger g_logger;

// function for initializing log4cplus configuration
bool init_logger( const string &log_config_pathname );



#endif
