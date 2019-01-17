class AgentRegressionCheck < ApplicationRecord
  belongs_to :agent

  serialize :expected, JSON
  serialize :got, JSON

  validates :sentence, length: { maximum: 2050 }, presence: true
  validates :expected, presence: true

  validates :expected_as_str, length: { maximum: 10_000 }

  enum state: [:unknown, :passed, :failed]

  def run
    nlp = Nlp::Interpret.new(
      ownername: agent.owner.username,
      agentname: agent.agentname,
      agent_token: agent.api_token,
      sentence: sentence,
      language: language,
      now: now.iso8601,
      verbose: false,
      format: 'json'
    )
    if nlp.valid?
      response = nlp.proceed
      status = response[:status]

      if status == '200'
        interpretation = response[:body]['interpretations'].sort_by { | interpretation | interpretation[:score].to_f }.last
        self.got = {
          'package' => interpretation['package'],
          'id' => interpretation['id'],
          'score' => interpretation['score'].to_s,
          'solution' => interpretation['solution'].to_json.to_s
        }
        self.state = self.got == self.expected ? 'passed' : 'failed'
      else
        self.got = ''
        self.state = 'unknown'
      end
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
