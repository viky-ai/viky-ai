import ApexCharts from 'apexcharts';

class ExpressionsGraph
  constructor: ->
    graph_container = $('#expressions_graph')
    if graph_container.length == 1
      options = {
        chart:
          type: "donut"
          height: 350
        legend:
          fontSize: '16'
          onItemClick:
            toggleDataSeries: false
          onItemHover:
            highlightDataSeries: false
          itemMargin: 
            horizontal: 5
          formatter: (seriesName, opts) ->
            "#{seriesName} (#{opts.w.globals.series[opts.seriesIndex].toLocaleString()})"
        dataLabels:
          enabled: false
        plotOptions:
          pie:
            expandOnClick: false
            donut:
              size: '80%'
              labels:
                show: true
                name:
                  fontSize: '20'
                value:
                  fontSize: '20'
                  formatter: (val) ->
                    val = parseInt(val)
                    val.toLocaleString()
                total:
                  show:true
                  color: '#000'
                  label: expressions_data.total.toLocaleString()
                  formatter: ->
                    "used out of #{expressions_data.quota.toLocaleString()}"
        colors: ['#ffdd57', '#3bb8d7', '#d5d5d5'],
        series: expressions_data.data,
        labels: expressions_data.label
      }
      chart = new ApexCharts(graph_container[0], options)
      chart.render()

Setup = ->
  if $('body').data('controller-name') == "profiles"
    new ExpressionsGraph()

$(document).on('turbolinks:load', Setup)

