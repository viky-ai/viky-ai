class AgentRegressionCheck < ApplicationRecord
  belongs_to :agent

  serialize :expected, JSON
  serialize :got, JSON

  validates :sentence, length: { maximum: 2050 }, presence: true
  validates :expected, presence: true

  validates :expected_as_str, length: { maximum: 10_000 }

  enum state: [:unknown, :passed, :failed, :error, :running]

  def run
    self.state = 'running'
    save

    request_params = {
      sentence: sentence,
      language: language,
      now: now.iso8601,
      verbose: false,
      client_type: 'regression_test',
    }
    response = Nlp::PublicInterpret.request_public_api(request_params, agent)
    status = response[:status]

    if status == 200
      interpretation = JSON.parse(response[:body])['interpretations'].sort_by { | interpretation | interpretation[:score].to_f }.last
      if interpretation.nil?
        self.got = {}
      else
        self.got = {
          'package' => Intent.find(interpretation['id']).agent.id,
          'id' => interpretation['id'],
          'score' => interpretation['score'].to_s,
          'solution' => interpretation['solution'].to_json.to_s
        }
      end
      self.state = self.got == self.expected ? 'passed' : 'failed'
    elsif status == 401 || status == 404
      self.got = ''
      self.state = 'unknown'
    else
      self.got = ''
      self.state = 'error'
    end
    save
  end

  def to_json
    ApplicationController.render(
      template: 'agent_regression_checks/_regression_check',
      locals: { test: self }
    )
  end

  private

    def expected_as_str
      expected.to_s
    end
end
