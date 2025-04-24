#include "../../include/synergy_protocol/Message_Join_Session_Request.h"

using namespace SynergyProtocol;

QJsonObject Message_Join_Session_Request::payloadToJson() const {
  if (m_username.isEmpty()){
    qCritical() << "JOIN_SESSION_REQUEST | Username is empty in JoinSessionRequest payload";
    // TBD : throw an exception or handle error? 
    // Option 1: Return empty object to indicate failure
    return QJsonObject();
    // Option 2: Throw an exception (if your error handling will use them)
    // throw std::runtime_error("Username cannot be empty for Join Session Request");
  }
  QJsonObject payload;
  if (!m_session_id_to_join.isEmpty()) {
      payload.insert("session_id", m_session_id_to_join);
  } else if (m_create_new) { // Only include if true? Or always include? Design choice.
      payload.insert("create_new", m_create_new);
  } else {
    qWarning() << "JOIN_SESSION_REQUEST | One of create_new or session_id_to_join must be present";
    // TBD : throw an exception or handle error? 
    // Option 1: Return empty object to indicate failure
    return QJsonObject();
    // Option 2: Throw an exception (if your error handling will use them)
    // throw std::runtime_error("Username cannot be empty for Join Session Request");
  }
  payload.insert("username", m_username);
  return payload;
}


bool Message_Join_Session_Request::payloadFromJson(const QJsonObject& payloadObj) {
  // VALIDATE FIELDS from JSON
  if (!payloadObj.contains("username") || !payloadObj.value("username").isString() || payloadObj.value("username").toString().isEmpty()) {
      // Username is mandatory and must be non-empty string
      qCritical() << "JOIN_SESSION_REQUEST | Payload missing or invalid 'username'.";
      return false;
  }
  // TBD: is necessary
  // Socket descriptor or real id
  // if(!payloadObj.contains("client_id")){
  //   qWarning() << "JOIN_SESSION_REQUEST | No clientId present ";
  //   return false;
  // }
  if(!payloadObj.contains("session_id")){
    if(!payloadObj.contains("create_new")) {
      qCritical() << "JOIN_SESSION_REQUEST | No session id nor request to create new session";
      return false;
    }
    if(!payloadObj["create_new"].isBool()){
      qCritical() << "JOIN_SESSION_REQUEST | Bool needs to be assigned to create_new";
      return false;
    }
    m_create_new = true;
  }
  if(payloadObj["session_id"].isString()) {
    qCritical() << "JOIN_SESSION_REQUEST | String needs to be assigned to session_id_to_join";
    return false;
  }
  m_session_id_to_join = payloadObj.value("session_id").toString();
  m_username = payloadObj.value("username").toString();


  // Add more validation: e.g., username length limits?
  return true; // Parsing successful
}