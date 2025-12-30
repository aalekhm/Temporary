#pragma once
#include "Singleton.h"
#include <string>
#include <map>
#include <vector>

#define MESSAGE_SEND(__stringKey__, __messageData__) \
GameMessaging::Get().MessageSend(__stringKey__, std::move(__messageData__));

#define MESSAGE_POST(__stringKey__, __messageData__) \
GameMessaging::Get().MessagePost(__stringKey__, std::move(__messageData__));

#define IMessageDataWrapper(__className__, __varType__) \
class __className__ : public IMessageDataPart \
{ \
	public: \
		virtual void			ImplementMe() {}; \
		__varType__				m_Var; \
}; \

#define CREATE_MESSAGEDATA(__messageData__) \
std::unique_ptr<MessageData> __messageData__ = std::make_unique<MessageData>(); \

#define CREATE_MESSAGEDATAPART(__className__, __messageDataPart__) \
std::unique_ptr<IMessageDataPart> __messageDataPart__ = std::make_unique<__className__>();

#define ADD_MESSAGEDATAPART(__messageData__, __stringKey__, __value__) \
__messageData__.get()->m_vMessageDataPart[__stringKey__] = std::move(__value__); \

#define CREATE_MESSAGE_AND_ADD_MESSAGEDATAPART(__messageData__, __className__, __key__, __value__) \
CREATE_MESSAGEDATA(__messageData__); \
{ \
	CREATE_MESSAGEDATAPART(__className__, defaultVar); \
	{ \
		((__className__*)defaultVar.get())->m_Var = __value__; \
	} \
	ADD_MESSAGEDATAPART(__messageData__, __key__, defaultVar); \
} \

#define ADDITIONAL_MESSAGEDATAPART(__messageData__, __className__, __key__, __value__) \
	CREATE_MESSAGEDATAPART(__className__, defaultVar); \
	{ \
		((__className__*)defaultVar.get())->m_Var = __value__; \
	} \
	ADD_MESSAGEDATAPART(__messageData__, __key__, defaultVar); \

#define GET_MESSAGEDATAPART(__MessageDataPtr__, __className__, __key__, __varOut__) \
{ \
	auto dataItr = __MessageDataPtr__->m_vMessageDataPart.find(__key__); \
	if (dataItr != __MessageDataPtr__->m_vMessageDataPart.end()) \
	{ \
		if (dataItr->first == __key__) \
		{ \
			__varOut__ = ((__className__*)(dataItr->second.get()))->m_Var; \
		} \
	} \
} \


class IMessageDataPart
{
	public:
		virtual void	ImplementMe() = 0;
		virtual			~IMessageDataPart() {};
};

class MessageData
{
	public:
		virtual														~MessageData() {};
		std::map<std::string, std::unique_ptr<IMessageDataPart>>	m_vMessageDataPart;
};

class IGameMessageHandler
{
	public:
		virtual void	HandleMessage(std::string, const MessageData* pMessageData) = 0;
};

IMessageDataWrapper(StringMessageData, std::string)

typedef	std::vector<IGameMessageHandler*>						GAMEMESSAGEHANDLERS;
typedef std::map<std::string, GAMEMESSAGEHANDLERS>				GAMEMESSAGEHANDLER_LIST;
typedef std::map<std::string, std::unique_ptr<MessageData>>		GAMEMESSAGE_POSTDATA;

class GameMessaging : public Singleton<GameMessaging>
{
	public:
		void								AddListener(std::string sMessage, IGameMessageHandler* pGameMessageHandler);
		void								RemoveListener(std::string sMessage, IGameMessageHandler* pGameMessageHandler);
		bool								IsRegistered(std::string sMessage, IGameMessageHandler* pGameMessageHandler);
		void								MessageSend(std::string sMessage, std::unique_ptr<MessageData> pData);
		void								MessagePost(std::string sMessage, std::unique_ptr<MessageData> pData);

		void								ProcessQueue();
	protected:

	private:
		GAMEMESSAGEHANDLER_LIST				m_HandlerList;
		GAMEMESSAGE_POSTDATA				m_PostDataList;
};