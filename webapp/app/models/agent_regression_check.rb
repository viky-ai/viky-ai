class AgentRegressionCheck < ApplicationRecord
  include Positionable
  positionable_ancestor :agent, touch: false

  belongs_to :agent

  serialize :expected, JSON
  serialize :got, JSON

  validates :sentence, length: { maximum: 200 }, presence: true
  validates :expected, presence: true

  validates :expected_as_str, length: { maximum: 10_000 }

  enum state: [:unknown, :success, :failure, :error, :running]

  before_destroy :check_running_state
  before_validation :clean_sentence

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
        self.got = AgentRegressionCheck::prepare_result(interpretation)
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

  def self.prepare_result(interpretation)
    if interpretation['slug'].include? 'entities_lists'
      type = 'entities_list'
      interpret_root = EntitiesList.find_by_id(interpretation['id'])
    else
      type = 'intent'
      interpret_root = Intent.find_by_id(interpretation['id'])
    end

    if interpret_root.nil?
      {}
    else
      {
        'root_type' => type,
        'package' => interpret_root.agent.id,
        'id' => interpretation['id'],
        'solution' => interpretation['solution'].to_json.to_s
      }
    end
  end

  def interpret_root
    if expected['root_type'] == 'entities_list'
      EntitiesList.find_by_id(expected['id'])
    else
      Intent.find_by_id(expected['id'])
    end
  end

  private

    def clean_sentence
      self.sentence = sentence.strip unless sentence.nil?
    end

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
