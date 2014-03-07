/**
 *  @file
 *  @author     2012-2014 Stefan Radomski (stefan.radomski@cs.tu-darmstadt.de)
 *  @copyright  Simplified BSD
 *
 *  @cond
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the FreeBSD license as published by the FreeBSD
 *  project.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  You should have received a copy of the FreeBSD license along with this
 *  program. If not, see <http://www.opensource.org/licenses/bsd-license>.
 *  @endcond
 */

#ifndef DEBUGGERSERVLET_H_ATUMDA3G
#define DEBUGGERSERVLET_H_ATUMDA3G

#include "uscxml/Common.h"
#include "getopt.h"

#include "uscxml/server/HTTPServer.h"
#include "uscxml/Interpreter.h"

#include "uscxml/debug/Debugger.h"
#include "uscxml/concurrency/tinythread.h"

namespace uscxml {

class USCXML_API DebuggerServlet : public Debugger, public HTTPServlet {
public:
	virtual ~DebuggerServlet() {}
	
	// from Debugger
	virtual void addBreakpoint(const Breakpoint& breakpoint) {};

	bool isCORS(const HTTPServer::Request& request);
	void handleCORS(const HTTPServer::Request& request);

	bool httpRecvRequest(const HTTPServer::Request& request);
	void setURL(const std::string& url) {
		_url = url;
	}

	void pushData(Data pushData);
	void returnData(const HTTPServer::Request& request, Data replyData);

	void hitBreakpoint(const Interpreter& interpreter,
										 Data data);
	
	void processDebugEval(const HTTPServer::Request& request);
	void processDebugPrepare(const HTTPServer::Request& request);
	void processDebugStart(const HTTPServer::Request& request);
	void processDebugStop(const HTTPServer::Request& request);
	void processDebugStep(const HTTPServer::Request& request);
	void processDebugResume(const HTTPServer::Request& request);
	void processDebugPause(const HTTPServer::Request& request);
	void processConnect(const HTTPServer::Request& request);
	void processListSessions(const HTTPServer::Request& request);
	void processDisconnect(const HTTPServer::Request& request);
	void processAddBreakPoint(const HTTPServer::Request& request);
	void processRemoveBreakPoint(const HTTPServer::Request& request);
	void processPoll(const HTTPServer::Request& request);
  
protected:
	void serverPushData();

	Interpreter _interpreter;
	std::string _sessionId;
	std::string _url;
	HTTPServer::Request _clientConn;
	tthread::condition_variable _resumeCond;
	tthread::recursive_mutex _mutex;
	concurrency::BlockingQueue<Data> _sendQueue;
};

}

#endif /* end of include guard: DEBUGGERSERVLET_H_ATUMDA3G */