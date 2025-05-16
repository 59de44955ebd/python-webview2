import os
import webview2

APP_DIR = os.path.dirname(os.path.realpath(__file__))

webview = webview2.WebView(
    width = 1024, height = 768,
    icon = os.path.join(APP_DIR, "resources", "main.ico"),
    title = "WebView2 Map Demo (Standalone)",
    url = "https://59de44955ebd.github.io/map/?nofullscreen",
    debug = True,
)

is_fullscreen = False

def toggle_fullscreen(id, args):
    global is_fullscreen
    is_fullscreen = not is_fullscreen
    webview.set_fullscreen(is_fullscreen)

webview.js_bind("__on_fs__", toggle_fullscreen)
webview.js_eval("window.addEventListener('keydown', (e) => {if (e.keyCode==122) __on_fs__();});")

webview.run()
