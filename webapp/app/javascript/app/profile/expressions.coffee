import ApexCharts from 'apexcharts';

class ExpressionsQuota
  constructor: ->
    options = {
      chart: {
        height: 280,
        type: "radialBar"
      },
      series: [90],
      plotOptions: {
        radialBar: {
          hollow: {
            margin: 15,
            size: "70%"
          },
          track: {
            dropShadow: {
              enabled: true,
              top: 2,
              left: 0,
              blur: 4,
              opacity: 0.15
            }
          },
          dataLabels: {
            name: {
              offsetY: -10,
              show: true,
              color: "#888",
              fontSize: "13px"
            },
            value: {
              color: "#111",
              fontSize: "30px",
              show: true,
              formatter: (val) ->
                val
            }
            total:{
              show: true,
              label: 'Remaining',
              formatter: (w) ->
                500
            }
          }
        }
      },
      stroke: {
        lineCap: "round",
      },
      labels: ["Used"]
    }
    chart = new ApexCharts(document.querySelector("#meter"), options)
    chart.render()

Setup = ->
  if $('body').data('controller-name') == "profiles"
    new ExpressionsQuota()

$(document).on('turbolinks:load', Setup)

