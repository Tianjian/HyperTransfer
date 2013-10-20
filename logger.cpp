#include "common_include.h"

log4cplus::Logger g_logger = log4cplus::Logger::getInstance("Global Logger");

bool init_logger( const string &log_config_pathname )
{
	string _log4cplus_prop = log_config_pathname ;
    
	struct stat file_status;
    if(::stat(_log4cplus_prop.c_str(), &file_status) < 0)
    {
        std::cout << _log4cplus_prop << " does not exist." << std::endl;
        return false;
    }
    PropertyConfigurator::doConfigure(_log4cplus_prop);
	return true;
};

