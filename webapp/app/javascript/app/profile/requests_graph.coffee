import ApexCharts from 'apexcharts';

class RequestsGraph
  constructor: ->
    graph_container = $('#requests_graph')
    if graph_container.length == 1
      options = {
        chart:
          type: 'area'
          stacked: true
        zoom:
          type: 'xy'
          autoScaleYaxis: true
        series: requests_data.data
        dataLabels:
          enabled: false
        colors: ['#00e396', "#ff4560"]
        xaxis:
          type: 'datetime'
          tooltip:
            enabled: false
          min: new Date(requests_data.from).getTime(),
          max: new Date(requests_data.to).getTime()
        tooltip:
          x:
            format: 'dd MMM yyyy HH:mm'
      }
      chart = new ApexCharts(graph_container[0], options)
      chart.render()

Setup = ->
  if $('body').data('controller-name') == "profiles"
    new RequestsGraph()

$(document).on('turbolinks:load', Setup)

