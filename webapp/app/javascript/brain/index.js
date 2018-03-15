import 'normalize.css'
import './3d-force-graph/3d-force-graph.css'
import './main'

window.$     = require('jquery');
window.THREE = require('three');
window.TWEEN = require('@tweenjs/tween.js');
import ForceGraph3D from './3d-force-graph/3d-force-graph';
window.Brain = require('./app');

// Mute THREEjs WARNNINGS
console.warn = function () {
  return '';
}

$.getJSON('/brain.json', function (data) {
  var graph = ForceGraph3D()(document.getElementById("3d-graph"));
  new Brain(graph, data);
})
