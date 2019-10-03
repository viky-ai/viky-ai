import ApexCharts from 'apexcharts';

class ProfileRadial
  constructor: ->
    @build(div) for div in $('.graph-radial')

  build: (div) ->
    chart = new ApexCharts(div, @graph_options(div))
    chart.render()

  graph_options: (div) ->
    {
      chart:
        height: 240
        type: 'radialBar'
        animations:
          enabled: false
      plotOptions:
        radialBar:
          hollow:
            size: '70%'
          dataLabels:
            showOn: "always"
            name:
              offsetY: 20
              show: true
              color: "#888"
              fontSize: "13px"
            value:
              offsetY: -15
              color: "#3bb8d7"
              fontSize: "24px"
              show: true
      stroke:
        lineCap: "round"
      series: $(div).data('series')
      labels: $(div).data('labels')
      colors: $(div).data('colors')
    }


class ProfileArea
  constructor: ->
    @build(div) for div in $('.graph-area')

  build: (div) ->
    chart = new ApexCharts(div, @graph_options(div))
    chart.render()

  graph_options: (div) ->
    {
      chart:
        type: 'area'
        stacked: true
        height: 300
        animations:
          enabled: false
        toolbar:
          tools:
            download: false
      legend:
        offsetY: -10
        onItemHover:
          highlightDataSeries: false
      stroke:
        width: 1
      fill:
        opacity: 1
        type: 'solid'
      dataLabels:
        enabled: false
      colors: ['#3bb8d7', "#ff3860"]
      series: $(div).data('series')
      xaxis:
        type: 'datetime'
        tooltip:
          enabled: false
      yaxis:
        labels:
          minWidth: 200
          maxWidth: 200
      tooltip:
        x:
          format: 'dd MMM yyyy HH:mm'
    }


class ProfileHeatMap
  constructor: ->
    @build(div) for div in $('.graph-heatmap')

  build: (div) ->
    chart = new ApexCharts(div, @graph_options(div))
    chart.render()

  graph_options: (div) ->
    {
      chart:
        height: ($(div).data('series').length * 20) + 44,
        type: 'heatmap'
        animations:
          enabled: false
        toolbar:
          show: false
      legend:
        show: false
      dataLabels:
        enabled: false
      plotOptions:
        heatmap:
          shadeIntensity: 0.5
          colorScale:
            ranges: [
              {
                from: -1,
                to: 0,
                color: '#dddddd'
              }
            ]
      colors: [$(div).data('color')]
      series: $(div).data('series')
      tooltip:
        custom: ({series, seriesIndex, dataPointIndex, w}) ->
          name = w.config.series[seriesIndex].name
          date = w.config.series[seriesIndex].data[dataPointIndex].x
          value = w.config.series[seriesIndex].data[dataPointIndex].y
          if value > 1
            return "<span class='heatmap-tooltips'>#{date} &mdash; #{name} agent &mdash; #{value} requests</span>"
          else
            return "<span class='heatmap-tooltips'>#{date} &mdash; #{name} agent &mdash; #{value} request</span>"
      yaxis:
        labels:
          minWidth: 200
          maxWidth: 200
      xaxis:
        labels:
          show: false
        axisBorder:
          show: false
        axisTicks:
          show: false
        tooltip:
          enabled: false
    }


Setup = ->
  if $('body').data('controller-name') == "profiles"
    new ProfileRadial()
    new ProfileArea()
    new ProfileHeatMap()

$(document).on('turbolinks:load', Setup)

