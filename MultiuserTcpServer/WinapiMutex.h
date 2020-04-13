#pragma once

#ifndef __WINAPI_MUTEX_H__
#define __WINAPI_MUTEX_H__

#include <Windows.h>

class WinapiMutex
{
public:
   WinapiMutex();
   ~WinapiMutex();
   void lock(void);
   void unlock(void);

private:
   CRITICAL_SECTION cs;
};

#endif // __WINAPI_MUTEX_H__