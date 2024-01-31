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

HRESULT InitFirewallCom(INetFwPolicy2 **ppNetFwPolicy2);
HRESULT IsFirewallEnabled(bool* enabled);
HRESULT AddFirewallRule(std::map<std::string, std::any> ruleArgs);
HRESULT DeleteFirewallRule(std::string ruleName);
HRESULT ToggleFirewallRule(std::string ruleName, bool enabled = false);
HRESULT GetFirewallRules(flutter::EncodableList* rules);
flutter::EncodableMap FirewallRuleToEncodableMap(INetFwRule* rule);

#endif // WIN32_FIREWALL_CONTROL_H_
