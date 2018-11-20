// CSS reset
import 'normalize.css'

// Selectize
import 'selectize'
import 'selectize/dist/css/selectize.css'

// PrismJs & Auto highlight
import 'prismjs/themes/prism.css';
require('prismjs');
require('prismjs/components/prism-json.js');
require('prismjs/components/prism-ruby.js');
import './highlight';

// CodeMirror (https://github.com/codemirror/CodeMirror)
window.CodeMirror = require('codemirror');
import 'codemirror/addon/display/autorefresh'
import 'codemirror/mode/javascript/javascript'
import 'codemirror/mode/gfm/gfm'
import 'codemirror/lib/codemirror.css'

// CSS framework
import './main'

// Coffee
import './password';
import './dropdown';
window.App.Modal = require('./modal')
import './form_delete_validation'
import './color_picker'
window.App.Message = require('./message')
import './user_search'
import './console'
window.App.FocusInput = require('./focus')
import './button_group'
import './upload_input'
import './highlight_and_scroll_to';

// Expose jQuery
window.$ = require('jquery');

window.App.CodeEditor = require('./code_editor');
window.App.CardListHelper = require('./card_list');
