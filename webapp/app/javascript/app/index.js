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

// LocalTime (updated_at & cache)
import LocalTime from "local-time"
LocalTime.start()

// ApexCharts
import 'apexcharts'
import './nav/nav.coffee';

import './profile/main.scss';
import './profile/requests_graph.coffee';
import './profile/expressions_graph.coffee';

import './authentication/main.scss';

import './chatbots/main.scss';
window.App.Statement = require('./chatbots/statements');

import './play/main.scss';
import './play/chooser.coffee';
import './play/form.coffee';

import './agents/agent_form.coffee';
import './agents/main.scss';
import './agents/transfer_ownership_form.coffee';
window.App.AgentDuplicator = require('./agents/duplicator');

import './bots/main.scss';
import './bots/bot_form.coffee';

import './dependencies/dependencies_filter.coffee';

import './console/console.coffee';
import './console/console_explain_footer.coffee'
import './console/console_footer.coffee';
import './console/console_test_suite.coffee';

import './intents/main.scss';
import './intents/intent_form.coffee';

import './entities_lists/main.scss';
import './entities_lists/entities_list_form.coffee';
import './entities_lists/entities_import_form.coffee';

window.App.EntitiesImport = require('./entities_lists/entities_import');

import './interpretations/main.scss';
import './interpretations/interpretation_form.coffee';

import './entities/main.scss';

import './aliased_intents/main.scss';

import './readme/form.coffee';

import './dashboard/main.scss';
import './dashboard/dashboard.coffee';

window.App.InterpretationsList = require('./interpretations/interpretations_list');
window.App.EntityForm = require('./entities/entity_form');
