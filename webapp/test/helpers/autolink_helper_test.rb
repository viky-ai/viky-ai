class AutolinkHelperTest < ActionView::TestCase

  test "basic text" do
    assert_equal 'Text', autolink('Text')
  end

  test "autolink" do
    expected = 'Go to <a href="http://www.google.fr" target="_blank" rel="noopener">www.google.fr</a>'
    assert_equal expected, autolink('Go to www.google.fr')
  end

  test "link enrichment and sanitization" do
    expected = 'Go to <a href="http://www.google.fr/" target="_blank" rel="noopener">Google</a>'
    assert_equal expected, autolink('Go to <a href="http://www.google.fr/" class="btn">Google</a>')
  end

  test "html sanitization" do
    assert_equal "text", autolink('<b>text</b>')
    assert_equal "<strong>text</strong>", autolink('<strong>text</strong>')
    assert_equal "<em>text</em>", autolink('<em>text</em>')
    assert_equal "<code>text</code>", autolink('<code>text</code>')
    assert_equal "<p>text</p>", autolink('<p>text</p>')
    assert_equal "<ul><li>text</li></ul>", autolink('<ul><li>text</li></ul>')
    assert_equal "<pre>text</pre>", autolink('<pre>text</pre>')
    assert_equal "text", autolink('<li>text</li>')
  end

end
