class AgentRegressionCheck < ApplicationRecord
  include Positionable
  positionable_ancestor :agent

  belongs_to :agent

  serialize :expected, JSON
  serialize :got, JSON

  validates :sentence, length: { maximum: 2050 }, presence: true
  validates :expected, presence: true

  validates :expected_as_str, length: { maximum: 10_000 }

  enum state: [:unknown, :success, :failure, :error, :running]

  before_destroy :check_running_state

  def run
    self.state = 'running'
    save

    request_params = {
      sentence: sentence,
      language: language,
      verbose: false,
      client_type: 'regression_test'
    }
    request_params[:now] = now.iso8601 if now.present?
    response = Nlp::PublicInterpret.request_public_api(request_params, agent)
    status = response[:status]

    if status == 200
      interpretation = JSON.parse(response[:body])['interpretations'].first
      if interpretation.nil?
        self.got = {}
      else
        self.got = {
          'package' => Intent.find(interpretation['id']).agent.id,
          'id' => interpretation['id'],
          'solution' => interpretation['solution'].to_json.to_s
        }
      end
      self.state = self.got == self.expected ? 'success' : 'failure'
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
      locals: { test: self, agent: self.agent, owner: self.agent.owner }
    )
  end


  private

    def expected_as_str
      expected.to_s
    end

    def check_running_state
      if self.state == 'running'
        errors.add(:base, I18n.t('errors.agent_regression_checks.delete.running'))
        throw(:abort)
      end
    end
end
