require 'html/pipeline'

class Readme < ApplicationRecord
  belongs_to :agent, touch: true
  validates :content, presence: true, length: { maximum: 20_000 }

  def display
    whitelist = unfreeze HTML::Pipeline::SanitizationFilter::WHITELIST
    whitelist[:elements] = unfreeze whitelist[:elements]
    whitelist[:elements] << 'cite'
    context = {
      gfm: true,
      whitelist: whitelist
    }

    pipeline = HTML::Pipeline.new [
      HTML::Pipeline::MarkdownFilter,
      HTML::Pipeline::SanitizationFilter,
      HTML::Pipeline::AutolinkFilter
    ], context

    result = pipeline.call(content)
    result[:output].to_s.html_safe
  end

  private
    def unfreeze(obj)
      obj.dup
    end
end
