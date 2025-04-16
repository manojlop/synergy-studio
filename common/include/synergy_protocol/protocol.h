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
    UNKNOWN,
    JOIN_SESSION_REQUEST
  };

  // static const ensures the maps are built only once.
  // QStringLiteral avoids runtime allocation, making it faster than QString("...").
  // QHash::value(..., default) returns a default enum if not found

  inline QString messageTypeToString(t_MessageType type){
    static const QHash<t_MessageType, QString> typeToString {
        { t_MessageType::UNKNOWN, QStringLiteral("UNKNOWN") },
        { t_MessageType::JOIN_SESSION_REQUEST, QStringLiteral("JOIN_SESSION_REQUEST") },
    };
    return typeToString.value(type, QStringLiteral("UNKNOWN"));
  }

  inline t_MessageType stringToMessageType(const QString &typeStr){
    static const QHash<QString, t_MessageType> stringToType {
        { QStringLiteral("UNKNOWN"), t_MessageType::UNKNOWN },
        { QStringLiteral("JOIN_SESSION_REQUEST"), t_MessageType::JOIN_SESSION_REQUEST },
    };
    return stringToType.value(typeStr, t_MessageType::UNKNOWN);
  }

  inline QString versionTypeToString(t_Versions version){
    static const QHash<t_Versions, QString> versionToString {
        { t_Versions::V1_0, QStringLiteral("V1_0") },
    };
    return versionToString.value(version, QStringLiteral("V1_0"));
  }

  inline t_Versions stringToVersionType(const QString &versionStr){
    static const QHash<QString, t_Versions> stringToVersion {
        { QStringLiteral("V1_0"), t_Versions::V1_0 },
    };
    return stringToVersion.value(versionStr, t_Versions::V1_0);
  }

  class Message_Base {
  public: 

    virtual ~Message_Base() = default;

    virtual t_MessageType type() const = 0;
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
    QJsonObject m_payload;
  };

  class Message_Join_Session_Request : public Message_Base {
  public:
    t_MessageType type() const override { return t_MessageType::JOIN_SESSION_REQUEST; }

    // Const getters provide read-only access
    const QString& sessionIdToJoin() const { return m_session_id_to_join; }
    bool shouldCreateNew() const { return m_create_new; }
    const QString& username() const { return m_username; }


    explicit Message_Join_Session_Request(QString sid = "", bool create = false, QString name = "") :
        m_session_id_to_join(std::move(sid)),
        m_create_new(create),
        m_username(std::move(name)) {}

    void createMessage(QString sid, bool create, QString name){
      m_session_id_to_join = sid;
      m_create_new = sid == "" ? create : false;
      m_username = name;
    }

  protected:
    QString m_session_id_to_join;
    bool m_create_new;
    QString m_username;

    virtual QJsonObject payloadToJson() const {
      QJsonObject payload;
      if (!m_session_id_to_join.isEmpty()) {
          payload.insert("session_id", m_session_id_to_join);
      } else if (m_create_new) { // Only include if true? Or always include? Design choice.
          payload.insert("create_new", m_create_new);
      } else {
        qWarning() << "JOIN_SESSION_REQUEST | One of create_new or session_id_to_join must be present";

      }
      if (m_username.isEmpty()){
          qWarning() << "JOIN_SESSION_REQUEST | Username is empty in JoinSessionRequest payload";
          // TBD : throw an exception or handle error? 
      }
      payload.insert("username", m_username);
      return payload;
    }

    virtual bool payloadFromJson(const QJsonObject& payloadObj) {
      // VALIDATE FIELDS from JSON
      if (!payloadObj.contains("username") || !payloadObj.value("username").isString() || payloadObj.value("username").toString().isEmpty()) {
          // Username is mandatory and must be non-empty string
            qWarning() << "JOIN_SESSION_REQUEST | Payload missing or invalid 'username'.";
          return false;
      }
      // TBD: is necessary
      // Socket descriptor or real id
      if(!payloadObj.contains("client_id")){
        qWarning() << "JOIN_SESSION_REQUEST | No clientId present ";
        return false;
      }
      if(!payloadObj.contains("session_id")){
        if(!payloadObj.contains("create_new")) {
          qWarning() << "JOIN_SESSION_REQUEST | No session id nor request to create new session";
          return false;
        }
        if(!payloadObj["create_new"].isBool()){
          qWarning() << "JOIN_SESSION_REQUEST | Bool needs to be assigned to create_new";
          return false;
        }
      }
      if(payloadObj["session_id"].isString()) {
        qWarning() << "JOIN_SESSION_REQUEST | String needs to be assigned to session_id_to_join";
        return false;
      }
      m_session_id_to_join = payloadObj.value("session_id").toString();
      m_username = payloadObj.value("username").toString();


      // Add more validation: e.g., username length limits?
      return true; // Parsing successful
    }
  };
}
#endif