import os
import webview2

from PyQt5.QtWidgets import QApplication, QMainWindow
# from PyQt6.QtWidgets import QApplication, QMainWindow
# from PySide6.QtWidgets import QApplication, QMainWindow

APP_DIR = os.path.dirname(os.path.realpath(__file__))


class Main(QMainWindow):

    def __init__(self):
        super().__init__()
        self.webview = webview2.WebView(
            window = int(self.winId()),
            autosize = True,
            icon = os.path.join(APP_DIR, "resources", "main.ico"),
            title = "WebView2 Map Demo (PyQt/PySide)",
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
        self.resize(1024, 768)
        self.show()

    def closeEvent(self, event):
        self.webview.terminate()

    def run(self):
        self.webview.run()


if __name__ == '__main__':
    app = QApplication([])
    main = Main()
    main.run()
