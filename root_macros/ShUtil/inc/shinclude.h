#ifndef SH_INCLUDE_H
#define SH_INCLUDE_H


#include "inc/util.h"
#include "inc/rootutil.h"
#include "inc/physutil.h"

#include "inc/shStyle.h"
#include "inc/histmanager.h"

#ifdef __CLING__
#include "src/util.cc"
#include "src/rootutil.cc"
#include "src/physutil.cc"

#include "src/shStyle.cc"
#include "src/histmanager.C"
#endif

#endif // SH_INCLUDE_H
