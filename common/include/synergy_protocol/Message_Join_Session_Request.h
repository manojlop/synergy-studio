#ifndef __SYNERGY_PROTOCOL_MESSAGE_JOIN_SESSION_REQUEST__
#define __SYNERGY_PROTOCOL_MESSAGE_JOIN_SESSION_REQUEST__

#include "protocol.h"
#include "Message_Base.h"
#include <utility>

namespace SynergyProtocol {

  class Message_Join_Session_Request : public SynergyProtocol::Message_Base {
  public:
  SynergyProtocol::t_MessageType type() const override { return SynergyProtocol::t_MessageType::JOIN_SESSION_REQUEST; }

    // Const getters provide read-only access
    const QString& sessionIdToJoin() const { return m_session_id_to_join; }
    bool shouldCreateNew() const { return m_create_new; }
    const QString& username() const { return m_username; }

    explicit Message_Join_Session_Request(qintptr id, QString sid = "", bool create = false, QString name = "") :
      Message_Base(id),
      m_session_id_to_join(std::move(sid)),
      m_create_new(create),
      m_username(std::move(name)) {}

  protected:
    QString m_session_id_to_join;
    bool m_create_new;
    QString m_username;

    virtual QJsonObject payloadToJson() const override;

    virtual bool payloadFromJson(const QJsonObject& payloadObj) override;
  };
}

#endif