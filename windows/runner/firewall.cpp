#include <flutter/encodable_value.h>

#include "firewall.h"

#include <iostream>
#include <map>
#include <any>

#include <windows.h>
#include <comutil.h>
#include <comdef.h>
#include <netfw.h>


HRESULT InitComLib() {
  return CoInitializeEx(NULL, tagCOINIT::COINIT_APARTMENTTHREADED);
}

void CleanupComLib() {
  CoUninitialize();
}

HRESULT InitFirewallCom(INetFwPolicy2 **policy) {
  HRESULT hr = S_OK;

  hr = CoCreateInstance(
    __uuidof(NetFwPolicy2), 
    NULL, 
    CLSCTX_INPROC_SERVER, 
    __uuidof(INetFwPolicy2), 
    (void**)policy
  );
  if (FAILED(hr)) {
    goto Cleanup;
  }

Cleanup:
  return hr;
}

HRESULT IsFirewallEnabled(bool* enabled) {
  HRESULT hr = S_OK;

  INetFwPolicy2 *pNetFwPolicy2 = NULL;
  long pFwProfileType = 0;
  VARIANT_BOOL pFwEnabled = VARIANT_FALSE;

  hr = InitFirewallCom(&pNetFwPolicy2);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwPolicy2 -> get_CurrentProfileTypes(&pFwProfileType);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwPolicy2 -> get_FirewallEnabled(NET_FW_PROFILE_TYPE2(pFwProfileType), &pFwEnabled);
  if (FAILED(hr)) {
    goto Cleanup;
  }

Cleanup:
  if (pNetFwPolicy2 != NULL){
    pNetFwPolicy2 -> Release();
  }

  *enabled = pFwEnabled == VARIANT_TRUE;

  return hr;
}

BSTR StringToBstr(std::any value) {
  return _com_util::ConvertStringToBSTR((std::any_cast<std::string>(value)).c_str());
}

BSTR StringToBstr(std::string value) {
  return _com_util::ConvertStringToBSTR((value).c_str());
}

std::string BstrToString(BSTR bstr) {
  std::wstring wstr(bstr, SysStringLen(bstr));
  if(wstr.empty()) return std::string();

  int size_needed = WideCharToMultiByte(
    CP_UTF8,
    0,
    &wstr[0],
    static_cast<int>(wstr.size()),
    NULL,
    0,
    NULL,
    NULL
  );

  std::string strTo(size_needed, 0);
  WideCharToMultiByte(
    CP_UTF8,
    0,
    &wstr[0],
    static_cast<int>(wstr.size()),
    &strTo[0],
    size_needed,
    NULL,
    NULL
  );

  return strTo;
}

HRESULT AddFirewallRule(std::map<std::string, std::any> ruleArgs) {
  HRESULT hr = S_OK;

  INetFwPolicy2 *pNetFwPolicy2 = NULL;
  long pFwProfileType = 0;
  INetFwRules *pNetFwRules = NULL;
  INetFwRule *pNetFwRule = NULL;
  std::map<std::string, std::any>::iterator pRuleArg = ruleArgs.find("");

  hr = InitFirewallCom(&pNetFwPolicy2);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwPolicy2 -> get_CurrentProfileTypes(&pFwProfileType);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  if ((pFwProfileType & NET_FW_PROFILE2_PUBLIC) && (pFwProfileType != NET_FW_PROFILE2_PUBLIC)){
    pFwProfileType ^= NET_FW_PROFILE2_PUBLIC;
  }

  hr = pNetFwPolicy2 -> get_Rules(&pNetFwRules);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = CoCreateInstance(
    __uuidof(NetFwRule),
    NULL,
    CLSCTX_INPROC_SERVER,
    __uuidof(INetFwRule),
    (void**)&pNetFwRule
  );
  if (FAILED(hr)){
    goto Cleanup;
  }

  pRuleArg = ruleArgs.find("name");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Name(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("description");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Description(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("app_name");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_ApplicationName(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("service_name");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_ServiceName(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("protocol");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Protocol(std::any_cast<long>(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("icmp_type");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_IcmpTypesAndCodes(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("local_ports");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_LocalPorts(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("remote_ports");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_RemotePorts(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("local_adresses");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_LocalAddresses(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("remote_addresses");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_RemoteAddresses(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("profiles");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Profiles(pFwProfileType);
  }
  pRuleArg = ruleArgs.find("direction");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Direction(NET_FW_RULE_DIRECTION(std::any_cast<long>(pRuleArg -> second)));
  }
  pRuleArg = ruleArgs.find("action");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Action(NET_FW_ACTION(std::any_cast<long>(pRuleArg -> second)));
  }
  pRuleArg = ruleArgs.find("interface_types");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_InterfaceTypes(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("enabled");
  if (pRuleArg != ruleArgs.end() && std::any_cast<bool>(pRuleArg -> second)) {
    pNetFwRule -> put_Enabled(VARIANT_TRUE);
  } else {
    pNetFwRule -> put_Enabled(VARIANT_FALSE);
  }
  pRuleArg = ruleArgs.find("grouping");
  if (pRuleArg != ruleArgs.end()) {
    pNetFwRule -> put_Grouping(StringToBstr(pRuleArg -> second));
  }
  pRuleArg = ruleArgs.find("edge_traversal");
  if (pRuleArg != ruleArgs.end() && std::any_cast<bool>(pRuleArg -> second)) {
    pNetFwRule -> put_EdgeTraversal(VARIANT_TRUE);
  } else {
    pNetFwRule -> put_EdgeTraversal(VARIANT_FALSE);
  }

  hr = pNetFwRules -> Add(pNetFwRule);
  if (FAILED(hr)) {
    goto Cleanup;
  }

Cleanup:
  if (pNetFwPolicy2 != NULL){
    pNetFwPolicy2 -> Release();
  }
  if (pNetFwRules != NULL){
    pNetFwRules -> Release();
  }
  if (pNetFwRule != NULL){
    pNetFwRule -> Release();
  }

  return hr;
}

HRESULT DeleteFirewallRule(std::string ruleName) {
  HRESULT hr = S_OK;

  INetFwPolicy2 *pNetFwPolicy2 = NULL;
  INetFwRules *pNetFwRules = NULL;

  hr = InitFirewallCom(&pNetFwPolicy2);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwPolicy2 -> get_Rules(&pNetFwRules);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwRules -> Remove(StringToBstr(ruleName));
  if (FAILED(hr)) {
    goto Cleanup;
  }

Cleanup:
  if (pNetFwPolicy2 != NULL){
    pNetFwPolicy2 -> Release();
  }
  if (pNetFwRules != NULL){
    pNetFwRules -> Release();
  }

  return hr;
}

HRESULT ToggleFirewallRule(std::string ruleName, bool enabled) {
  HRESULT hr = S_OK;

  INetFwPolicy2 *pNetFwPolicy2 = NULL;
  INetFwRules *pNetFwRules = NULL;
  INetFwRule *pNetFwRule = NULL;

  hr = InitFirewallCom(&pNetFwPolicy2);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwPolicy2 -> get_Rules(&pNetFwRules);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwRules -> Item(StringToBstr(ruleName), &pNetFwRule);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwRule -> put_Enabled(enabled ? VARIANT_TRUE : VARIANT_FALSE);
  if (FAILED(hr)) {
    goto Cleanup;
  }

Cleanup:
  if (pNetFwPolicy2 != NULL){
    pNetFwPolicy2 -> Release();
  }
  if (pNetFwRules != NULL){
    pNetFwRules -> Release();
  }
  if (pNetFwRule != NULL){
    pNetFwRule -> Release();
  }

  return hr;
}

flutter::EncodableMap FirewallRuleToEncodableMap(INetFwRule* rule) {
  flutter::EncodableMap res = {};
  BSTR strBuf = StringToBstr(std::string());
  long longBuf = 0;
  VARIANT_BOOL boolBuf = VARIANT_FALSE;
  NET_FW_RULE_DIRECTION dirBuf = NET_FW_RULE_DIRECTION::NET_FW_RULE_DIR_MAX;
  NET_FW_ACTION actionBuf = NET_FW_ACTION::NET_FW_ACTION_MAX;
  
  rule -> get_Name(&strBuf);
  res.insert({std::string("name"), BstrToString(strBuf)});
  rule -> get_Description(&strBuf);
  res.insert({std::string("description"), BstrToString(strBuf)});
  rule -> get_ApplicationName(&strBuf);
  res.insert({std::string("app_name"), BstrToString(strBuf)});
  rule -> get_ServiceName(&strBuf);
  res.insert({std::string("service_name"), BstrToString(strBuf)});
  rule -> get_Protocol(&longBuf);
  res.insert({std::string("protocol"), longBuf});
  rule -> get_IcmpTypesAndCodes(&strBuf);
  res.insert({std::string("icmp_type"), BstrToString(strBuf)});
  rule -> get_LocalPorts(&strBuf);
  res.insert({std::string("local_ports"), BstrToString(strBuf)});
  rule -> get_RemotePorts(&strBuf);
  res.insert({std::string("remote_ports"), BstrToString(strBuf)});
  rule -> get_LocalAddresses(&strBuf);
  res.insert({std::string("local_adresses"), BstrToString(strBuf)});
  rule -> get_RemoteAddresses(&strBuf);
  res.insert({std::string("remote_addresses"), BstrToString(strBuf)});
  rule -> get_Profiles(&longBuf);
  res.insert({std::string("profiles"), longBuf});
  rule -> get_Direction(&dirBuf);
  res.insert({std::string("direction"), static_cast<long>(dirBuf)});
  rule -> get_Action(&actionBuf);
  res.insert({std::string("action"), static_cast<long>(actionBuf)});
  rule -> get_InterfaceTypes(&strBuf);
  res.insert({std::string("interface_types"), BstrToString(strBuf)});
  rule -> get_Enabled(&boolBuf);
  res.insert({std::string("enabled"), boolBuf == VARIANT_TRUE});
  rule -> get_Grouping(&strBuf);
  res.insert({std::string("grouping"), BstrToString(strBuf)});
  rule -> get_EdgeTraversal(&boolBuf);
  res.insert({std::string("edge_traversal"), boolBuf == VARIANT_TRUE});

  return res;
}

HRESULT GetFirewallRules(flutter::EncodableList* rules) {
  HRESULT hr = S_OK;

  ULONG cFetched = 0; 
  VARIANT var;

  IUnknown* pEnumerator;
  IEnumVARIANT* pVariant = NULL;

  INetFwPolicy2 *pNetFwPolicy2 = NULL;
  INetFwRules *pNetFwRules = NULL;
  INetFwRule *pNetFwRule = NULL;

  long fwRuleCount;

  hr = InitFirewallCom(&pNetFwPolicy2);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwPolicy2 -> get_Rules(&pNetFwRules);
  if (FAILED(hr)) {
    goto Cleanup;
  }

  hr = pNetFwRules -> get_Count(&fwRuleCount);
  if (FAILED(hr)) {
    goto Cleanup;
  }
  

  pNetFwRules -> get__NewEnum(&pEnumerator);

  if(pEnumerator) {
    hr = pEnumerator -> QueryInterface(
      __uuidof(IEnumVARIANT),
      (void**) &pVariant)
    ;
  }

  while(SUCCEEDED(hr) && hr != S_FALSE) {
    VariantClear(&var);
    hr = pVariant -> Next(1, &var, &cFetched);

    if (S_FALSE != hr) {
      if (SUCCEEDED(hr)) {
        hr = VariantChangeType(&var, &var, VARIANT_NOVALUEPROP, VT_DISPATCH);
      }

      if (SUCCEEDED(hr)) {
        hr = (V_DISPATCH(&var)) -> QueryInterface(
          __uuidof(INetFwRule),
          reinterpret_cast<void**>(&pNetFwRule)
        );
      }

      if (SUCCEEDED(hr)) {
        rules -> push_back(FirewallRuleToEncodableMap(pNetFwRule));
      }
    }
  }

Cleanup:
  if (pNetFwRule != NULL) {
    pNetFwRule -> Release();
  }

  if (pNetFwPolicy2 != NULL) {
    pNetFwPolicy2 -> Release();
  }

  return hr;
}
