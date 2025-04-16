#ifndef __SYNERGY_PROTOCOL_MESSAGE_FACTOR__
#define __SYNERGY_PROTOCOL_MESSAGE_FACTOR__

#include <functional>
#include <memory>
#include <QJsonObject>
#include <QMap>

#include "protocol.h"
#include "Message_Base.h"

#include "Message_Join_Session_Request.h"

namespace SynergyProtocol {
  using MessageCreatorFunc = std::function<std::unique_ptr<Message_Base>(qintptr id)>;

  class MessageFactory {
  public:
    // We can do two approaches: Singleton or static class aproach, which we chose here
    static MessageFactory& instance() {
      static MessageFactory factory;
      // Registration happens once, only on startup
      if(!factory.m_initialized) {
        factory.registerMessages();
        factory.m_initialized = true;
      }
      return factory;
    }

    std::unique_ptr<Message_Base> createMessage(const QJsonObject& jsonObject, qintptr id) const;
  
  private:
    MessageFactory() = default; // Private constructor for singleton-like pattern
    ~MessageFactory() = default;
    MessageFactory(const MessageFactory&) = delete;
    MessageFactory& operator=(const MessageFactory&) = delete;

    void registerMessages(); // Private helper to register all types

    QMap<QString, MessageCreatorFunc> m_creators;
    bool m_initialized = false;
  };
}

#endif