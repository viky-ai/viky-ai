module StyleGuideHelper


  def highlight_html(bg_class = '')
    html = []
    value = nil
    buffer = with_output_buffer { value = yield }
    if (string = buffer.presence || value) && string.is_a?(String)
      html << "<div class='component-and-code' data-turbolinks='false'>"
      html << "<div class='component #{bg_class}'>"
      html << ERB::Util.html_escape(string)
      html << "</div>"

      html << "<div class='code #{bg_class}'>"
      html << "<pre>"
      html << "<code class='language-html'>"
      html << escape_once(string.strip)
      html << "</code>"
      html << "</pre>"
      html << "</div>"
      html << "</div>"
    end

    html.join.html_safe
  end


end
