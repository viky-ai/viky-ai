import 'normalize.css'
import 'selectize'
import 'selectize/dist/css/selectize.css'
import './main'

window.$     = require('jquery');
window.THREE = require('three');
window.TWEEN = require('@tweenjs/tween.js');
import ForceGraph3D from './3d-force-graph/3d-force-graph';
import './3d-force-graph/3d-force-graph.css'
window.Brain = require('./app');

$(document).ready(function() {
  $(window).resize(function() {
    location.reload(true);
  });
  $.getJSON('/brain.json', function (data) {
    var graph = ForceGraph3D()(document.getElementById("3d-graph"));
    new Brain(graph, data);
  })
})
