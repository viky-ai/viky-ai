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


  test 'Readme markdown support' do
    readme = Readme.new(
      content: '## Hello'
    )
    readme.agent = agents(:weather)
    assert readme.save

    assert_equal "## Hello", agents(:weather).readme.content
    assert_equal "<h2>Hello</h2>", agents(:weather).readme.display

    table_content = "<table><tr><td>Hello</td></tr></table>"
    readme.content = table_content
    assert readme.save
    assert_equal table_content, agents(:weather).readme.display

    cite_content = "<cite>Hello</cite>"
    readme.content = cite_content
    assert readme.save
    assert_equal "<p>#{cite_content}</p>", agents(:weather).readme.display

    script_content = '<script>alert("hello")</script>'
    readme.content = script_content
    assert readme.save

    expected = "&lt;script&gt;alert(\"hello\")&lt;/script&gt;"
    assert_equal expected, agents(:weather).readme.display
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
