#include "thread.h"

namespace QtPth
{

Thread::Thread(bool destroyOnDone,
               QObject *parent) :
    QObject(parent),
    mThread(NULL),
    mDestroyOnDone(destroyOnDone),
    mHandler(NULL)
{
}

Thread::~Thread()
{
    if(mThread)
    {
        pth_join(mThread, NULL);
    }
    DLOG("Destroyed");
}

void Thread::yield()
{
    Q_ASSERT(mThread);
    DLOG("Yield");
    pth_yield(mThread);
}

void Thread::kill()
{
    Q_ASSERT(mThread);
    DLOG("Kill");
    pth_cancel(mThread);
    if(mDestroyOnDone)
        deleteLater();
}

void Thread::nap(qreal sec)
{
    pth_nap(pth_time(0, qint64(sec * 1000000.0)));
}

void Thread::sleep(qreal sec)
{
    pth_usleep(qint64(sec * 1000000.0));
}

Thread* Thread::create(QObject* parent,
                       bool destroyOnDone)
{
    return new Thread(destroyOnDone, parent);
}

Thread* Thread::current()
{
    pth_t th = pth_self();
    Q_ASSERT(th);
    pth_attr_t attr = pth_attr_of(th);
    void* curTh = NULL;
    pth_attr_get(attr, PTH_ATTR_START_ARG, &curTh);
    Thread* result = static_cast<Thread*>(curTh);
    QTPTH_ASSERT(result, "Current thread was not started");
    return result;
}

void* Thread::pthRun(void* ctx)
{
    static_cast<Thread*>(ctx)->run();
}

void Thread::run()
{
    try
    {
        if(mHandler)
        {
            DLOG("Running...");
            mHandler();
            DLOG("Done");
        }
        else
        {
            WLOG("Thread method is not specified");
        }
    }
    catch(std::exception& e)
    {
        if(mExcept)
        {
            mExcept(ExecutionException(
                        e, "Thread execution"));
        }
        else
        {
            ELOG("Unhandled exception: %1", e);
        }
    }
    if(mDestroyOnDone)
        deleteLater();
}

}

