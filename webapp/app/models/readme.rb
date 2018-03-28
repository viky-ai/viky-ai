require 'html/pipeline'

class Readme < ApplicationRecord
  belongs_to :agent, touch: true
  validates :content, presence: true

  def display
    pipeline = HTML::Pipeline.new [
      HTML::Pipeline::MarkdownFilter,
      HTML::Pipeline::SanitizationFilter,
      HTML::Pipeline::AutolinkFilter
    ]
    result = pipeline.call(content)
    result[:output].to_s.html_safe
  end
end
