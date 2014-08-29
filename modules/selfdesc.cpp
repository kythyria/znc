/*
 * Copyright (C) 2004-2014 ZNC, see the NOTICE file for details.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This module is used to remember the user's descriptions of themself, and
 * repeat those on demand; scoped to a target. At present, the trigger is
 * a CTCP ACTION starting "$is". If that is followed by text, remember the
 * text with the target as the key.
 * 
 * If it is not followed by text, append the remembered text.
 * 
 * Then, send the message on to the server, after removing the $
 */

#include <znc/Client.h>
#include <znc/Chan.h>
#include <znc/Modules.h>
#include <znc/User.h>
#include <znc/IRCNetwork.h>

class CSelfDescription : public CModule {
public:
  MODCONSTRUCTOR(CSelfDescription) {
    m_trigger = "$is";
    m_responseprefix = "is ";
    m_leaktriggers = false;
  }
  
  virtual bool OnLoad(const CString& sArgs, CString& sMessage);
  virtual EModRet OnUserAction(CString& sTarget, CString& sMessage);
  
private:
  CString GetTargetPath(CString& sTarget);
  EModRet OnTrigger(CString& sTarget, CString& sMessage);
  void PutAction(CString& sTarget, CString& sMessage);
  
  CString m_trigger;
  CString m_responseprefix;
  bool    m_leaktriggers;
  CString m_path;
};

CString CSelfDescription::GetTargetPath(CString &sTarget) {
  CString sPath = m_path;
  sPath.Replace("$NETWORK", CString((m_pNetwork ? m_pNetwork->GetName() : "znc")).AsLower());
  sPath.Replace("$WINDOW", CString(sTarget.Replace_n("/", "-").Replace_n("\\", "-")).AsLower());
  sPath.Replace("$USER", CString((m_pUser ? m_pUser->GetUserName() : "UNKNOWN")).AsLower());
  return sPath;
}

bool CSelfDescription::OnLoad(const CString& sArgs, CString& sMessage) {
  m_path = GetSavePath();
  m_path += "/";
  
  if (GetType() == CModInfo::UserModule) {
    m_path += "$NETWORK/$WINDOW";
  } else if (GetType() == CModInfo::NetworkModule) {
    m_path += "$WINDOW";
  } else {
    m_path += "$USER/$NETWORK/$WINDOW";
  }
  
  return true;
}

CModule::EModRet CSelfDescription::OnUserAction(CString& sTarget, CString& sMessage) {
  if (sMessage.StartsWith(m_trigger)) {
    return OnTrigger(sTarget, sMessage);
  }
  return CModule::CONTINUE;
}

CModule::EModRet CSelfDescription::OnTrigger(CString& sTarget, CString& sMessage) {
  
  CString sPath = GetTargetPath(sTarget);
  
  CString sDesc = sMessage.TrimPrefix_n(m_trigger);
  sDesc.TrimLeft();
  sDesc.TrimRight();
  
  CFile fDescription(sPath);
  
  struct stat ModDirInfo;
  CFile::GetInfo(GetSavePath(), ModDirInfo);
  CString sDescDir = fDescription.GetDir();
  if (!CFile::Exists(sDescDir)) CDir::MakeDir(sDescDir, ModDirInfo.st_mode);
  
  if (fDescription.Open(O_RDWR | O_APPEND | O_CREAT)) {
    if (sDesc.empty()) {
      fDescription.ReadLine(sDesc);
    } else {
      fDescription.Truncate();
      fDescription.Write(sDesc);
    }
  } else {
    PutModule("Failure accessing description file");
  }
  
  PutAction(sTarget, sDesc);
  
  return CModule::HALTCORE;
}

void CSelfDescription::PutAction(CString& sTarget, CString& sMessage) {
  
  CString sPayload = "\001ACTION $RP$MSG\001";
  sPayload.Replace("$RP",m_responseprefix);
  sPayload.Replace("$MSG",sMessage);
  
  //CChan* pChan = m_pNetwork->FindChan(sTarget);
  //if (pChan) {
  //  if (!pChan->AutoClearChanBuffer()) {
  //    pChan->AddBuffer(":" + NickPrefix() + _NAMEDFMT(m_pNetwork->GetIRCNick().GetNickMask()) + " PRIVMSG " + _NAMEDFMT(sTarget) + " :{text}", sPayload);
  //  }
  //  m_pUser->PutUser(":" + NickPrefix() + m_pNetwork->GetIRCNick().GetNickMask() + " PRIVMSG " + sTarget + " :" + sPayload, NULL, m_pClient);
  //}
  
  PutIRC("PRIVMSG " + sTarget + " :" + sPayload);
}

template<> void TModInfo<CSelfDescription>(CModInfo& Info) {
  
}

USERMODULEDEFS(CSelfDescription, "Self description thing");