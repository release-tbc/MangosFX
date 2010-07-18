#include <Policies/SingletonImp.h>
#include <omniORB4/CORBA.h>
#include "CORBAThread.h"

INSTANTIATE_SINGLETON_1( CORBAThread );
volatile bool CORBAThread::m_stopEvent = false;
uint8 CORBAThread::m_ExitCode = SHUTDOWN_EXIT_CODE;

CORBAThread::CORBAThread()
{
}

CORBAThread::~CORBAThread()
{
}

void CORBAThread::run()
{
	uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();
	uint32 prevSleepTime = 0;                               // used for balanced full tick time length near WORLD_SLEEP_CONST

	int argc = 0;
	char** argv = NULL;
	sLog.outDetail("Initializing CORBA...");
	CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB4");

	sLog.outDetail("Initializing CORBA POA...");
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");

	sLog.outDetail("CORBA Thread launched successfuly !");
	while(!MustStop())
	{
		// Updates
		realCurrTime = getMSTime();
        uint32 diff = getMSTimeDiff(realPrevTime,realCurrTime);

		// Main update there if need
		realPrevTime = realCurrTime;
		if (diff <=  CLUSTER_SLEEP_CONST+prevSleepTime)
        {
            prevSleepTime = CLUSTER_SLEEP_CONST+prevSleepTime-diff;
            ACE_Based::Thread::Sleep(prevSleepTime);
        }
        else
            prevSleepTime = 0;
	}
}

void CORBAThread::Wait()
{
	while(!MustStop())
	{
		//ACE_Based::Thread::Sleep(1000);
	}
}