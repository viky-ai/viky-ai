require 'test_helper'

class EntityTermsParserTest < ActiveSupport::TestCase

  test "basic parsing" do
    p = EntityTermsParser.new("")
    assert_nil p.proceed

    p = EntityTermsParser.new(":")
    expected = [{"term" => "", "locale" => "*"}]
    assert_nil p.proceed

    p = EntityTermsParser.new(":fr")
    expected = [{"term" => "", "locale" => "fr"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris")
    expected = [{"term" => "paris", "locale" => "*"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("  paris  ")
    expected = [{"term" => "paris", "locale" => "*"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris:")
    expected = [{"term" => "paris", "locale" => "*"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris : ")
    expected = [{"term" => "paris", "locale" => "*"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris:fr")
    expected = [{"term" => "paris", "locale" => "fr"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("  paris  :  fr  ")
    expected = [{"term" => "paris", "locale" => "fr"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris : la divine:fr")
    expected = [{"term" => "paris : la divine", "locale" => "fr"}]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris\npanam:xy")
    expected = [
      {"term" => "paris", "locale" => "*"},
      {"term" => "panam", "locale" => "xy"}
    ]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris\npanam:xy:fr")
    expected = [
      {"term" => "paris", "locale" => "*"},
      {"term" => "panam:xy", "locale" => "fr"}
    ]
    assert_equal expected, p.proceed
  end


  test "de-duplication" do
    p = EntityTermsParser.new("paris\npanam:fr\nparis")
    expected = [
      {"term" => "paris", "locale" => "*"},
      {"term" => "panam", "locale" => "fr"}
    ]
    assert_equal expected, p.proceed

    p = EntityTermsParser.new("paris\n paris:en \n paris : en \npanam:fr\nparis:en\nparis")
    expected = [
      {"term" => "paris", "locale" => "*"},
      {"term" => "paris", "locale" => "en"},
      {"term" => "panam", "locale" => "fr"}
    ]
    assert_equal expected, p.proceed
  end

end
