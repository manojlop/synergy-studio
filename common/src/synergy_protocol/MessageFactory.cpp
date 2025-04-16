#include "../../include/synergy_protocol/MessageFactory.h"

using namespace SynergyProtocol;

std::unique_ptr<Message_Base> MessageFactory::createMessage(const QJsonObject& jsonObject) const {
  if(!jsonObject.contains("type") || !jsonObject["type"].isString()) {
    qCritical() << "MESSAGE FACTORY | JSON missing 'type' string";
    return nullptr;
  }

  QString messageTypeStr = jsonObject["type"].toString();
  if(m_creators.contains(messageTypeStr)) {
    // We create empty object using registered function
    std::unique_ptr<Message_Base> message = m_creators[messageTypeStr]();

    // Populate it using its virtual fromJson method
    if(message && message->fromJson(jsonObject)){
      // created and parsed
      return message; // ownership transfered via unique_ptr
    } else {
      qCritical() << "MESSAGE FACTORY | Failed to parse JSON for type" << messageTypeStr;
      return nullptr;
    }
  } else {
    qCritical() << "MESSAGE FACTORY | Unknown message type encountered";
    return nullptr;
  }
}

// We map type string to function that creates the corresponding C++ object
void MessageFactory::registerMessages() {
  m_creators.insert(messageTypeToString(t_MessageType::JOIN_SESSION_REQUEST),
                    []() { return std::make_unique<Message_Join_Session_Request>(); });

  
  // m_creators.insert(messageTypeToString(t_MessageType::TEXT_EDIT),
  //                   []() { return std::make_unique<Message_Text_Edit>(); });

  // ... register ALL further message types here ...

  qInfo() << "MessageFactory: Registered" << m_creators.count() << "message types.";
}