const { environment } = require('@rails/webpacker')
const coffee =  require('./loaders/coffee')

// resolve-url-loader must be used before sass-loader
environment.loaders.get('sass').use.splice(-1, 0, {
  loader: 'resolve-url-loader'
});

environment.loaders.append('coffee', coffee)
module.exports = environment
