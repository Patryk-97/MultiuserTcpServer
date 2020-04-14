#include "WinapiMutex.h"

WinapiMutex::WinapiMutex()
{
   InitializeCriticalSection(&this->cs);
}

WinapiMutex::~WinapiMutex()
{
   DeleteCriticalSection(&this->cs);
}

void WinapiMutex::lock(void)
{
   EnterCriticalSection(&this->cs);
}

void WinapiMutex::unlock(void)
{
   LeaveCriticalSection(&this->cs);
}