#include "stdafx.h"
#include "ControllerCommandFactory.h"

const CommandBase CommandBase::Empty = CommandBase("");
const string CommandSetData::CMD_DFLT_KEY_VAL_DLMTR_CLEAR = ("");

//const CmdsSequenceBase CmdsSequenceBase::Empty = CmdsSequenceBase(list<const CommandBase*>());

//const ControllerOperationBase ControllerOperationBase::Empty = ControllerOperationBase(ModeConfigBase::Empty, 0, list<const CmdsSequenceBase*>());