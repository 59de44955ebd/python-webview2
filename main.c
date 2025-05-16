#include <Python.h>

#include "structmember.h"

#define WEBVIEW_IMPLEMENTATION
#include <webview/webview.h>

#include <windows.h>
#include <commctrl.h>

typedef struct {
	PyObject_HEAD webview_t w;
	BOOL bEmbedded;
	HWND hWndParent;
	HWND hWndWidget;
} WebView;

static void WebView_dealloc(WebView *self)
{
	webview_destroy(self->w);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *WebView_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	WebView *self = (WebView *)type->tp_alloc(type, 0);

	if (self == NULL)
	{
		return NULL;
	}
	memset(&self->w, 0, sizeof(self->w));

	return (PyObject *)self;
}

#define AUTO_SIZE_SUBCLASS_ID 1

//##########################################################
//
//##########################################################
LRESULT CALLBACK AutoSizeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
			SetWindowPos((HWND)dwRefData, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

//##########################################################
//
//##########################################################
static int WebView_init(WebView *self, PyObject *args, PyObject *kwds)
{
	int debug = 0;
	HWND hWndParent = NULL;
	int autosize = 0;
	int width=320, height=240;
	const char *url = NULL;
	const char *title = NULL;
	const char *icon = NULL;
	int bgcolor = 0xFFFFFF;

	static char *kwlist[] = {"debug", "window", "autosize", "width", "height", "url", "title", "icon", "bgcolor", NULL};

	if (!PyArg_ParseTupleAndKeywords(
		args,
		kwds,
		"|iiiiisssi",
		kwlist,
		&debug,
		&hWndParent,
		&autosize,
		&width,
		&height,
		&url,
		&title,
		&icon,
		&bgcolor
	))
	{
		return -1;
	}

	if (hWndParent)
	{
		if (icon)
			SetClassLongPtr(hWndParent, GCLP_HICON, (LONG_PTR)LoadImage(NULL, icon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
		if (title)
			SetWindowText(hWndParent, title);
	}

	self->w = webview_create(debug, (HWND)hWndParent, (COLORREF)bgcolor);

	self->hWndWidget = (HWND)webview_get_native_handle(self->w, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET);

	if (hWndParent)
	{
		self->bEmbedded = TRUE;
		self->hWndParent = hWndParent;
		if (autosize)
		{
			SetWindowSubclass(self->hWndParent, &AutoSizeProc, AUTO_SIZE_SUBCLASS_ID, (DWORD_PTR)self->hWndWidget);
			RECT rc;
			GetClientRect(self->hWndParent, &rc);
			SetWindowPos(self->hWndWidget, 
				NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else
			SetWindowPos(self->hWndWidget,
				NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		self->hWndParent = (HWND)webview_get_native_handle(self->w, WEBVIEW_NATIVE_HANDLE_KIND_UI_WINDOW);
		if (title)
			webview_set_title(self->w, title);
		if (icon)
			SetClassLongPtr(self->hWndParent, GCLP_HICON, (LONG_PTR)LoadImage(NULL, icon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
		webview_set_size(self->w, width, height, WEBVIEW_HINT_NONE);
	}

	if (url)
		webview_navigate(self->w, url);

	HWND hWnd = FindWindowEx(self->hWndWidget, NULL, "Chrome_WidgetWin_0", NULL);
	if (hWnd)
		hWnd = FindWindowEx(hWnd, NULL, "Chrome_WidgetWin_1", NULL);
	if (hWnd)
		SetFocus(hWnd);

	return 1;
}

//##########################################################
// Runs the main loop until it's terminated.
//##########################################################
static PyObject *WebView_run(WebView *self, PyObject *args)
{
	HWND hWnd = NULL;
	HACCEL hacc = NULL;
	if (!PyArg_ParseTuple(args, "|ii", &hWnd, &hacc))
	{
		return NULL;
	}

	webview_run(self->w, hWnd, hacc);
	Py_RETURN_NONE;
}

//##########################################################
// Stops the main loop.It is safe to call this function from another other
// background thread.
//##########################################################
static PyObject *WebView_terminate(WebView *self) {
	webview_terminate(self->w);
	Py_RETURN_NONE;
}

//##########################################################
// Updates the title of the native window.
//##########################################################
static PyObject *WebView_set_title(WebView *self, PyObject *args)
{
	const char *title = "";
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		return NULL;
	}
	webview_set_title(self->w, title);
	Py_RETURN_NONE;
}

//##########################################################
// Updates the size of the native window.
//
// @param width New width.
// @param height New height.
// @param hints Size hints
//##########################################################
static PyObject *WebView_set_size(WebView *self, PyObject *args)
{
	int width, height, hints = WEBVIEW_HINT_NONE;

	if (!PyArg_ParseTuple(args, "ii|i", &width, &height, &hints))
	{
		return NULL;
	}

	if (self->bEmbedded)
		SetWindowPos(self->hWndWidget,
			NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	else
		webview_set_size(self->w, width, height, hints);

	Py_RETURN_NONE;
}

//##########################################################
// Activates/deactivates fullscreen mode
// CUSTOM
//##########################################################
static PyObject *WebView_set_fullscreen(WebView *self, PyObject *args)
{
	int fullscreen = 0;
	if (!PyArg_ParseTuple(args, "i", &fullscreen))
	{
		return NULL;
	}

	if (fullscreen)
	{
		SetParent(self->hWndWidget, NULL);
		ShowWindow(self->hWndWidget, SW_SHOWMAXIMIZED);
		//SetWindowPos(hWndWidget, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		//ShowWindow(hWndParent, SW_HIDE);
	}
	else
	{
		//SetWindowPos(hWndWidget, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		SetParent(self->hWndWidget, self->hWndParent);
		ShowWindow(self->hWndWidget, SW_SHOWNORMAL);
		//ShowWindow(hWndParent, SW_SHOWNORMAL);
	}

  Py_RETURN_NONE;
}

//##########################################################
// Injects JavaScript code to be executed immediately upon loading a page.
// The code will be executed before @c window.onload.
//##########################################################
static PyObject *WebView_js_init(WebView *self, PyObject *args)
{
	const char *js = NULL;
	if (!PyArg_ParseTuple(args, "s", &js))
	{
		return NULL;
	}
	webview_init(self->w, js);
	Py_RETURN_NONE;
}

//##########################################################
// Evaluates arbitrary JavaScript code.
// Use bindings if you need to communicate the result of the evaluation.
//##########################################################
static PyObject *WebView_js_eval(WebView *self, PyObject *args)
{
	const char *js = NULL;
	if (!PyArg_ParseTuple(args, "s", &js))
	{
		return NULL;
	}
	webview_eval(self->w, js);
	Py_RETURN_NONE;
}

//##########################################################
//
//##########################################################
static void webview_bind_cb(const char *id, const char *req, void *arg)
{
	PyObject *cb = (PyObject *)arg;

	////PyObject_CallObject(cb, NULL);

	PyObject_CallFunction(cb, "ss", id, req);

	Py_XINCREF(cb);
}

//##########################################################
// Binds a function pointer to a new global JavaScript function.
//
// Internally, JS glue code is injected to create the JS function by the
// given name.The callback function is passed a request identifier,
// a request string and a user - provided argument.The request string is
// a JSON array of the arguments passed to the JS function.
//
// @param name Name of the JS function.
// @param fn Callback function.
//##########################################################
static PyObject *WebView_js_bind(WebView *self, PyObject *args)
{
	const char *name = NULL;
	PyObject *func;

	if (!PyArg_ParseTuple(args, "sO:set_callback", &name, &func))
	{
		return NULL;
	}

	if (!PyCallable_Check(func))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return NULL;
	}

	Py_XINCREF(func);

	webview_bind(self->w, name, webview_bind_cb, func);

	Py_RETURN_NONE;
}

//##########################################################
// Removes a binding created with webview_bind().
// @param name Name of the binding.
//##########################################################
static PyObject *WebView_js_unbind(WebView *self, PyObject *args)
{
	const char *name = NULL;

	if (!PyArg_ParseTuple(args, "s", &name))
	{
		return NULL;
	}

	webview_unbind(self->w, name);

	Py_RETURN_NONE;
}

//##########################################################
// Responds to a binding call from the JS side.
//
// This function is safe to call from another thread.
//
// @param id The identifier of the binding call.Pass along the value received
//           in the binding handler(see webview_bind()).
// @param status A status of zero tells the JS side that the binding call was
//               successful; any other value indicates an error.
// @param result The result of the binding call to be returned to the JS side.
//               This must either be a valid JSON value or an empty string for
//               the primitive JS value @c undefined.
//##########################################################
static PyObject *WebView_js_return(WebView *self, PyObject *args)
{
	const char *id = NULL;
	int status;
	const char *result = NULL;

	if (!PyArg_ParseTuple(args, "sis", &id, &status, &result))
	{
		return NULL;
	}

	webview_return(self->w, id, status, result);

	Py_RETURN_NONE;
}

//##########################################################
// Get a native handle of choice.
//##########################################################
static PyObject *WebView_get_native_handle(WebView *self, PyObject *args)
{
	int kind;

	if (!PyArg_ParseTuple(args, "i", &kind))
	{
		return NULL;
	}

	HWND hwnd = (HWND)webview_get_native_handle(self->w, kind);
	return PyLong_FromUnsignedLongLong((unsigned long long)hwnd);
}

//##########################################################
// Navigates webview to the given URL. URL may be a properly encoded data URI.
//##########################################################
static PyObject *WebView_navigate(WebView *self, PyObject *args)
{
	const char *url = NULL;
	if (!PyArg_ParseTuple(args, "s", &url))
	{
		return NULL;
	}
	webview_navigate(self->w, url);
	Py_RETURN_NONE;
}

//##########################################################
// Set/restore mouse scroll and keyboard focus to webview
// CUSTOM
//##########################################################
static PyObject *WebView_set_focus(WebView *self)
{
	HWND hWnd = FindWindowEx(self->hWndWidget, NULL, "Chrome_WidgetWin_0", NULL);
	if (hWnd)
		hWnd = FindWindowEx(hWnd, NULL, "Chrome_WidgetWin_1", NULL);
	if (hWnd)
		SetFocus(hWnd);
	Py_RETURN_NONE;
}

//##########################################################
// Load HTML content into the webview.
//##########################################################
static PyObject *WebView_set_html(WebView *self, PyObject *args)
{
	const char *html = NULL;
	if (!PyArg_ParseTuple(args, "s", &html))
	{
		return NULL;
	}
	webview_set_html(self->w, html);
	Py_RETURN_NONE;
}

static PyMemberDef WebView_members[] =
{
	{NULL} /* Sentinel */
};

static PyMethodDef WebView_methods[] =
{
	{"get_native_handle", (PyCFunction)WebView_get_native_handle, METH_VARARGS, "..."},
	{"js_bind", (PyCFunction)WebView_js_bind, METH_VARARGS, "..."},
	//{"js_dispatch", (PyCFunction)WebView_js_dispatch, METH_VARARGS, "..."},
	{"js_eval", (PyCFunction)WebView_js_eval, METH_VARARGS, "..."},
	{"js_init", (PyCFunction)WebView_js_init, METH_VARARGS, "..."},
	{"js_return", (PyCFunction)WebView_js_return, METH_VARARGS, "..."},
	{"js_unbind", (PyCFunction)WebView_js_unbind, METH_VARARGS, "..."},
	{"navigate", (PyCFunction)WebView_navigate, METH_VARARGS, "..."},
	{"run", (PyCFunction)WebView_run, METH_VARARGS, "..."},
	{"set_focus", (PyCFunction)WebView_set_focus, METH_VARARGS, "..."},
	{"set_fullscreen", (PyCFunction)WebView_set_fullscreen, METH_VARARGS, "..."},
	{"set_html", (PyCFunction)WebView_set_html, METH_VARARGS, "..."},
	{"set_size", (PyCFunction)WebView_set_size, METH_VARARGS, "..."},
	{"set_title", (PyCFunction)WebView_set_title, METH_VARARGS, "..."},
	{"terminate", (PyCFunction)WebView_terminate, METH_NOARGS, "..."},

	//{"resize", (PyCFunction)WebView_resize, METH_VARARGS, "..."},

    {NULL} /* Sentinel */
};

static PyTypeObject WebViewType =
{
    PyVarObject_HEAD_INIT(NULL, 0) "webview2.WebView", /* tp_name */
    sizeof(WebView),                                  /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)WebView_dealloc,                      /* tp_dealloc */
    0,                                                /* tp_print */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                                                /* tp_compare */
    0,                                                /* tp_repr */
    0,                                                /* tp_as_number */
    0,                                                /* tp_as_sequence */
    0,                                                /* tp_as_mapping */
    0,                                                /* tp_hash */
    0,                                                /* tp_call */
    0,                                                /* tp_str */
    0,                                                /* tp_getattro */
    0,                                                /* tp_setattro */
    0,                                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,         /* tp_flags */
    "WebView2 objects",                                /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    0,                                                /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    0,                                                /* tp_iter */
    0,                                                /* tp_iternext */
    WebView_methods,                                  /* tp_methods */
    WebView_members,                                  /* tp_members */
    0,                                                /* tp_getset */
    0,                                                /* tp_base */
    0,                                                /* tp_dict */
    0,                                                /* tp_descr_get */
    0,                                                /* tp_descr_set */
    0,                                                /* tp_dictoffset */
    (initproc)WebView_init,                           /* tp_init */
    0,                                                /* tp_alloc */
    WebView_new,                                      /* tp_new */
};

static PyMethodDef module_methods[] =
{
    {NULL} /* Sentinel */
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef =
{
    PyModuleDef_HEAD_INIT,
    "webview2",
    "Example module",
    0,
    module_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

#define MODINIT_ERROR NULL
#define MODINIT_NAME PyInit_webview2
#else
#define MODINIT_ERROR
#define MODINIT_NAME initwebview
#endif

PyMODINIT_FUNC MODINIT_NAME(void)
{
	PyObject *m;

	if (PyType_Ready(&WebViewType) < 0)
	{
		return MODINIT_ERROR;
	}

#if PY_MAJOR_VERSION >= 3
	 m = PyModule_Create(&moduledef);
#else
		m = Py_InitModule3("webview2", module_methods,
			"Example module that creates an extension type.");
#endif

	if (m == NULL)
	{
		return MODINIT_ERROR;
	}

	Py_INCREF(&WebViewType);
	PyModule_AddObject(m, "WebView", (PyObject *)&WebViewType);

	// Window size hints
	PyModule_AddIntConstant(m, "HINT_NONE", WEBVIEW_HINT_NONE);
	PyModule_AddIntConstant(m, "HINT_MIN", WEBVIEW_HINT_MIN);
	PyModule_AddIntConstant(m, "HINT_MAX", WEBVIEW_HINT_MAX);
	PyModule_AddIntConstant(m, "HINT_FIXED", WEBVIEW_HINT_FIXED);

	// Native handle kind.The actual type depends on the backend.
	PyModule_AddIntConstant(m, "NATIVE_HANDLE_KIND_UI_WINDOW", WEBVIEW_NATIVE_HANDLE_KIND_UI_WINDOW);
	PyModule_AddIntConstant(m, "NATIVE_HANDLE_KIND_UI_WIDGET", WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET);
	PyModule_AddIntConstant(m, "NATIVE_HANDLE_KIND_BROWSER_CONTROLLER", WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER);

#if PY_MAJOR_VERSION >= 3
		return m;
#endif
}
