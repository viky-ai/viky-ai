import ApexCharts from 'apexcharts';

class RequestsQuota
  constructor: ->
    options = {
      chart:{
        type: 'area',
        stacked: true,
      },
      zoom: {
        autoScaleYaxis: true
      },
      series: requests_data.data
      dataLabels: {
        enabled: false
      },
      colors: ['#00e396', "#ff4560"],
      xaxis: {
        type: 'datetime',
        min: new Date(requests_data.from).getTime(),
        max: new Date(requests_data.to).getTime()
      },
      tooltip: {
        x: {
          format: 'dd MMM yyyy HH:mm'
        }
      }
    }
    chart = new ApexCharts(document.querySelector("#chart"), options)
    chart.render()

Setup = ->
  if $('body').data('controller-name') == "profiles"
    new RequestsQuota()

$(document).on('turbolinks:load', Setup)

