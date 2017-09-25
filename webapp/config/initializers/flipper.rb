require 'flipper'
require 'flipper/adapters/memory'

# sets up default adapter so Flipper works like Flipper::DSL
FLIPPER = Flipper.new Flipper::Adapters::Memory.new
FLIPPER.disable :user_registration
