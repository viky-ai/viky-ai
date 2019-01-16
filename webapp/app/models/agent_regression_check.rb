class AgentRegressionCheck < ApplicationRecord
  belongs_to :agent

  serialize :expected, JSON
  serialize :got, JSON

  validates :sentence, length: { maximum: 2050 }, presence: true
  validates :expected, presence: true

  validates :expected_as_str, length: { maximum: 10_000 }

  enum state: [:unknown, :passed, :failed]

  private
    def expected_as_str
      expected.to_s
    end
end
