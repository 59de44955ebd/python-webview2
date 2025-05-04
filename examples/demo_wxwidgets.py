import os
import webview2
import wx

APP_DIR = os.path.dirname(os.path.realpath(__file__))


class Main(wx.Frame):

    def __init__(self):
        super().__init__(None, -1, size=(1024, 768))

        self.webview = webview2.WebView(
            window = self.GetHandle(),
            autosize = True,
            icon = os.path.join(APP_DIR, "resources", "main.ico"),
            title = "WebView2 Map Demo (wxWidgets)",
            url = "https://59de44955ebd.github.io/map/",
            debug = True,
        )
        self.webview.js_bind("__on_fs__", lambda id, args: self.webview.set_fullscreen(eval(args)[0]))
        self.webview.js_eval("""
        document.addEventListener(
            'fullscreenchange',
            () => __on_fs__(+!map._isFullscreen)
        )
        """)
        self.Bind(wx.EVT_CLOSE, lambda e: self.webview.terminate())
        self.Centre()
        self.Show()

    def run(self):
        self.webview.run()


if __name__ == '__main__':
    app = wx.App()
    main = Main()
    main.run()
