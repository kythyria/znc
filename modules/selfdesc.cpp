#include <znc/Client.h>
#include <znc/Chan.h>
#include <znc/Modules.h>

class CSelfDescription : CModule {
public:
  MODCONSTRUCTOR(CSelfDescription) {
    m_trigger("$is")
    m_leaktriggers = false;
  }
  
  virtual EModRet OnUserAction(CString& sTarget, CString& sMessage);
  
private:
  EModRet OnTrigger(CString& sTarget, CString& sMessage);
  
  CString m_trigger;
  bool    m_leaktriggers;
};

EModRet CSelfDescription::OnUserAction(CString& sTarget, CString& sMessage) {
  if (sMessage.StartsWith(m_trigger)) {
    return handle_trigger(sTarget, sMessage);
  }
  return CModule::CONTINUE;
}

EModRet CSelfDescription::OnTrigger(CString& sTarget, CString& sMessage) {
  CString sPath = GetSavePath()
  sPath += sTarget;
  
  PutModule("[" + sPath + "]")
  
  return CModule::HALTCORE;
}

template<> void TModInfo<CSampleMod>(CModInfo& Info) {
  
}

USERMODULEDEFS(CSelfDescription, "Self description thing");