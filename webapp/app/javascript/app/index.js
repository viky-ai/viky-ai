// App specific
window.Sortable = require('sortablejs');

// Trix
window.Trix = require('trix');
import 'trix/dist/trix.css'

import './nav/nav';
import './profile/main';
import './authentication/main';
import './intents/intent_form';
import './intents/intents_list';
import './agents/agent_form';
import './agents/main';
import './console/console';
import './interpretations/main';
import './interpretations/interpretation_form';

window.App.InterpretationsList = require('./interpretations/interpretations_list');
