 module AutolinkHelper

  require 'html/pipeline'

  def autolink(text)
    context = {
      link_attr: 'target="_blank"'
    }
    pipeline = HTML::Pipeline.new [
      HTML::Pipeline::AutolinkFilter
    ], context
    result = pipeline.call(text)
    result[:output].to_s.html_safe
  end
end
