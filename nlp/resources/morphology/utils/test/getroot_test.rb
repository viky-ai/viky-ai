# frozen_string_literal: true

$LOAD_PATH.unshift File.join(File.dirname(__FILE__))
require 'setup'

class GetrootTest < Test::Unit::TestCase
  def test_type_root
    r = Getroot.new
    assert_equal 'a:0voir', r.process('a::avoir+Vmmp3s--', :root)
    assert_equal nil, r.process('aisy::=+Ncms--', :root)
    assert_equal 'ais:2voir', r.process('ais::avoir+Vmmp3s--', :root)
    assert_equal 'été:5être', r.process('été::être+Vmps-sm-', :root)
    assert_equal 'seriez:6être', r.process('seriez::être+Vmcp2p--', :root)
    assert_equal 'fûmes:6être', r.process('fûmes::être+Vmis1p--', :root)

    # unaccent
    assert_equal 'ete:1re', r.process('ete::etre+Vmps-sm-', :root)
    assert_equal 'seriez:6etre', r.process('seriez::etre+Vmcp2p--', :root)
    assert_equal 'fumes:5etre', r.process('fumes::etre+Vmis1p--', :root)
  end

  def test_type_form
    r = Getroot.new
    assert_equal 'avoir:4', r.process('a::avoir+Vmmp3s--', :form)
    assert_equal nil, r.process('aisy::=+Ncms--', :form)
    assert_equal 'avoir:4is', r.process('ais::avoir+Vmmp3s--', :form)
    assert_equal 'être:5été', r.process('été::être+Vmps-sm-', :form)
    assert_equal 'être:5seriez', r.process('seriez::être+Vmcp2p--', :form)
    assert_equal 'être:5fûmes', r.process('fûmes::être+Vmis1p--', :form)

    # unaccent
    assert_equal 'etre:2e', r.process('ete::etre+Vmps-sm-', :form)
    assert_equal 'etre:4seriez', r.process('seriez::etre+Vmcp2p--', :form)
    assert_equal 'etre:4fumes', r.process('fumes::etre+Vmis1p--', :form)
  end
end
