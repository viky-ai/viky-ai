$ = require('jquery');

class CodeEditor
  @buildJavaScriptEditor: (textarea, readonly=false)->
    if $(textarea).length == 0
      return
    options = {
      lineNumbers: true,
      mode: "javascript",
      autoRefresh: true,
      tabSize: 2,
      insertSoftTab: true,
      extraKeys:
        'Tab': (editor) -> editor.execCommand 'insertSoftTab'
    }
    if readonly
      options.readOnly = 'nocursor'
    return CodeMirror.fromTextArea(textarea, options);

module.exports = CodeEditor
