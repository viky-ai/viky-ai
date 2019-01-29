class AgentRegressionCheck < ApplicationRecord
  belongs_to :agent

  serialize :expected, JSON
  serialize :got, JSON

  validates :sentence, length: { maximum: 2050 }, presence: true
  validates :expected, presence: true

  validates :expected_as_str, length: { maximum: 10_000 }

  # [:unknown, :passed, :failed, :error, :running]
  enum state: [:unknown, :passed, :failed]

  def run(user_who_started_run, base_url)
    request_params = {
      sentence: sentence,
      language: language,
      now: now.iso8601,
      verbose: false,
      client_type: 'regression_test',
      user_id: user_who_started_run.id
    }
    response = Nlp::PublicInterpret.request_public_api(request_params, agent, base_url)
    status = response[:status]

    if status == 200
      interpretation = JSON.parse(response[:body])['interpretations'].sort_by { | interpretation | interpretation[:score].to_f }.last
      self.got = {
        'package' => Intent.find(interpretation['id']).agent.id,
        'id' => interpretation['id'],
        'score' => interpretation['score'].to_s,
        'solution' => interpretation['solution'].to_json.to_s
      }
      self.state = self.got == self.expected ? 'passed' : 'failed'
    else
      self.got = ''
      self.state = 'unknown'
    end
    save
  end


  private

    def expected_as_str
      expected.to_s
    end
end
