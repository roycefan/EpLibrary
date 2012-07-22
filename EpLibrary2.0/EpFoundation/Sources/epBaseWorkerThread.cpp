/*! 
BaseWorkerThread for the EpLibrary
Copyright (C) 2012  Woong Gyu La <juhgiyo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "epBaseWorkerThread.h"
#include "epSmartObject.h"
#include "epWorkerThreadDelegate.h"

using namespace epl;

BaseWorkerThread::BaseWorkerThread(const ThreadLifePolicy policy,LockPolicy lockPolicyType) :Thread(NULL,lockPolicyType)
{
	m_lifePolicy=policy;
	m_callBackClass=NULL;
	m_lockPolicy=lockPolicyType;
	switch(lockPolicyType)
	{
	case LOCK_POLICY_CRITICALSECTION:
		m_callBackLock=EP_NEW CriticalSectionEx();
		break;
	case LOCK_POLICY_MUTEX:
		m_callBackLock=EP_NEW Mutex();
		break;
	case LOCK_POLICY_NONE:
		m_callBackLock=EP_NEW NoLock();
		break;
	default:
		m_callBackLock=NULL;
		break;
	}
}

BaseWorkerThread::BaseWorkerThread(const BaseWorkerThread & b):Thread(b)
{
	m_lifePolicy=b.m_lifePolicy;
	m_callBackClass=b.m_callBackClass;
	m_lockPolicy=b.m_lockPolicy;
	switch(m_lockPolicy)
	{
	case LOCK_POLICY_CRITICALSECTION:
		m_callBackLock=EP_NEW CriticalSectionEx();
		break;
	case LOCK_POLICY_MUTEX:
		m_callBackLock=EP_NEW Mutex();
		break;
	case LOCK_POLICY_NONE:
		m_callBackLock=EP_NEW NoLock();
		break;
	default:
		m_callBackLock=NULL;
		break;
	}
}
BaseWorkerThread::~BaseWorkerThread()
{
	if(GetArg())
	{
		(reinterpret_cast<SmartObject*>(GetArg()))->ReleaseObj();
	}
	while(!m_workPool.IsEmpty())
	{
		m_workPool.Front()->JobReport(BaseJob::JOB_STATUS_INCOMPLETE);
		m_workPool.Pop();
	}
	if(m_callBackLock)
		EP_DELETE m_callBackLock;
}

void BaseWorkerThread::Push(BaseJob* const  work)
{
	m_workPool.Push(work);
}

BaseJob * &BaseWorkerThread::Front()
{
	return m_workPool.Front();
}
void BaseWorkerThread::Pop()
{
	m_workPool.Pop();
}

bool BaseWorkerThread::Erase(BaseJob *const work)
{
	bool retVal=false;
	if(m_workPool.Erase(work))
	{
		work->ReleaseObj();
		retVal=true;
	}
	return retVal;
}
int BaseWorkerThread::GetJobCount() const
{
	return m_workPool.Size();
}
void BaseWorkerThread::setArg(void* a)
{
	if(GetStatus()!=THREAD_STATUS_STARTED)
	{
		if(GetArg())
			(reinterpret_cast<SmartObject*>(GetArg()))->ReleaseObj();
		Thread::setArg(a);
		if(GetArg())
			(reinterpret_cast<SmartObject*>(GetArg()))->RetainObj();
	}
	else
	{
		EP_ASSERT_EXPR(0,_T("Cannot Set Argument during Thread Running!"));
	}
}
void BaseWorkerThread::SetCallBackClass(void *callBackClass)
{
	LockObj lock(m_callBackLock);
	m_callBackClass=callBackClass;
}

void BaseWorkerThread::callCallBack()
{
	LockObj lock(m_callBackLock);
	if(m_callBackClass)	
		(reinterpret_cast<WorkerThreadDelegate*>(m_callBackClass))->CallBackFunc(this);
	m_callBackClass=NULL;
}