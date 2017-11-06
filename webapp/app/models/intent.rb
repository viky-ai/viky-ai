class Intent < ApplicationRecord

  belongs_to :agent

  validates :intentname, uniqueness: { scope: [:agent_id] }, length: { in: 3..25 }, presence: true

  before_validation :clean_intentname

  private

  def clean_intentname
    unless intentname.nil?
      self.intentname = intentname.parameterize(separator: '-')
    end
  end
end