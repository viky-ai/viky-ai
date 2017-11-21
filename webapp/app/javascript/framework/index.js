// CSS reset
import 'normalize.css'

// Selectize
import 'selectize'
import 'selectize/dist/css/selectize.css'

// PrismJs & Auto highlight
import 'prismjs/themes/prism.css';
require('prismjs');
import './highlight';

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

// Expose jQuery
window.$ = require('jquery');
