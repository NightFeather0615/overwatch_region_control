#include <flutter/encodable_value.h>

#include <iostream>
#include <map>
#include <vector>
#include <any>

#include <windows.h>
#include <comutil.h>
#include <comdef.h>
#include <netfw.h>


#ifndef WIN32_FIREWALL_CONTROL_H_
#define WIN32_FIREWALL_CONTROL_H_

HRESULT InitComLib();
void CleanupComLib();

namespace Firewall {
  HRESULT InitCom(INetFwPolicy2 **ppNetFwPolicy2);
  HRESULT IsEnabled(bool* enabled);
  HRESULT SetEnabled(bool enabled = true);
  HRESULT AddRule(flutter::EncodableMap ruleArgs);
  HRESULT DeleteRule(std::string ruleName);
  HRESULT ToggleRule(std::string ruleName, bool enabled = false);
  HRESULT GetRules(flutter::EncodableList* rules);
  flutter::EncodableMap RuleToEncodableMap(INetFwRule* rule);
}

#endif // WIN32_FIREWALL_CONTROL_H_
