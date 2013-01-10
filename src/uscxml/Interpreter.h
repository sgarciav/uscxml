#ifndef RUNTIME_H_SQ1MBKGN
#define RUNTIME_H_SQ1MBKGN

#include "uscxml/Common.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <set>
#include <map>

#include <XPath/XPath.hpp>
#include <DOM/Document.hpp>
#include <io/uri.hpp>

#include <DOM/SAX2DOM/SAX2DOM.hpp>
#include <SAX/helpers/CatchErrorHandler.hpp>

#include "uscxml/concurrency/tinythread.h"
#include "uscxml/concurrency/eventqueue/DelayedEventQueue.h"
#include "uscxml/concurrency/BlockingQueue.h"
#include "uscxml/Message.h"
#include "uscxml/Factory.h"

namespace uscxml {

class NumAttr {
public:
  NumAttr(const std::string& str) {
    size_t valueStart = str.find_first_of("0123456789.");
    if (valueStart != std::string::npos) {
      size_t valueEnd = str.find_last_of("0123456789.");
      if (valueEnd != std::string::npos) {
        value = str.substr(valueStart, (valueEnd - valueStart) + 1);
        size_t unitStart = str.find_first_not_of(" \t", valueEnd + 1);
        if (unitStart != std::string::npos) {
          size_t unitEnd = str.find_last_of(" \t");
          if (unitEnd != std::string::npos && unitEnd > unitStart) {
            unit = str.substr(unitStart, unitEnd - unitStart);
          } else {
            unit = str.substr(unitStart, str.length() - unitStart);
          }
        }
      }
    }
  }
  
  std::string value;
  std::string unit;
};
  
class Interpreter : protected Arabica::SAX2DOM::Parser<std::string> {
public:
	enum Binding {
	    EARLY = 0,
	    LATE = 1
	};

	virtual ~Interpreter();

	static Interpreter* fromDOM(const Arabica::DOM::Node<std::string>& node);
	static Interpreter* fromXML(const std::string& xml);
	static Interpreter* fromURI(const std::string& uri);
	static Interpreter* fromInputSource(Arabica::SAX::InputSource<std::string>& source);

  virtual void startPrefixMapping(const std::string& /* prefix */, const std::string& /* uri */);

	void start();
	static void run(void*);
	void join() {
		if (_thread != NULL) _thread->join();
	};

	void interpret();
	bool validate();

	void setBaseURI(std::string baseURI)                     {
		_baseURI = Arabica::io::URI(baseURI);
	}
  std::string getBaseURI()                                 {
		return _baseURI.as_string();
	}
	bool makeAbsolute(Arabica::io::URI& uri);

	DataModel* getDataModel()                                {
		return _dataModel;
	}
	Invoker* getInvoker()                                    {
		return _invoker;
	}
	void setInvoker(Invoker* invoker)                        {
		_invoker = invoker;
	}
	std::string getNSPrefix()                                {
		return _nsPrefix;
	}
  Arabica::XPath::StandardNamespaceContext<std::string>& getNSContext() {
		return _nsContext;
	}

	void waitForStabilization();

	void receive(Event& event)                               {
		_externalQueue.push(event);
	}
	void receiveInternal(Event& event)                       {
		_internalQueue.push_back(event);
	}
	Arabica::XPath::NodeSet<std::string> getConfiguration()  {
		return _configuration;
	}
	Arabica::DOM::Node<std::string> getState(const std::string& stateId);
	Arabica::DOM::Document<std::string>& getDocument()       {
		return _document;
	}

	const std::string& getName()                             {
		return _name;
	}
	const std::string& getSessionId()                        {
		return _sessionId;
	}

  bool runOnMainThread(int fps, bool blocking = true);
  
	static bool isMember(const Arabica::DOM::Node<std::string>& node, const Arabica::XPath::NodeSet<std::string>& set);

	void dump();
	static void dump(const Arabica::DOM::Node<std::string>& node, int lvl = 0);

	static bool isState(const Arabica::DOM::Node<std::string>& state);
	static bool isPseudoState(const Arabica::DOM::Node<std::string>& state);
	static bool isTransitionTarget(const Arabica::DOM::Node<std::string>& elem);
	static bool isTargetless(const Arabica::DOM::Node<std::string>& transition);
	static bool isAtomic(const Arabica::DOM::Node<std::string>& state);
	static bool isFinal(const Arabica::DOM::Node<std::string>& state);
	static bool isHistory(const Arabica::DOM::Node<std::string>& state);
	static bool isParallel(const Arabica::DOM::Node<std::string>& state);
	static bool isCompound(const Arabica::DOM::Node<std::string>& state);
	static bool isDescendant(const Arabica::DOM::Node<std::string>& s1, const Arabica::DOM::Node<std::string>& s2);

	bool isInitial(const Arabica::DOM::Node<std::string>& state);
	Arabica::DOM::Node<std::string> getInitialState(Arabica::DOM::Node<std::string> state = Arabica::DOM::Node<std::string>());
	static Arabica::XPath::NodeSet<std::string> getChildStates(const Arabica::DOM::Node<std::string>& state);
	Arabica::XPath::NodeSet<std::string> getTargetStates(const Arabica::DOM::Node<std::string>& transition);

  static Arabica::XPath::NodeSet<std::string> filterChildElements(const std::string& tagname, const Arabica::DOM::Node<std::string>& node);
	static const std::string getUUID();

protected:
	Interpreter();
	void init();

	void normalize(const Arabica::DOM::Document<std::string>& node);
	void setupIOProcessors();

	void mainEventLoop();

	bool _stable;
	tthread::thread* _thread;
	tthread::mutex _mutex;
	tthread::condition_variable _stabilized;

	Arabica::io::URI _baseURI;
	Arabica::DOM::Document<std::string> _document;
	Arabica::DOM::Element<std::string> _scxml;
	Arabica::XPath::XPath<std::string> _xpath;
	Arabica::XPath::StandardNamespaceContext<std::string> _nsContext;
	std::string _nsPrefix;

	bool _running;
	Binding _binding;
	Arabica::XPath::NodeSet<std::string> _configuration;
	Arabica::XPath::NodeSet<std::string> _statesToInvoke;

	DataModel* _dataModel;
	std::map<std::string, Arabica::XPath::NodeSet<std::string> > _historyValue;

	std::list<Event > _internalQueue;
	uscxml::concurrency::BlockingQueue<Event> _externalQueue;
	DelayedEventQueue* _sendQueue;
	Invoker* _invoker;

	static Arabica::io::URI toBaseURI(const Arabica::io::URI& uri);

	void microstep(const Arabica::XPath::NodeSet<std::string>& enabledTransitions);
	void exitStates(const Arabica::XPath::NodeSet<std::string>& enabledTransitions);
	void enterStates(const Arabica::XPath::NodeSet<std::string>& enabledTransitions);
	void executeTransitionContent(const Arabica::XPath::NodeSet<std::string>& enabledTransitions);
	void executeContent(const Arabica::DOM::Node<std::string>& content);
	void executeContent(const Arabica::DOM::NodeList<std::string>& content);
	void initializeData(const Arabica::DOM::Node<std::string>& data);
	void exitInterpreter();

	void addStatesToEnter(const Arabica::DOM::Node<std::string>& state,
	                      Arabica::XPath::NodeSet<std::string>& statesToEnter,
	                      Arabica::XPath::NodeSet<std::string>& statesForDefaultEntry);

	Arabica::XPath::NodeSet<std::string> selectEventlessTransitions();
	Arabica::XPath::NodeSet<std::string> selectTransitions(const std::string& event);
	Arabica::DOM::Node<std::string> getSourceState(const Arabica::DOM::Node<std::string>& transition);
	Arabica::DOM::Node<std::string> findLCCA(const Arabica::XPath::NodeSet<std::string>& states);
	static Arabica::XPath::NodeSet<std::string> getProperAncestors(const Arabica::DOM::Node<std::string>& s1, const Arabica::DOM::Node<std::string>& s2);


	void send(const Arabica::DOM::Node<std::string>& element);
	void invoke(const Arabica::DOM::Node<std::string>& element);
	void cancelInvoke(const Arabica::DOM::Node<std::string>& element);
	void returnDoneEvent(const Arabica::DOM::Node<std::string>& state);
	void internalDoneSend(const Arabica::DOM::Node<std::string>& state);
	static void delayedSend(void* userdata, std::string eventName);

	static bool nameMatch(const std::string& transitionEvent, const std::string& event);
	Arabica::XPath::NodeSet<std::string> filterPreempted(const Arabica::XPath::NodeSet<std::string>& enabledTransitions);
	bool hasConditionMatch(const Arabica::DOM::Node<std::string>& conditional);
	bool isPreemptingTransition(const Arabica::DOM::Node<std::string>& t1, const Arabica::DOM::Node<std::string>& t2);
	bool isInFinalState(const Arabica::DOM::Node<std::string>& state);
	bool isWithinSameChild(const Arabica::DOM::Node<std::string>& transition);
	bool parentIsScxmlState(Arabica::DOM::Node<std::string> state);

	static std::vector<std::string> tokenizeIdRefs(const std::string& idRefs);
  
	static boost::uuids::random_generator uuidGen;

  long _lastRunOnMainThread;
	std::string _name;
	std::string _sessionId;

	IOProcessor* getIOProcessor(const std::string& type);
//    IOProcessor* getIOProcessorForId(const std::string& sendId);

	std::map<std::string, IOProcessor*> _ioProcessors;
	std::map<std::string, std::pair<Interpreter*, SendRequest> > _sendIds;
	std::map<std::string, Invoker*> _invokers;

  /// We need to remember to adapt them when the DOM is operated upon
  std::map<std::string, Arabica::DOM::Node<std::string> > _cachedStates;
};

}

#endif
