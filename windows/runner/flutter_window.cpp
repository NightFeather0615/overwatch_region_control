#include "flutter_window.h"
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <flutter/encodable_value.h>

#include "firewall.h"

#include <memory>
#include <optional>
#include <iostream>
#include <map>
#include <vector>
#include <any>
#include <variant>

#include <winerror.h>

#include "flutter/generated_plugin_registrant.h"


HRESULT InitComLib() {
  return CoInitializeEx(NULL, tagCOINIT::COINIT_APARTMENTTHREADED);
}

void CleanupComLib() {
  CoUninitialize();
}


FlutterWindow::FlutterWindow(
  const flutter::DartProject& project
): project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  InitComLib();

  RECT frame = GetClientArea();

  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
    frame.right - frame.left,
    frame.bottom - frame.top,
    project_
  );

  if (!flutter_controller_ -> engine() || !flutter_controller_ -> view()) {
    return false;
  }
  RegisterPlugins(flutter_controller_ -> engine());

  flutter::MethodChannel<> firewallChannel (
    flutter_controller_ -> engine() -> messenger(),
    "overwatch_region_control.nightfeather.dev/firewall",
    &flutter::StandardMethodCodec::GetInstance()
  );

  firewallChannel.SetMethodCallHandler(
    [](
      const flutter::MethodCall<>& call,
      std::unique_ptr<flutter::MethodResult<>> call_result
    ) {
      std::string methodName = call.method_name();

      if (methodName == "isEnabled") {
        bool pEnabled = false;
        HRESULT result = Firewall::IsEnabled(&pEnabled);
        
        if (SUCCEEDED(result)) {
          call_result -> Success(pEnabled);
        } else {
          call_result -> Error(
            "GET_FIREWALL_STATUS_FAILED",
            "Unabled to get firewall status"
          );
        }
      } else if (methodName == "setEnabled") {
        if (!std::holds_alternative<bool>(*call.arguments())) {
          call_result -> Error(
            "BAD_ARGUMENT_TYPE",
            "The argument type should be single `bool`"
          );
          return;
        }

        bool enabled = std::get<bool>(*call.arguments());
        HRESULT result = Firewall::SetEnabled(enabled);
      
        if (SUCCEEDED(result)) {
          call_result -> Success(std::monostate());
        } else if (result == E_ACCESSDENIED) {
          call_result -> Error(
            "ACCESS_DENIED",
            "The operation was aborted due to permissions issues"
          );
        } else {
          call_result -> Error(
            "SET_FIREWALL_ENABLED_FAILED",
            "Unknown error"
          );
        }
      } else if (methodName == "addRule") {
        if (!std::holds_alternative<flutter::EncodableMap>(*call.arguments())) {
          call_result -> Error(
            "BAD_ARGUMENT_TYPE",
            "The argument type should be single `flutter::EncodableMap`"
          );
          return;
        }

        flutter::EncodableMap ruleArgs = std::get<flutter::EncodableMap>(*call.arguments());
        HRESULT result = Firewall::AddRule(ruleArgs);
      
        if (SUCCEEDED(result)) {
          call_result -> Success(std::monostate());
        } else if (result == E_ACCESSDENIED) {
          call_result -> Error(
            "ACCESS_DENIED",
            "The operation was aborted due to permissions issues"
          );
        } else {
          call_result -> Error(
            "ADD_FIREWALL_RULE_FAILED",
            "Unknown error"
          );
        }
      } else if (methodName == "deleteRule") {
        if (!std::holds_alternative<std::string>(*call.arguments())) {
          call_result -> Error(
            "BAD_ARGUMENT_TYPE",
            "The argument type should be single `std::string`"
          );
          return;
        }

        std::string ruleName = std::get<std::string>(*call.arguments());
        HRESULT result = Firewall::DeleteRule(ruleName);
      
        if (SUCCEEDED(result)) {
          call_result -> Success(std::monostate());
        } else if (result == E_ACCESSDENIED) {
          call_result -> Error(
            "ACCESS_DENIED",
            "The operation was aborted due to permissions issues"
          );
        } else {
          call_result -> Error(
            "DELETE_FIREWALL_RULE_FAILED",
            "Unknown error"
          );
        }
      } else if (methodName == "toggleRule") {
        if (!std::holds_alternative<flutter::EncodableMap>(*call.arguments())) {
          call_result -> Error(
            "BAD_ARGUMENT_TYPE",
            "The argument type should be single `flutter::EncodableMap`"
          );
          return;
        }

        flutter::EncodableMap args = std::get<flutter::EncodableMap>(*call.arguments());
        flutter::EncodableMap::iterator name = args.find(std::string("name"));
        flutter::EncodableMap::iterator enabled = args.find(std::string("enabled"));
        if (name == args.end() || enabled == args.end()) {
          call_result -> Error(
            "BAD_ARGUMENT_TYPE",
            "The argument map should be key `name` and `enabled`"
          );
          return;
        }

        HRESULT result = Firewall::ToggleRule(
          std::get<std::string>(name -> second),
          std::get<bool>(enabled -> second)
        );
      
        if (SUCCEEDED(result)) {
          call_result -> Success(std::monostate());
        } else if (result == E_ACCESSDENIED) {
          call_result -> Error(
            "ACCESS_DENIED",
            "The operation was aborted due to permissions issues"
          );
        } else {
          call_result -> Error(
            "TOGGLE_FIREWALL_RULE_FAILED",
            "Unknown error"
          );
        }
      } else if (methodName == "getRule") {
        if (!std::holds_alternative<std::string>(*call.arguments())) {
          call_result -> Error(
            "BAD_ARGUMENT_TYPE",
            "The argument type should be single `std::string`"
          );
          return;
        }

        std::string ruleName = std::get<std::string>(*call.arguments());
        flutter::EncodableMap rule = {};
        HRESULT result = Firewall::GetRule(ruleName, &rule);
        
        if (SUCCEEDED(result)) {
          call_result -> Success(rule);
        } else if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
          call_result -> Error(
            "RULE_NOT_FOUND",
            "The requested item does not exist"
          );
        } else {
          call_result -> Error(
            "GET_FIREWALL_RULE_FAILED",
            "Unable to get firewall rule"
          );
        }
      } else if (methodName == "getRules") {
        flutter::EncodableList rules = {};
        HRESULT result = Firewall::GetRules(&rules);
        
        if (SUCCEEDED(result)) {
          call_result -> Success(rules);
        } else {
          call_result -> Error(
            "GET_FIREWALL_RULES_FAILED",
            "Unable to get firewall rules"
          );
        }
      } else {
        call_result -> NotImplemented();
      }
    }
  );

  flutter::MethodChannel<> nativeUtilsChannel (
    flutter_controller_ -> engine() -> messenger(),
    "overwatch_region_control.nightfeather.dev/native_utils",
    &flutter::StandardMethodCodec::GetInstance()
  );

  nativeUtilsChannel.SetMethodCallHandler(
    [](
      const flutter::MethodCall<>& call,
      std::unique_ptr<flutter::MethodResult<>> call_result
    ) {
      std::string methodName = call.method_name();

      if (methodName == "isUserAdmin") {
        BOOL b;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;
        b = AllocateAndInitializeSid(
          &NtAuthority,
          2,
          SECURITY_BUILTIN_DOMAIN_RID,
          DOMAIN_ALIAS_RID_ADMINS,
          0, 0, 0, 0, 0, 0,
          &AdministratorsGroup
        );

        if (b) {
          if (!CheckTokenMembership(NULL, AdministratorsGroup, &b)) {
            b = FALSE;
          }
          FreeSid(AdministratorsGroup);
        } else {
          call_result -> Error(
            "GET_TOKEN_MEMBERSHIP_FAILED",
            "Unable to token member ID"
          );
        }
        call_result -> Success(b == TRUE);
      } else {
        call_result -> NotImplemented();
      }
    }
  );

  SetChildContent(flutter_controller_ -> view() -> GetNativeWindow());

  flutter_controller_ -> engine() -> SetNextFrameCallback(
    [&]() {
      this -> Show();
    }
  );

  flutter_controller_ -> ForceRedraw();

  return true;
}

void FlutterWindow::OnDestroy() {
  if (flutter_controller_) {
    flutter_controller_ = nullptr;
    CleanupComLib();
  }

  Win32Window::OnDestroy();
}

LRESULT FlutterWindow::MessageHandler(
  HWND hwnd,
  UINT const message,
  WPARAM const wparam,
  LPARAM const lparam
) noexcept {
  if (flutter_controller_) {
    std::optional<LRESULT> result = flutter_controller_ -> HandleTopLevelWindowProc(
      hwnd, message, wparam, lparam
    );

    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      flutter_controller_ -> engine() -> ReloadSystemFonts();
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}
