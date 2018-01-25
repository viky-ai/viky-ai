// App specific

// Sortable (https://github.com/RubaXa/Sortable)
window.Sortable = require('sortablejs');

// CodeMirror (https://github.com/codemirror/CodeMirror)
window.CodeMirror = require('codemirror');
import 'codemirror/addon/display/autorefresh'
import 'codemirror/mode/javascript/javascript'
import 'codemirror/lib/codemirror.css'

// Mousetrap (https://github.com/ccampbell/mousetrap)
window.Mousetrap = require('mousetrap/mousetrap.js');
require('mousetrap/plugins/global-bind/mousetrap-global-bind.js')

// Trix (https://github.com/basecamp/trix)
window.Trix = require('trix');
import 'trix/dist/trix.css'

// MomentJS (https://momentjs.com/)
import 'moment'

import './nav/nav';
import './profile/main';
import './authentication/main';
import './agents/agent_form';
import './agents/main';
import './console/console';
import './intents/main';
import './intents/intent_form';
import './intents/intents_list';
import './entities_lists/main';
import './entities_lists/entities_list_form';
import './entities_lists/entities_lists_list';
import './interpretations/main';
import './interpretations/interpretation_form';
window.App.InterpretationsList = require('./interpretations/interpretations_list');
