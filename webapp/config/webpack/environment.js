const { environment } = require('@rails/webpacker')
const coffee =  require('./loaders/coffee')
const { VueLoaderPlugin } = require('vue-loader')
const vue = require('./loaders/vue')

// resolve-url-loader must be used before sass-loader
environment.loaders.get('sass').use.splice(-1, 0, {
  loader: 'resolve-url-loader'
});

environment.plugins.prepend('VueLoaderPlugin', new VueLoaderPlugin())
environment.loaders.prepend('vue', vue)
environment.loaders.prepend('coffee', coffee)
module.exports = environment
