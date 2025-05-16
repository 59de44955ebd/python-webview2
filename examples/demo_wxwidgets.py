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
            url = "https://59de44955ebd.github.io/map/?nofullscreen",
            debug = True,
        )

        self.is_fullscreen = False

        def toggle_fullscreen(id, args):
            self.is_fullscreen = not self.is_fullscreen
            self.webview.set_fullscreen(self.is_fullscreen)

        self.webview.js_bind("__on_fs__", toggle_fullscreen)
        self.webview.js_eval("window.addEventListener('keydown', (e) => {if (e.keyCode==122) __on_fs__();});")

        self.Bind(wx.EVT_CLOSE, lambda e: self.webview.terminate())
        self.Centre()
        self.Show()

    def run(self):
        self.webview.run()


if __name__ == '__main__':
    app = wx.App()
    main = Main()
    main.run()
