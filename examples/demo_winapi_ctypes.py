import os
import sys
import webview2

from ctypes import Structure, sizeof, byref, WINFUNCTYPE, c_long, c_longlong, windll, POINTER
from ctypes.wintypes import HWND, INT, UINT, WPARAM, LPARAM, HANDLE, LPCWSTR

is_64_bit = sys.maxsize > 2**32
LONG_PTR = c_longlong if is_64_bit else c_long

WNDPROC = WINFUNCTYPE(LONG_PTR, HWND, UINT, WPARAM, LPARAM)

class WNDCLASSEXW(Structure):
    def __init__(self, *args, **kwargs):
        super(WNDCLASSEXW, self).__init__(*args, **kwargs)
        self.cbSize = sizeof(self)
    _fields_ = [
        ("cbSize", UINT),
        ("style", UINT),
        ("lpfnWndProc", WNDPROC),
        ("cbClsExtra", INT),
        ("cbWndExtra", INT),
        ("hInstance", HANDLE),
        ("hIcon", HANDLE),
        ("hCursor", HANDLE),
        ("hBrush", HANDLE),
        ("lpszMenuName", LPCWSTR),
        ("lpszClassName", LPCWSTR),
        ("hIconSm", HANDLE)
    ]

user32 = windll.user32
user32.PostMessageW.argtypes = (HWND, UINT, WPARAM, LPARAM)
user32.DefWindowProcW.argtypes = (HWND, UINT, WPARAM, LPARAM)
user32.RegisterClassExW.argtypes = (POINTER(WNDCLASSEXW),)

COLOR_WINDOW = 5
CS_HREDRAW = 2
CS_VREDRAW = 1
CW_USEDEFAULT = -2147483648
IDC_ARROW = 32512
WM_CLOSE = 16
WM_QUIT = 18
WS_OVERLAPPEDWINDOW = 13565952
WS_VISIBLE = 268435456

APP_DIR = os.path.dirname(os.path.realpath(__file__))


class Main():

    def __init__(self):

        def window_proc_callback(hwnd, msg, wparam, lparam):
            if msg == WM_CLOSE:
                user32.PostMessageW(hwnd, WM_QUIT, 0, 0)
            return user32.DefWindowProcW(hwnd, msg, wparam, lparam)

        self.windowproc = WNDPROC(window_proc_callback)

        newclass = WNDCLASSEXW()
        newclass.lpfnWndProc = self.windowproc
        newclass.style = CS_VREDRAW | CS_HREDRAW
        newclass.lpszClassName = "WebView2DemoClass"
        newclass.hBrush = COLOR_WINDOW + 1
        newclass.hCursor = user32.LoadCursorW(0, IDC_ARROW)
        user32.RegisterClassExW(byref(newclass))

        hwnd = user32.CreateWindowExW(
            0,
            newclass.lpszClassName,
            "WebView2 Map Demo (WinAPI/ctypes)",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
            0, 0, 0, 0
        )

        self.webview = webview2.WebView(
            window = hwnd,
            icon = os.path.join(APP_DIR, "resources", "main.ico"),
            url = "https://59de44955ebd.github.io/map/?nofullscreen",
            autosize = True,
            debug = True,
        )

        self.is_fullscreen = False

        def toggle_fullscreen(id, args):
            self.is_fullscreen = not self.is_fullscreen
            self.webview.set_fullscreen(self.is_fullscreen)

        self.webview.js_bind("__on_fs__", toggle_fullscreen)
        self.webview.js_eval("window.addEventListener('keydown', (e) => {if (e.keyCode==122) __on_fs__();});")

    def run(self):
        self.webview.run()


if __name__ == "__main__":
    main = Main()
    main.run()
