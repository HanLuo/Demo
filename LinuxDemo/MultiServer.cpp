#include "MultiServer.h"

Conn::Conn(int fd) : m_fd(fd)
{
	m_Prev = NULL;
	m_Next = NULL;
}

Conn::~Conn()
{

}

ConnQueue::ConnQueue()
{
	m_head = new Conn(0);
	m_tail = new Conn(0);
	m_head->m_Prev = m_tail->m_Next = NULL;
	m_head->m_Next = m_tail;
	m_tail->m_Prev = m_head;
}

ConnQueue::~ConnQueue()
{
	Conn *tcur, *tnext;
	tcur = m_head;

	while (tcur != NULL)
	{
		tnext = tcur->m_Next;
		delete tcur;
		tcur = tnext;
	}
}

Conn *ConnQueue::InsertConn(int fd, LibeventThread *t)
{
	Conn *c = new Conn(fd);
	c->m_Thread = t;
	Conn *next = m_head->m_Next;

	c->m_Prev = m_head;
	c->m_Next = m_head->m_Next;
	m_head->m_Next = c;
	next->m_Prev = c;
	return c;
}

void ConnQueue::DeleteConn(Conn *c)
{
	c->m_Prev->m_Next = c->m_Next;
	c->m_Next->m_Prev = c->m_Prev;
	delete c;
}

MultiServer::MultiServer(int count)
{
	m_ThreadCount = count;
	m_Port = -1;
	m_MainBase = new LibeventThread;
	m_Threads = new LibeventThread[m_ThreadCount];
	m_MainBase->tid = pthread_self();
	m_MainBase->base = event_base_new();
	memset(m_SignalEvents, 0, sizeof(m_SignalEvents));

	//初始化各个子线程的结构体
	for(int i=0; i<m_ThreadCount; i++)
	{
		SetupThread(&m_Threads[i]);
	}
}

MultiServer::~MultiServer()
{
	// 停止事件循环（如果时间循环没开始，则没有效果）
	StopRun(NULL);

	// 释放内存
	event_base_free(m_MainBase->base);
	for (int i = 0; i < m_ThreadCount; ++i)
		event_base_free(m_Threads[i].base);

	delete m_MainBase;
	delete [] m_Threads;
}

void MultiServer::SetupThread(LibeventThread *me)
{
	int res;

	// 建立libevent事件处理机制
	me->tcpConnect = this;
	me->base = event_base_new();
	assert(me->base != NULL);

	// 在主线程和子线程之间建立管道
	int fds[2];
	res = pipe(fds);
	assert( res == 0 )
	me->notifyReceiveFd = fds[0];
	me->notifySendFd = fds[1];

	// 让子线程的状态机监听管道
	event_set(&me->notifyEvent, me->notifyReceiveFd, EV_READ|EV_PERSIST, ThreadProcess, me);
	event_base_set(me->base, &me->notifyEvent);
	res = event_add(&me->notifyEvent, 0);
	assert( res == 0);
}

void *MultiServer::WorkerLibevent(void *arg)
{
	// 开始libevent的事件循环,准备处理业务
	LibeventThread *me = (LibeventThread*)arg;
	event_base_dispatch(me->base);
}

bool MultiServer::StartRun()
{
	evconnlistener *listener;

	// 如果端口号不是EXIT_CODE，就监听端口号
	if (m_Port != EXIT_CODE)
	{
		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(m_Port);
		listener = evconnlistener_new_bind(m_MainBase->base,
			ListenerEventCb, (void*)this,
			LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
			(sockaddr*)&sin, sizeof(sockaddr_in));
		if( NULL == listener )
		{
			fprintf(stderr, "listen error: %s\n", strerror(errno));
			exit(1);
		}
	}

	// 开启各个子线程
	for(int i=0; i<m_ThreadCount; i++)    
	{    
		pthread_create(&m_Threads[i].tid, NULL,      
			WorkerLibevent, (void*)&m_Threads[i]);    
	}    
  
	//开启主线程的事件循环    
	event_base_dispatch(m_MainBase->base);    
  
	//事件循环结果，释放监听者的内存    
	if( m_Port != EXIT_CODE )    
	{    
		//printf("free listen\n");    
		evconnlistener_free(listener);    
	}
}

void MultiServer::StopRun(timeval *tv)    
{    
	int contant = EXIT_CODE;    
	//向各个子线程的管理中写入EXIT_CODE，通知它们退出    
	for(int i=0; i<m_ThreadCount; i++)    
	{    
		write(m_Threads[i].notifySendFd, &contant, sizeof(int));    
	}    
	//结果主线程的事件循环    
	event_base_loopexit(m_MainBase->base, tv);    
}    
  
void MultiServer::ListenerEventCb(struct evconnlistener *listener,     
									 evutil_socket_t fd,    
struct sockaddr *sa,     
	int socklen,     
	void *user_data)    
{    
	MultiServer *server = (MultiServer*)user_data;    
  
	//随机选择一个子线程，通过管道向其传递socket描述符    
	int num = rand() % server->m_ThreadCount;    
	int sendfd = server->m_Threads[num].notifySendFd;    
	write(sendfd, &fd, sizeof(evutil_socket_t));    
}    
  
void MultiServer::ThreadProcess(int fd, short which, void *arg)    
{    
	LibeventThread *me = (LibeventThread*)arg;    
  
	//从管道中读取数据（socket的描述符或操作码）    
	int pipefd = me->notifyReceiveFd;    
	evutil_socket_t confd;    
	read(pipefd, &confd, sizeof(evutil_socket_t));    
  
	//如果操作码是EXIT_CODE，则终于事件循环    
	if( EXIT_CODE == confd )    
	{    
		event_base_loopbreak(me->base);    
		return;    
	}    
  
	//新建连接    
	struct bufferevent *bev;    
	bev = bufferevent_socket_new(me->base, confd, BEV_OPT_CLOSE_ON_FREE);    
	if (!bev)    
	{    
		fprintf(stderr, "Error constructing bufferevent!");    
		event_base_loopbreak(me->base);    
		return;    
	}    
  
	//将该链接放入队列    
	Conn *conn = me->connectQueue.InsertConn(confd, me);    
  
	//准备从socket中读写数据    
	bufferevent_setcb(bev, ReadEventCb, WriteEventCb, CloseEventCb, conn);    
	bufferevent_enable(bev, EV_WRITE);    
	bufferevent_enable(bev, EV_READ);    
  
	//调用用户自定义的连接事件处理函数    
	me->tcpConnect->ConnectionEvent(conn);    
}    
  
void MultiServer::ReadEventCb(struct bufferevent *bev, void *data)    
{    
	Conn *conn = (Conn*)data;    
	conn->m_ReadBuf = bufferevent_get_input(bev);    
	conn->m_WriteBuf = bufferevent_get_output(bev);    
  
	//调用用户自定义的读取事件处理函数    
	conn->m_Thread->tcpConnect->ReadEvent(conn);    
}     
  
void MultiServer::WriteEventCb(struct bufferevent *bev, void *data)    
{    
	Conn *conn = (Conn*)data;    
	conn->m_ReadBuf = bufferevent_get_input(bev);    
	conn->m_WriteBuf = bufferevent_get_output(bev);    
  
	//调用用户自定义的写入事件处理函数    
	conn->m_Thread->tcpConnect->WriteEvent(conn);    
  
}    
  
void MultiServer::CloseEventCb(struct bufferevent *bev, short events, void *data)    
{    
	Conn *conn = (Conn*)data;    
	//调用用户自定义的断开事件处理函数    
	conn->m_Thread->tcpConnect->CloseEvent(conn, events);    
	conn->GetThread()->connectQueue.DeleteConn(conn);    
	bufferevent_free(bev);    
}    
  
bool MultiServer::AddSignalEvent(int sig, void (*ptr)(int, short, void*))    
{    
	if( sig >= MAX_SIGNAL )  
		return false;  
  
	//新建一个信号事件    
	event *ev = evsignal_new(m_MainBase->base, sig, ptr, (void*)this);    
	if ( !ev ||     
		event_add(ev, NULL) < 0 )    
	{    
		event_del(ev);    
		return false;    
	}    
  
	//删除旧的信号事件（同一个信号只能有一个信号事件）   
	if( NULL != m_SignalEvents[sig] )  
		DeleteSignalEvent(sig);    
	m_SignalEvents[sig] = ev;    
  
	return true;    
}    
  
bool MultiServer::DeleteSignalEvent(int sig)    
{    
	event *ev = m_SignalEvents[sig];  
	if( sig >= MAX_SIGNAL || NULL == ev )  
		return false;  
  
	event_del(ev);    
	ev = NULL;  
	return true;    
}    
  
event *MultiServer::AddTimerEvent(void (*ptr)(int, short, void *),     
									 timeval tv, bool once)    
{    
	int flag = 0;    
	if( !once )    
		flag = EV_PERSIST;    
  
	//新建定时器信号事件    
	event *ev = new event;    
	event_assign(ev, m_MainBase->base, -1, flag, ptr, (void*)this);    
	if( event_add(ev, &tv) < 0 )    
	{    
		event_del(ev);    
		return NULL;    
	}    
	return ev;    
}    
  
bool MultiServer::DeleteTImerEvent(event *ev)    
{    
	int res = event_del(ev);    
	return (0 == res);    
}