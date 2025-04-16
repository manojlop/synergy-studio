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

  class Message_Base;

  class Message_Join_Session_Request;

  class MessageFactory;
}
#endif