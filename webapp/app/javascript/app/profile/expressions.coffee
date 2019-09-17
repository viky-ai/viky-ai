import ApexCharts from 'apexcharts';

class ExpressionsQuota
  constructor: ->
    options = {
      chart: {
        type: "radialBar"
      },
      plotOptions: {
        radialBar: {
          startAngle: -135,
          endAngle: 135,
          dataLabels: {
            name: {
              offsetY: 170,
            }
          }
        }
      },
      stroke: {
        dashArray: 4,
      },
      states: {
        hover: {
          filter: {
            type: 'none',
            value: 0
          }
        }
      }
      colors: ['#0086ea']
      series: [expressions_data.consumed],
      labels: [expressions_data.label]
    }
    chart = new ApexCharts(document.querySelector("#meter"), options)
    chart.render()

Setup = ->
  if $('body').data('controller-name') == "profiles"
    new ExpressionsQuota()

$(document).on('turbolinks:load', Setup)

