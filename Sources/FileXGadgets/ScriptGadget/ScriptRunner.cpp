#include "StdAfx.h"
#include "SCriptGadget.h"
#include "ScriptRunner.h"
#include "GenericScriptRunner.h"
#include "GenericScriptRunnerDlg.h"

#define THIS_MODULENAME "ScriptRunner"


ScriptRunner::ScriptRunner(void)
{
  FXString sRunnerIndex("");
  strrev(m_GadgetName.GetBuffer());
  for ( int i = 0 ; i < m_GadgetName.GetLength() ; i++ )
  {    
    if (isdigit(m_GadgetName[i]))
    {
      FXString Digit = m_GadgetName[i];
      sRunnerIndex.Append(Digit);
    }
  }
  strrev(sRunnerIndex.GetBuffer());
  m_GadgetName.Delete(0, m_GadgetName.GetLength());
  m_GadgetName = "ScriptRunner";
  m_GadgetName.Append(sRunnerIndex);
}

 void ScriptRunner::ShutDown()
{
  GenericScriptRunner::ShutDown(); 
}
