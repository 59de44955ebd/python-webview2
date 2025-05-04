import webview2

webview = webview2.WebView()
webview.set_html('''
<br>
<p>
  <label for="inp">Expression:</label>
  <input id="inp" value="23 * 7 - 42" style="width: 100%">
</p>
<p><button id="btn">Evaluate in Python</button></p>
<p>
  <label for="res">Result:</label>
  <input id="res" style="width: 100%">
</p>
<script>
document.getElementById('btn').addEventListener("click", async () => {
    document.getElementById('res').value = await __py_eval__(document.getElementById('inp').value);
});
</script>
''')

def python_eval(id, args):
    try:
        expression = eval(args)[0]  # quick & dirty, better use json.loads
        webview.js_return(id, 0, str(eval(expression)))
    except Exception as e:
        webview.js_return(id, 0, f'"Error: {e}"')  # quick & dirty, better use json.dumps

webview.js_bind("__py_eval__", python_eval)
webview.run()
