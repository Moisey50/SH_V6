#pragma once

#include "GenericScriptRunner.h"

class ScriptRunner : public GenericScriptRunner
{
public: 
  ScriptRunner(void);
  void ShutDown(void);
  DECLARE_RUNTIME_GADGET(ScriptRunner); 
};
