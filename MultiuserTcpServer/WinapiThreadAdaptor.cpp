#include "WinapiThreadAdaptor.h"

WinapiThreadAdaptor::WinapiThreadAdaptor(uint32_t (*threadFunc)(void* arg), void* threadArg,
   ThreadState threadState)
{
   this->threadState = threadState;
   this->thread = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)threadFunc, threadArg,
      (threadState == ThreadState::RESUMED ? 0 : CREATE_SUSPENDED), nullptr);

}

WinapiThreadAdaptor::~WinapiThreadAdaptor()
{
   if (this->thread != nullptr)
   {
      ::CloseHandle(this->thread);
      this->thread = nullptr;
   }
}

void WinapiThreadAdaptor::suspend(void)
{
   if (this->threadState != ThreadState::SUSPENDED)
   {
      ::SuspendThread(this->thread);
      this->threadState = ThreadState::SUSPENDED;
   }
}

void WinapiThreadAdaptor::resume(void)
{
   if (this->threadState != ThreadState::RESUMED)
   {
      ::ResumeThread(this->thread);
      this->threadState = ThreadState::RESUMED;
   }
}
WinapiThreadAdaptor::ThreadState WinapiThreadAdaptor::getThreadState(void)
{
   return this->threadState;
}

bool WinapiThreadAdaptor::isSuspended(void) const
{
   return this->threadState == ThreadState::SUSPENDED;
}

bool WinapiThreadAdaptor::isResumed(void) const
{
   return this->threadState == ThreadState::RESUMED;
}

void WinapiThreadAdaptor::join(void)
{
   ::WaitForSingleObject(this->thread, INFINITE);
}

uint32_t WinapiThreadAdaptor::getLastError(void) const
{
   return GetLastError();
}

void WinapiThreadAdaptor::sleep(uint32_t interval)
{
   ::Sleep(interval);
}