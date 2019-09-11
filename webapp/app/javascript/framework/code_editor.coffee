$ = require('jquery');

class CodeEditor
  @buildJavaScriptEditor: (textarea, readonly=false)->
    return if $(textarea).length == 0

    options = {
      lineNumbers: true,
      mode: "javascript",
      autoRefresh: true,
      tabSize: 2,
      insertSoftTab: true,
      lineWrapping: true,
      extraKeys:
        'Tab': (editor) -> editor.execCommand 'insertSoftTab'
    }

    options.readOnly = 'nocursor' if readonly

    return CodeMirror.fromTextArea(textarea, options);

module.exports = CodeEditor
