require 'test_helper'

class ReadmeTest < ActiveSupport::TestCase

  test 'Basic readme creation & agent association' do
    readme = Readme.new(
      content: 'Hello'
    )
    assert_not readme.save
    readme.agent = agents(:weather)
    assert readme.save

    assert_equal "Hello", agents(:weather).readme.content
  end

  test 'Delete cascade' do
    assert_equal 1, Readme.count

    weather = agents(:weather)
    readme = Readme.new(
      content: 'Hello'
    )
    readme.agent = weather
    assert readme.save
    assert_equal 2, Readme.count

    weather.collaborators.each { |c| c.destroy }
    assert weather.destroy

    assert_equal 1, Readme.count
  end

end
