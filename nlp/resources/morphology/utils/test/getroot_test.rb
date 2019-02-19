# frozen_string_literal: true

$LOAD_PATH.unshift File.join(File.dirname(__FILE__))
require 'setup'

class GetrootTest < Test::Unit::TestCase
  def test_with_accent
    getroot = Getroot.new
    assert_equal 'a:0voir', getroot.process('a::avoir+Vmmp3s--')
    assert_equal nil, getroot.process('aisy::=+Ncms--')
    assert_equal 'ais:2voir', getroot.process('ais::avoir+Vmmp3s--')
    assert_equal 'été:5être', getroot.process('été::être+Vmps-sm-')
    assert_equal 'seriez:6être', getroot.process('seriez::être+Vmcp2p--')
    assert_equal 'fûmes:6être', getroot.process('fûmes::être+Vmis1p--')

    # unaccent
    assert_equal 'ete:1re', getroot.process('ete::etre+Vmps-sm-')
    assert_equal 'seriez:6etre', getroot.process('seriez::etre+Vmcp2p--')
    assert_equal 'fumes:5etre', getroot.process('fumes::etre+Vmis1p--')
  end
end
