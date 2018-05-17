$ = require('jquery');
require 'slick-carousel/slick/slick.css';
require 'slick-carousel/slick/slick-theme.css';
require 'slick-carousel';

class ChatbotList
  constructor: ->
    for carrousel in $("#horizontal-list, #horizontal-list-with-card")
      $(carrousel).slick(
        dots: true,
        slidesToShow: 1,
        centerMode: true,
        infinite: false,
        centerPadding: '12px',
        arrows: false
      )

Setup = ->
  new ChatbotList()

$(document).on('turbolinks:load', Setup)
