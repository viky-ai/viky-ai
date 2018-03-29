// App specific

// Sortable (https://github.com/RubaXa/Sortable)
window.Sortable = require('sortablejs');

// Mousetrap (https://github.com/ccampbell/mousetrap)
window.Mousetrap = require('mousetrap/mousetrap.js');
require('mousetrap/plugins/global-bind/mousetrap-global-bind.js')

// Trix (https://github.com/basecamp/trix)
window.Trix = require('trix');
import 'trix/dist/trix.css'

// MomentJS (https://momentjs.com/)
import 'moment'

// Autosize (https://github.com/jackmoore/autosize)
import 'autosize'

import './nav/nav';
import './profile/main';
import './authentication/main';
import './agents/agent_form';
import './agents/agent_search_form';
import './agents/main';
import './agents/transfer_ownership_form';
import './dependencies/dependencies_filter';
import './console/console';
import './intents/main';
import './intents/intent_form';
import './entities_lists/main';
import './entities_lists/entities_list_form';
import './entities_lists/entities_import_form';
import './interpretations/main';
import './interpretations/interpretation_form';
import './entities/main';
window.App.InterpretationsList = require('./interpretations/interpretations_list');
window.App.EntityForm = require('./entities/entity_form');
