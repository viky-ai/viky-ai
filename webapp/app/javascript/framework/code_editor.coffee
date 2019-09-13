$ = require('jquery');

class CodeEditor
  @buildJavaScriptEditor: (textarea, readonly=false, lineNumbers=true)->
    return if $(textarea).length == 0

    options = {
      lineNumbers: lineNumbers,
      mode: "javascript",
      autoRefresh: true,
      tabSize: 2,
      insertSoftTab: true,
      lineWrapping: true,
      extraKeys:
        'Tab': (editor) -> editor.execCommand 'insertSoftTab'
    }

    options.readOnly = true if readonly
    code_editor = CodeMirror.fromTextArea(textarea, options);
    $(code_editor.display.wrapper).addClass("CodeMirror--read-only") if readonly

    return code_editor

module.exports = CodeEditor
