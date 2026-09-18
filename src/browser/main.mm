#import <Cocoa/Cocoa.h>
#include "include/cef_application_mac.h"
#include "include/wrapper/cef_library_loader.h"
#include "drift_app.h"

// NSApplication subclass required by CEF on macOS for event loop integration.
@interface DriftApplication : NSApplication <CefAppProtocol>
@end

@implementation DriftApplication {
  BOOL _handlingSendEvent;
}
- (BOOL)isHandlingSendEvent { return _handlingSendEvent; }
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  _handlingSendEvent = handlingSendEvent;
}
- (void)sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  [super sendEvent:event];
}
- (void)terminate:(id)sender {
  CefQuitMessageLoop();
}
@end

int main(int argc, char* argv[]) {
  // CEF 152 uses dylib mode: must load the framework before any CEF call.
  // Helper processes (renderer, GPU, plugin) have --type= on the command line.
  // Helper processes (renderer, GPU, plugin) carry --type= on the command line.
  bool is_helper = false;
  for (int i = 1; i < argc; ++i) {
    if (strncmp(argv[i], "--type=", 7) == 0) {
      is_helper = true;
      break;
    }
  }

  CefScopedLibraryLoader library_loader;
  bool loaded = is_helper ? library_loader.LoadInHelper()
                          : library_loader.LoadInMain();
  if (!loaded) {
    fprintf(stderr, "Failed to load CEF framework\n");
    return 1;
  }

  @autoreleasepool {
    CefMainArgs main_args(argc, argv);
    CefRefPtr<DriftApp> app(new DriftApp);

    // Returns immediately in the browser process;
    // runs to completion in renderer/GPU/plugin helper processes.
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0)
      return exit_code;

    [DriftApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    CefSettings settings;
    settings.log_severity = LOGSEVERITY_WARNING;

    if (!CefInitialize(main_args, settings, app, nullptr))
      return 1;

    CefRunMessageLoop();
    CefShutdown();
  }
  return 0;
}
