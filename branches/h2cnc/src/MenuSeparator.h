// MenuSeparator.h
// Copyright (c) 2011, Joseph Coffland
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Tool.h"

class MenuSeparator : public Tool {
public:
  // From Tool
  void Run() {}
  const wxChar *GetTitle() {return _("MenuSeparator");}
  bool IsSeparator() const {return true;}
};

