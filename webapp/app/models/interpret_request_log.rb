class InterpretRequestLog

  INDEX_NAME = ['stats-interpret_request_log', Rails.env].join('-').freeze
  INDEX_ALIAS_NAME = ['index', INDEX_NAME].join('-').freeze
  SEARCH_ALIAS_NAME = ['search', INDEX_NAME].join('-').freeze

  attr_reader :id, :timestamp, :sentence, :language

  def self.count(params = {})
    client = IndexManager.client
    result = client.count index: SEARCH_ALIAS_NAME, body: params
    result['count']
  end

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
