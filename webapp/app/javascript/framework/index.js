window.App = {}

// CSS reset
import 'normalize.css'

// PrismJs & Auto highlight
import 'prismjs/themes/prism.css';
require('prismjs');
require('prismjs/components/prism-json.js');
require('prismjs/components/prism-ruby.js');
import './highlight.coffee';

//CodeMirror (https://github.com/codemirror/CodeMirror)
window.CodeMirror = require('codemirror');
import 'codemirror/addon/display/autorefresh'
import 'codemirror/mode/javascript/javascript'
import 'codemirror/mode/gfm/gfm'
import 'codemirror/lib/codemirror.css'

// CSS framework part
import './main.scss'

// JS framework part
import './password.coffee';
import './dropdown.coffee';
window.App.Modal = require('./modal')
import './form_delete_validation.coffee'
import './color_picker.coffee'
window.App.Message = require('./message')
import './console.coffee'
window.App.FocusInput = require('./focus')
import './button_group.coffee'
import './upload_input.coffee'
import './highlight_and_scroll_to.coffee';
window.App.CodeEditor = require('./code_editor');
window.App.CardListHelper = require('./card_list');

// Expose jQuery
window.$ = require('jquery');
