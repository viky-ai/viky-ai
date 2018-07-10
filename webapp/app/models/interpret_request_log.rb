class InterpretRequestLog

  INDEX_NAME = ['stats-interpret_request_log', Rails.env].join('-').freeze
  INDEX_ALIAS_NAME = ['index', INDEX_NAME].join('-').freeze
  SEARCH_ALIAS_NAME = ['search', INDEX_NAME].join('-').freeze

  def self.count(params = {})
    client = IndexManager.client
    result = client.count index: SEARCH_ALIAS_NAME, body: params
    result['count']
  end

  def initialize(params = {})
    @id = params[:id]
    @agent = params[:agent]
    @timestamp = params[:timestamp]
    @sentence = params.fetch(:sentence, '')
    @language = params[:language]
  end

  def save
    refresh = Rails.env == 'test'
    client = IndexManager.client
    result = client.index index: INDEX_ALIAS_NAME, type: 'log', body: to_json, id: @id, refresh: refresh
    @id = result['_id']
  end


  private

    def to_json
      {
        timestamp: @timestamp,
        sentence: @sentence,
        language: @language,
        agent_id: @agent.id,
        owner_id: @agent.owner.id
      }
    end
end
