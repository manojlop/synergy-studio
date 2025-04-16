#ifndef __SYNERGY_PROTOCOL_MESSAGE_BASE__
#define __SYNERGY_PROTOCOL_MESSAGE_BASE__

#include "protocol.h"

namespace SynergyProtocol {

  class Message_Base {
  public: 

    virtual ~Message_Base() = default;

    virtual SynergyProtocol::t_MessageType type() const = 0;
    SynergyProtocol::t_Versions version() const { return m_version; }
    void setVersion(const SynergyProtocol::t_Versions& version) { m_version = version; }
    qintptr clientId() const { return m_id; }

    // Serialize entire message to JSON
    virtual QJsonObject toJSon() const {
      QJsonObject obj;
      obj["version"] = SynergyProtocol::versionTypeToString(m_version);
      obj["type"] = SynergyProtocol::messageTypeToString(type()); // use mapping function
      obj["id"] = m_id;
      obj["payload"] = payloadToJson(); // delegate payload creation
      // TBD : should we add seq_id here or in payload later?
      return obj;
    }

    virtual inline QString toString() const {
      return QJsonDocument(this->toJSon()).toJson();
    }

    // Deserialize common fields from a full JSON message object
    // Returns false if basic structure (version, type) is invalid
    virtual bool fromJson(const QJsonObject& obj) {
      if(!obj.contains("version") || !obj.value("version").isString()) {
        qCritical() << "BASE MESAGE |  JSON message missing version string";
        return false;
      }
      if(!obj.contains("type") || !obj.value("type").isString()) {
        qCritical() << "BASE MESAGE |  JSON message missing type string";
        return false;
      }
      if(!obj.contains("id") || !obj.value("id").isDouble()) {
        qCritical() << "BASE MESAGE | JSON message missing id";
        return false;
      }
      m_version = SynergyProtocol::stringToVersionType(obj["version"].toString());
      m_id = (quintptr)obj["id"].toDouble();
      if (!obj.contains("payload") || !obj.value("payload").isObject()) {
        qCritical() << "Base JSON missing payload object.";
        return false;
      }
      // TBD : Where is sequence value stored/client id?

      return payloadFromJson(obj["payload"].toObject());
    }

  protected:
    Message_Base(qintptr id) : m_id(id) {} 

    // Derived classes implement how their specific payload is built/parsed
    virtual QJsonObject payloadToJson() const = 0;
    virtual bool payloadFromJson(const QJsonObject& payloadObj) = 0;

    SynergyProtocol::t_Versions m_version;
    qintptr m_id;
  };
}

#endif