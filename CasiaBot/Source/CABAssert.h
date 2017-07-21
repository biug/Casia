#pragma once

#include "Common.h"
#include <cstdio>
#include <cstdarg>
#include "Logger.h"
#include <sstream>
#include <stdarg.h>

#include <ctime>
#include <iomanip>

#define CAB_BREAK

#define CAB_ASSERT_ALL

#ifdef CAB_ASSERT_ALL
    #define CAB_ASSERT(cond, msg, ...) \
        do \
        { \
            if (!(cond)) \
            { \
                CasiaBot::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), ##__VA_ARGS__); \
                CAB_BREAK \
            } \
        } while(0)

    #define CAB_ASSERT_WARNING(cond, msg, ...) \
        do \
        { \
            if (!(cond)) \
            { \
                CasiaBot::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), ##__VA_ARGS__); \
            } \
        } while(0)

	#define CAB_ASSERT_SIMPLE(msg, ...) \
        do \
		{ \
            CasiaBot::Assert::ReportFailureSimple((msg), ##__VA_ARGS__); \
			CAB_BREAK \
		} while(0)
#else
    #define CAB_ASSERT(cond, msg, ...) 
#endif

namespace CasiaBot
{
    namespace Assert
    {
        void ShutDown();

        extern std::string lastErrorMessage;

        const std::string currentDateTime();

		void ReportFailureSimple(const char * msg, ...);

        void ReportFailure(const char * condition, const char * file, int line, const char * msg, ...);
    }
}
