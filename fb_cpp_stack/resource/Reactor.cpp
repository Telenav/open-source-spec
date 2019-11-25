// REF: http://www.dre.vanderbilt.edu/~schmidt/PDF/reactor-siemens.pdf

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>

#include <map>

typedef void * HANDLE;

//The Logging Handler is response for receiving and process Logging records
//The Logging Acceptor creates and connects Logging Handles that process subsequet
// loging records from clients
enum eEventType
{
    eAcceptEvent = 01,
    eReadEvent = 02,
    eWriteEvent = 04,
    eTimeoutEvent = 010,
    eSignalEvent = 020,
    eCloseEvent =040
};

class EventHandle
{
public:

// Hook methods that are called back by
// the Initiation_Dispatcher to handle
// particular types of events.

// Avoid switchs, but need user to anticipate the set 
// of event hander menthods in advance

//    virtual int HandleAccpet() = 0;
//    virtual int HandleInput() = 0;
//    virtual int HandleOutput() = 0;
//    virtual int HandleTimeout() = 0;
//    virtual int HandleClose() = 0;

// There need a switch in subclass
    virtual void HandleEvents(eEventType& et) = 0;

    virtual HANDLE GetHandle() const = 0;
};

class Time_Value
{

};

//TITLE
//Demulitplex and dispatch Event_Handles
//in response to client request
class InitiationDispatcher
{
public:
    static InitiationDispatcher* Instance();
    static InitiationDispatcher* m_sDispatcher;
public:
    int RegisterHandle(EventHandle *eh, eEventType et);
    int RemoveHandle(EventHandle* eh, eEventType et);

    // Entry point into the reactive event loop.
    // The main
    // event loop of the entire application is controlled by this entry
    // point.
    // When events occur, the Initiation Dispatcher
    // returns from the synchronous event demultiplexing call
    // and “reacts” by dispatching the Event Handler’s
    // handle event hook method for each handle that is
    // “ready.” This hook method executes user-defined code and
    // returns control to the Initiation Dispatcher when it
    // completes.
    int HandleEvents(Time_Value* timeout = 0);
private:
    InitiationDispatcher(){
    };

    // a table of Concrete Event Handler
    typedef std::map<EventHandle*, eEventType> MpEvent;
    MpEvent m_mpEvent;
};

InitiationDispatcher*  InitiationDispatcher::m_sDispatcher = NULL;

InitiationDispatcher* InitiationDispatcher::Instance()
{
    if(m_sDispatcher == NULL)
        m_sDispatcher = new InitiationDispatcher;
    return m_sDispatcher;
}
int InitiationDispatcher::RegisterHandle(EventHandle* sh, eEventType et){
    m_mpEvent[sh] = et;
    return 0;
}

int InitiationDispatcher::RemoveHandle(EventHandle* eh, eEventType et){
    return 0;
}

int InitiationDispatcher:: HandleEvents(Time_Value* timeout){
    MpEvent::iterator it ;
    for(it = m_mpEvent.begin(); it!= m_mpEvent.end(); it++){
        EventHandle* pEvent = it->first;
        pEvent->HandleEvents(it->second);
    }
    return 0;
}


typedef sockaddr_in SockStream;


class SockAcceptor
{
public:
    virtual HANDLE GetHandle() const {
        return m_handle;
    }
    virtual void Accept(const SockStream& stream){

    }
private:
    HANDLE* m_handle;
};

class LoggingHandle: public EventHandle
{
public:

        // Initialize the acceptor_ endpoint and
// register with the Initiation Dispatcher.
    LoggingHandle(const SockStream& stream): m_peerStream(stream){
        InitiationDispatcher::Instance()->RegisterHandle(this, eReadEvent);
    }


    // Factory method that accepts a new
// SOCK_Stream connection and creates a
// Logging_Handler object to handle logging
// records sent using the connection.
    virtual void  HandleEvents(eEventType& et) {
    if (et == READ_EVENT) {
        Log_Record log_record;
        peer_stream_.recv ((void *) log_record, sizeof log_record);
        // Write logging record to standard output.
        log_record.write (STDOUT);
        }
    else if (et == CLOSE_EVENT) {
        peer_stream_.close ();
        delete (void *) this;
        }
    }


    // Get the I/O Handle (called by the
    // Initiation Dispatcher when
    // Logging_Acceptor is registered)
    virtual HANDLE GetHandle() const{
        return m_peerStream.get_handle();
    }

private:
    // Socket factory that accepts clients
    // connections.
    SockStream m_peerStream;
};

class LoggingAcceptor: public Event_Handler
{
public:

    LoggingAcceptor(const SockStream& addr)
    : acceptor_(addr)
    {
        // Register acceptor with the Initiation
        // Dispatcher, which "double dispatches"
        // the Logging_Acceptor::get_handle() method
        // to obtain the HANDLE.
        Initiation_Dispatcher::instance()->register_handler(this, ACCEPT_EVENT);
    }

    virtual void HandleEvents(eEventType et)
    {
        assert(et == eAcceptEvent);
        SockStream newConnection;

        // accept and create handler
        m_acceptor->Accept(newConnection);
        LoggingHandle* lh = new LoggingHandle(newConnection);
        // register_handler(lh, READ_EVENT)
        // get_handle
        // handle_event(READ_EVENT)
    }

    virtual HANDLE gethandle() const {
        return m_acceptor->GetHandle();
    }
private:

    SockAcceptor* m_acceptor;
};

const u_short uport = 10000;


int main(int argc, char *argv[])
{
    SockStream  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(uport);

    LoggingAcceptor la(addr);
    for(;;)
        InitiationDispatcher::Instance()->HandleEvents();
    return 0;
}
