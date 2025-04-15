#ifndef __SYNERGY_PROTOCOL__
#define __SYNERGY_PROTOCOL__

#include <QString>
#include <QJsonObject>
#include <QHash>

#include <vector>

namespace SynergyProtocol {
  enum class t_Versions {
    V1_0 = 0
  };
  enum class t_MessageType {
    JOIN_SESSION_REQUEST
  };

  // Function to map enum to JSON string type and back (essential)
  QString messageTypeToString(t_MessageType type);
  t_MessageType stringToMessageType(const QString& typeStr);
  QString versionTypeToString(t_Versions type);
  t_Versions stringToVersionType(const QString& typeStr);

  class Message_Base {
  public: 

    virtual ~Message_Base() = default;

    virtual t_MessageType type() const { return m_type; };
    t_Versions version() const { return m_version; }
    void setVersion(const t_Versions& version) { m_version = version; }

    // Serialize entire message to JSON
    virtual QJsonObject toJSon() const {
      QJsonObject obj;
      obj["version"] = versionTypeToString(m_version);
      obj["type"] = messageTypeToString(type()); // use mapping function
      obj["payload"] = payloadToJson(); // delegate payload creation
      // TBD : should we add seq_id here or in payload later?
      return obj;
    }

    // Deserialize common fields from a full JSON message object
    // Returns false if basic structure (version, type) is invalid
    virtual bool fromJson(const QJsonObject& obj) {
      if(!obj.contains("version") || !obj.value("version").isString()) {
        qWarning() << "JSON message missing version string";
        return false;
      }
      if(!obj.contains("type") || !obj.value("type").isString()) {
        qWarning() << "JSON message missing version string";
        return false;
      }
      m_version = stringToVersionType(obj["version"].toString());
      m_type = stringToMessageType(obj["type"].toString());
      if (!obj.contains("payload") || !obj.value("payload").isObject()) {
        qWarning() << "Base JSON missing payload object.";
        return false;
      }
      // TBD : Where is sequence value stored

      return payloadFromJson(obj["payload"].toObject());
    }

  protected:
    // Derived classes implement how their specific payload is built/parsed
    virtual QJsonObject payloadToJson() const = 0;
    virtual bool payloadFromJson(const QJsonObject& payloadObj) = 0;

    t_Versions m_version;
    t_MessageType m_type;
    QJsonObject payload;
  };
}
#endif;