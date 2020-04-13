#pragma once

#ifndef __WINAPI_THREAD_ADAPTOR_H__
#define __WINAPI_THREAD_ADAPTOR_H__

#include <Windows.h>
#include <stdint.h>

class WinapiThreadAdaptor
{
public:

   enum class ThreadState
   {
      UNDEFINED = -1,
      SUSPENDED = 0,
      RESUMED = 1
   };

   WinapiThreadAdaptor(uint32_t (*threadFunc)(void* arg), void* threadArg,
      ThreadState threadState = ThreadState::RESUMED);
   ~WinapiThreadAdaptor();
   void suspend(void);
   void resume(void);
   ThreadState getThreadState(void);
   bool isSuspended(void) const;
   bool isResumed(void) const;
   void join(void);
   uint32_t getLastError(void) const;
   static void sleep(uint32_t interval);

private:
   HANDLE thread;
   ThreadState threadState;
};

#endif // __WINAPI_THREAD_ADAPTOR_H__