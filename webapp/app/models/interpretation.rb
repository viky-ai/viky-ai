class Interpretation < ApplicationRecord
  Locales = %w[en-US fr-FR *].sort.freeze

  belongs_to :intent

  validates :expression, presence: true
  validates :locale, inclusion: { in: self::Locales }, presence: true

  before_create :set_position

  after_save do
    Nlp::Package.new(intent.agent).push
  end

  after_destroy do
    Nlp::Package.new(intent.agent).push
  end


  private

    def set_position
      unless intent.nil?
        self.position = intent.interpretations_with_local(self.locale).count
      end
    end
end
