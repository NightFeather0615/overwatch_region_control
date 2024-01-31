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

#include <winerror.h>

#include "flutter/generated_plugin_registrant.h"


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

  flutter::MethodChannel<> channel (
    flutter_controller_ -> engine() -> messenger(),
    "overwatch_region_control.nightfeather.dev/firewall",
    &flutter::StandardMethodCodec::GetInstance()
  );



  channel.SetMethodCallHandler(
    [](
      const flutter::MethodCall<>& call,
      std::unique_ptr<flutter::MethodResult<>> call_result
    ) {
      OutputDebugStringW(L"Tasdasd");
      if (call.method_name() == "isFirewallEnabled") {
        bool pEnabled = false;
        HRESULT result = IsFirewallEnabled(&pEnabled);
        
        if (SUCCEEDED(result)) {
          call_result -> Success(pEnabled);
        } else {
          call_result -> Error("GET_FIREWALL_STATUS_FAILED", "Unabled to get firewall status.");
        }
      } else if (call.method_name() == "addFirewallRule") {
        std::map<std::string, std::any> example {
          {std::string("name"), std::string("TestRule")},
          {std::string("description"), std::string("Testing")},
          {std::string("app_name"), std::string("C:\\NightFeather\\Coding\\Flutter\\overwatch_region_control\\build\\windows\\x64\\runner\\Debug\\overwatch_region_control.exe")},
          {std::string("service_name"), std::string("Flutter App")},
          {std::string("protocol"), (long) 6},
          {std::string("action"), (long) 1},
          {std::string("direction"), (long) 2},
          {std::string("enabled"), false},
          {std::string("grouping"), std::string("_Flutter_App_Test")},
        };
        HRESULT result = AddFirewallRule(example);

        if (SUCCEEDED(result)) {
          call_result -> Success(true);
        } else {
          call_result -> Error(
            "GET_FIREWALL_STATUS_FAILED",
            result == E_ACCESSDENIED ? "no access" : "other"
          );
        }
      } else if (call.method_name() == "getFirewallRules") {
        flutter::EncodableList rules = {};
        HRESULT result = GetFirewallRules(&rules);
        
        if (SUCCEEDED(result)) {
          call_result -> Success(rules);
        } else {
          call_result -> Error(
            "GET_FIREWALL_STATUS_FAILED",
            result == E_ACCESSDENIED ? "no access" : "other"
          );
        }
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
