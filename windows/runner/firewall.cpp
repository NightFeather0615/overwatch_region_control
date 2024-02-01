#include <flutter/encodable_value.h>

#include "firewall.h"

#include <iostream>
#include <map>
#include <any>
#include <variant>

#include <windows.h>
#include <comutil.h>
#include <comdef.h>
#include <netfw.h>


BSTR StringToBstr(std::any value) {
  return _com_util::ConvertStringToBSTR((std::any_cast<std::string>(value)).c_str());
}

BSTR StringToBstr(flutter::EncodableValue value) {
  return _com_util::ConvertStringToBSTR((std::get<std::string>(value)).c_str());
}

BSTR StringToBstr(std::string value) {
  return _com_util::ConvertStringToBSTR((value).c_str());
}

long EncodableToLong(flutter::EncodableValue value) {
  return static_cast<long>(std::get<int32_t>(value));
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

namespace Firewall {
  HRESULT InitCom(INetFwPolicy2 **policy) {
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

  HRESULT IsEnabled(bool* enabled) {
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    long pFwProfileType = 0;
    VARIANT_BOOL pFwEnabled = VARIANT_FALSE;

    hr = InitCom(&pNetFwPolicy2);
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

  HRESULT SetEnabled(bool enabled) {
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    long pFwProfileType = 0;

    hr = InitCom(&pNetFwPolicy2);
    if (FAILED(hr)) {
      goto Cleanup;
    }

    hr = pNetFwPolicy2 -> get_CurrentProfileTypes(&pFwProfileType);
    if (FAILED(hr)) {
      goto Cleanup;
    }

    hr = pNetFwPolicy2 -> put_FirewallEnabled(
      NET_FW_PROFILE_TYPE2(pFwProfileType),
      enabled ? VARIANT_TRUE : VARIANT_FALSE
    );
    if (FAILED(hr)) {
      goto Cleanup;
    }

  Cleanup:
    if (pNetFwPolicy2 != NULL){
      pNetFwPolicy2 -> Release();
    }

    return hr;
  }

  HRESULT AddRule(flutter::EncodableMap ruleArgs) {
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    long pFwProfileType = 0;
    INetFwRules *pNetFwRules = NULL;
    INetFwRule *pNetFwRule = NULL;
    flutter::EncodableMap::iterator pRuleArg = ruleArgs.find(std::string());

    hr = InitCom(&pNetFwPolicy2);
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

    pRuleArg = ruleArgs.find(std::string("name"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Name(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("description"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Description(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("app_name"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_ApplicationName(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("service_name"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_ServiceName(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("protocol"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Protocol(EncodableToLong(pRuleArg -> second));
    } else {
      pNetFwRule -> put_Protocol(static_cast<long>(6));
    }
    pRuleArg = ruleArgs.find(std::string("icmp_type"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_IcmpTypesAndCodes(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("local_ports"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_LocalPorts(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("remote_ports"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_RemotePorts(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("local_adresses"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_LocalAddresses(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("remote_addresses"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_RemoteAddresses(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("profiles"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Profiles(EncodableToLong(pRuleArg -> second));
    } else {
      pNetFwRule -> put_Profiles(pFwProfileType);
    }
    pRuleArg = ruleArgs.find(std::string("direction"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Direction(NET_FW_RULE_DIRECTION(EncodableToLong(pRuleArg -> second)));
    }
    pRuleArg = ruleArgs.find(std::string("action"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Action(NET_FW_ACTION(EncodableToLong(pRuleArg -> second)));
    }
    pRuleArg = ruleArgs.find(std::string("interface_types"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_InterfaceTypes(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("enabled"));
    if (pRuleArg != ruleArgs.end() && std::get<bool>(pRuleArg -> second)) {
      pNetFwRule -> put_Enabled(VARIANT_TRUE);
    } else {
      pNetFwRule -> put_Enabled(VARIANT_FALSE);
    }
    pRuleArg = ruleArgs.find(std::string("grouping"));
    if (pRuleArg != ruleArgs.end()) {
      pNetFwRule -> put_Grouping(StringToBstr(pRuleArg -> second));
    }
    pRuleArg = ruleArgs.find(std::string("edge_traversal"));
    if (pRuleArg != ruleArgs.end() && std::get<bool>(pRuleArg -> second)) {
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

  HRESULT DeleteRule(std::string ruleName) {
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pNetFwRules = NULL;

    hr = InitCom(&pNetFwPolicy2);
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

  HRESULT ToggleRule(std::string ruleName, bool enabled) {
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pNetFwRules = NULL;
    INetFwRule *pNetFwRule = NULL;

    hr = InitCom(&pNetFwPolicy2);
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

  flutter::EncodableMap RuleToEncodableMap(INetFwRule* rule) {
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

  HRESULT GetRules(flutter::EncodableList* rules) {
    HRESULT hr = S_OK;

    ULONG cFetched = 0; 
    VARIANT var;

    IUnknown* pEnumerator;
    IEnumVARIANT* pVariant = NULL;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pNetFwRules = NULL;
    INetFwRule *pNetFwRule = NULL;

    long fwRuleCount;

    hr = InitCom(&pNetFwPolicy2);
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
          rules -> push_back(RuleToEncodableMap(pNetFwRule));
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
}
