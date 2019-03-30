/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#define SUN_DEF (defined(__sun__) or defined(__sun) or defined(sun))
#define SRV4_DEF (defined(__SVR4) or defined(__svr4__))
#define OSX_DEF (defined(__APPLE__) and defined(__MACH__))
#define UNIX_DEF (defined(__unix__) or defined(__unix) or defined(unix))
#define AIX_DEF (defined(_AIX) or defined(__TOS__AIX__))
#define LINUX_DEF (defined(__linux__) or defined(__linux) or defined(linux) or defined(__gnu_linux__))
#define WIN_DEF (defined(_WIN32))

#include "resources.hh"

#if WIN_DEF
#include <windows.h>
#include <psapi.h>

#elif UNIX_DEF or OSX_DEF
#include <unistd.h>
#include <sys/resource.h>

#if OSX_DEF
#include <mach/mach.h>

#elif AIX_DEF or (SUN_DEF and SRV4_DEF)
#include <fcntl.h>
#include <procfs.h>

#elif LINUX_DEF
#include <stdio.h>

#endif

#else
#error "Cannot define util::rss::peak() or util::rss::current() for an unknown OS."
#endif

namespace util::rss {
  /**
   * Returns the peak (maximum so far) resident set size (physical
   * memory use) measured in bytes, or zero if the value cannot be
   * determined on this OS.
   */

  uintmax_t peak (void) {
  #if WIN_DEF // Windows
  	PROCESS_MEMORY_COUNTERS info;
  	GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
  	return info.PeakWorkingSetSize;

  #elif AIX_DEF or (SUN_DEF and SRV4_DEF) // AIX and Solaris
  	int const fd = open("/proc/self/psinfo", O_RDONLY);

  	if (fd != -1) { // Can't open?
  		return 0;
    }

    uintmax_t result = 0;
    struct psinfo psinfo;

  	if (read(fd, &psinfo, sizeof(psinfo)) == sizeof(psinfo)) { // Can read?
      result = psinfo.pr_rssize << UINTMAX_C(10);
  	}

  	close(fd);
  	return result;

  #elif UNIX_DEF or OSX_DEF // BSD, Linux, and OSX
  	struct rusage rusage;
  	getrusage(RUSAGE_SELF, &rusage);

  #if OSX_DEF
  	return rusage.ru_maxrss;
  #else
  	return rusage.ru_maxrss << UINTMAX_C(10);
  #endif

  #else // Unknown OS
  	return 0; // Unsupported

  #endif
  }

  /**
   * Returns the current resident set size (physical memory use) measured
   * in bytes, or zero if the value cannot be determined on this OS.
   */
  uintmax_t current (void) {
  #if WIN_DEF // Windows
  	PROCESS_MEMORY_COUNTERS info;
  	GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
  	return info.WorkingSetSize;

  #elif OSX_DEF // OSX
  	struct mach_task_basic_info info;
  	mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;

  	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
  		static_cast<task_info_t>(&info), &infoCount) != KERN_SUCCESS) {
  		return 0;		/* Can't access? */
    }

  	return info.resident_size;

  #elif LINUX_DEF // Linux
  	FILE* fp = fopen("/proc/self/statm", "r");

  	if (fp == NULL) { // Can't open?
  		return 0;
    }

    uintmax_t result = 0;

  	if (fscanf(fp, "%*s%ld", &result) == 1) { // Can read?
      result *= sysconf(_SC_PAGESIZE);
  	}

  	fclose(fp);
  	return result;

  #else // AIX, BSD, Solaris, and Unknown OS
  	return 0; // Unsupported

  #endif
  }

  #if UNIX_DEF or OSX_DEF
  bool limit (uintmax_t mem) {
    struct rlimit the_limit{ mem, RLIM_INFINITY };
    return setrlimit(RLIMIT_AS, &the_limit) == 0;
  }
  #endif
};
