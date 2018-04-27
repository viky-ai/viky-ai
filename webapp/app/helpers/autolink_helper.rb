 module AutolinkHelper

  require 'html/pipeline'

  # cf. https://github.com/jch/html-pipeline/blob/a84d7fa441d545fc3bdd43d5003af67c53cfd2b8/lib/html/pipeline/sanitization_filter.rb#L40
  LISTS = Set.new(%w[ul ol].freeze)
  LIST_ITEM = 'li'.freeze
  ANCHOR_SCHEMES = ['http', 'https'].freeze
  WHITELIST = {
    elements: %w[
      strong em a p ul li code pre
    ],
    remove_contents: ['script'],
    attributes: {
      'a' => ['href', 'target', 'rel'],
    },
    protocols: {
      'a' => { 'href' => ANCHOR_SCHEMES },
    },
    transformers: [
      # Top-level <li> elements are removed because they can break out of
      # containing markup.
      lambda { |env|
        name = env[:node_name]
        node = env[:node]
        if name == LIST_ITEM && node.ancestors.none? { |n| LISTS.include?(n.name) }
          node.replace(node.children)
        end
      },
      # Force target="_blank" & rel="noopener" for links
      lambda { |env|
        name = env[:node_name]
        node = env[:node]
        if name == 'a'
          node.set_attribute('target', '_blank')
          node.set_attribute('rel', 'noopener')
        end
      }
    ]
  }.freeze

  def autolink(text)
    context = {
      link_attr: 'target="_blank" rel="noopener"',
      whitelist: WHITELIST
    }
    pipeline = HTML::Pipeline.new [
      HTML::Pipeline::SanitizationFilter,
      HTML::Pipeline::AutolinkFilter
    ], context
    result = pipeline.call(text)
    result[:output].to_s.html_safe
  end
end
