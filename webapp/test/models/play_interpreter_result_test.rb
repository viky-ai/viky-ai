require 'test_helper'

class PlayInterpreterResultTest < ActiveSupport::TestCase

  test 'succeed?, failed?, errors' do
    body = {
      errors: ["First error", "Second error"]
    }
    r = PlayInterpreterResult.new(422, body)

    assert_not r.succeed?
    assert r.failed?
    assert_equal "First error, Second error", r.error_messages

    r = PlayInterpreterResult.new(200, sample_body)
    assert r.succeed?
    assert_not r.failed?
    assert_nil r.error_messages
  end


  test 'interpretations_count' do
    r = PlayInterpreterResult.new(200, sample_body)
    assert_equal 3, r.interpretations_count

    r = PlayInterpreterResult.new(200, blank_body)
    assert_equal 0, r.interpretations_count
  end


  test 'interpretations_count_per_agent' do
    r = PlayInterpreterResult.new(200, blank_body)
    expected = {}
    assert_equal expected, r.interpretations_count_per_agent

    r = PlayInterpreterResult.new(200, sample_body)
    expected = {
      agents(:weather).id => 2,
      agents(:terminator).id => 1,
    }
    assert_equal expected, r.interpretations_count_per_agent
  end


  test 'interpretations_count_for_agent' do
    r = PlayInterpreterResult.new(200, blank_body)
    assert_equal 0, r.interpretations_count_for_agent(agents(:weather).id)

    r = PlayInterpreterResult.new(200, sample_body)
    assert_equal 2, r.interpretations_count_for_agent(agents(:weather).id)
    assert_equal 1, r.interpretations_count_for_agent(agents(:terminator).id)
  end


  test 'slug_colors' do
    r = PlayInterpreterResult.new(200, blank_body)
    expected = {}
    assert_equal expected, r.slug_colors

    r = PlayInterpreterResult.new(200, sample_body)
    expected = {
      "admin/weather/weather_forecast" => 'pink',
      "admin/weather/weather_question" => 'indigo',
      "admin/terminator/terminator_find" => 'cyan'
    }
    assert_equal expected, r.slug_colors
  end


  private

  def blank_body
    { 'interpretations' => {} }
  end

  def sample_body
    {
      'interpretations' => [
        {
          "package" => agents(:weather).id,
          "id" => intents(:weather_forecast).id,
          "slug" => "admin/weather/weather_forecast",
          "name" => "weather_forecast",
          "score" => 1.0,
          "start_position" => 0,
          "end_position"=> 10,
          "solution"=> {}
        },
        {
          "package" => agents(:weather).id,
          "id" => intents(:weather_question).id,
          "slug" => "admin/weather/weather_question",
          "name" => "weather_question",
          "score" => 1.0,
          "start_position" => 20,
          "end_position"=> 30,
          "solution"=> {}
        },
        {
          "package" => agents(:terminator).id,
          "id" => intents(:terminator_find).id,
          "slug" => "admin/terminator/terminator_find",
          "name" => "terminator_find",
          "score" => 1.0,
          "start_position" => 40,
          "end_position"=> 100,
          "solution"=> {}
        }
      ]
    }
  end

end
