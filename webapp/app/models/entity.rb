class Entity < ApplicationRecord
  include Positionable
  positionable_ancestor :entities_list

  include ActionView::Helpers::NumberHelper

  belongs_to :entities_list, touch: true

  serialize :terms, JSON

  validates :solution, length: { maximum: 2000 }
  validates :solution, presence: true, if: -> { self.auto_solution_enabled }
  validates :terms, length: { maximum: 5000 }, presence: true
  validate :validate_locales_exists
  validate :validate_terms_present
  validate :validate_terms_length_in_bytes

  before_validation :parse_terms
  before_validation :build_solution
  after_validation :build_locales
  after_save :update_agent_locales

  def terms_to_s
    return '' if terms.blank?
    terms.collect { |term|
      if term['locale'] == Locales::ANY
        term['term']
      else
        "#{term['term']}:#{term['locale']}"
      end
    }.reject(&:blank?).join("\n")
  end


  private

    def parse_terms
      if terms.is_a?(String) && terms.length <= 5000
        self.terms = EntityTermsParser.new(terms).proceed
      end
    end

    def build_solution
      return unless self.auto_solution_enabled
      return if self.terms.blank?
      if terms.is_a? String
        self.solution = self.terms
      else
        self.solution = self.terms.first['term']
      end
    end

    def build_locales
      if errors.empty?
        locales = self.terms.collect{|t| t["locale"]}.uniq
        self.locales = Locales::ALL.select { |l| locales.include?(l) }
      end
    end

    def validate_locales_exists
      return unless terms.instance_of?(Array)
      terms.each do |json|
        unless Locales::ALL.include?(json['locale'])
          errors.add(:terms, I18n.t('errors.entity.unknown_locale', current_locale: json['locale']))
        end
      end
    end

    def update_agent_locales
      agent = entities_list.agent
      agent_locales = (agent.locales + locales).uniq
      if agent_locales != agent.locales
        agent.update_columns(locales: agent_locales)
      end
    end

    def validate_terms_present
      return unless terms.instance_of?(Array)
      if terms.any? { |json| json['term'].empty? }
        errors.add(:terms, I18n.t('errors.entity.term_abscent'))
      end
    end

    def validate_terms_length_in_bytes
      return unless terms.instance_of?(Array)
      terms.each do |json|
        actual_size = json['term'].bytesize
        if actual_size > 2048
          errors.add(:terms, I18n.t('errors.messages.too_large', actual: number_to_human_size(actual_size, precision: 4), count: number_to_human_size(2048, precision: 4)))
        end
      end
    end

end
