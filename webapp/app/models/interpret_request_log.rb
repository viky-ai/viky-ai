class InterpretRequestLog

  INDEX_NAME = ['stats-interpret_request_log', Rails.env].join('-').freeze
  INDEX_ALIAS_NAME = ['index', INDEX_NAME].join('-').freeze

  attr_reader :id, :timestamp, :sentence, :language

  def initialize(timestamp, sentence, language, id = nil)
    @id = id
    @timestamp = timestamp
    @sentence = sentence
    @language = language
  end

  def save
    client = IndexManager.client
    result = client.index index: INDEX_ALIAS_NAME, type: 'log', body: to_json, id: @id
    @id = result['_id']
  end


  private

    def to_json
      {
        timestamp: @timestamp,
        sentence: @sentence,
        language: @language
      }
    end
end
