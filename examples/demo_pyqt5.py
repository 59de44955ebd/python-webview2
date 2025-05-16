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
            url = "https://59de44955ebd.github.io/map/?nofullscreen",
            debug = True,
        )
        self.is_fullscreen = False
        def toggle_fullscreen(id, args):
            self.is_fullscreen = not self.is_fullscreen
            self.webview.set_fullscreen(self.is_fullscreen)
        self.webview.js_bind("__on_fs__", toggle_fullscreen)
        self.webview.js_eval("window.addEventListener('keydown', (e) => {if (e.keyCode==122) __on_fs__();});")
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
