import os
import webview2

APP_DIR = os.path.dirname(os.path.realpath(__file__))

webview = webview2.WebView(
    width = 1024, height = 768,
    icon = os.path.join(APP_DIR, "resources", "main.ico"),
    title = "WebView2 Map Demo (Standalone)",
    url = "https://59de44955ebd.github.io/map/",
    debug = True,
)
webview.js_bind("__on_fs__", lambda id, args: webview.set_fullscreen(eval(args)[0]))
webview.js_eval("""
document.addEventListener(
    'fullscreenchange',
    () => __on_fs__(+!map._isFullscreen)
)
""")
webview.run()
