/** @file Exception.h */

#ifndef GEM_DAQMON_EXCEPTION_EXCEPTION_H
#define GEM_DAQMON_EXCEPTION_EXCEPTION_H

#include <string>

#include "xcept/Exception.h"

/***
 // Macros defined in xdaq code that are useful to remember
 //! Macro to throw an excpetion with line number and function name
 #define XCEPT_RAISE( EXCEPTION, MSG )  throw EXCEPTION( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__)

 #define XCEPT_RETHROW( EXCEPTION, MSG, PREVIOUS )  throw EXCEPTION( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__, PREVIOUS)

 #define XCEPT_ASSERT(COND, EXCEPTION, MSG)  if (!(COND))  { XCEPT_RAISE(EXCEPTION, MSG); }

 // Create a new exception and use
 // it as a variable called VAR
 #define XCEPT_DECLARE( EXCEPTION, VAR, MSG )				 EXCEPTION VAR( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__)

 // Create a new exception from a previous one and use
 // it as a variable called VAR
 #define XCEPT_DECLARE_NESTED( EXCEPTION, VAR, MSG, PREVIOUS )		 EXCEPTION VAR( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__, PREVIOUS)
***/

#define GEM_DAQMON_DEFINE_EXCEPTION(EXCEPTION_NAME)                        namespace gem {                                                           namespace daqmon {                                                         namespace exception {                                                     class EXCEPTION_NAME : virtual public xcept::Exception                    {                                                                       public :                                                                EXCEPTION_NAME(std::string name,                                                       std::string message,                                                    std::string module,                                                     int line,                                                               std::string function) :                                    xcept::Exception(name, message, module, line, function)                   {};                                                                 EXCEPTION_NAME(std::string name,                                                       std::string message,                                                    std::string module,                                                     int line,                                                               std::string function,                                                   xcept::Exception& err) :                                   xcept::Exception(name, message, module, line, function, err)               {};                                                                 };                                                                  }  /* namespace gem::daqmon::exception */                              }  /* namespace gem::daqmon            */                              }  /* namespace gem                   */

// The gem::daqmon exceptions.
GEM_DAQMON_DEFINE_EXCEPTION(Exception)
GEM_DAQMON_DEFINE_EXCEPTION(SoftwareProblem)
GEM_DAQMON_DEFINE_EXCEPTION(ValueError)

// The gem::daqmon alarms.
#define GEM_DAQMON_DEFINE_ALARM(ALARM_NAME) GEM_DAQMON_DEFINE_EXCEPTION(ALARM_NAME)
GEM_DAQMON_DEFINE_ALARM(MonitoringFailureAlarm)

#endif  // GEM_DAQMON_EXCEPTION_EXCEPTION_H
