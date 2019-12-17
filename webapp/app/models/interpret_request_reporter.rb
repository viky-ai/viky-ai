class InterpretRequestReporter
  attr_accessor :query, :from, :to

  def initialize
    @query = { query: { bool: { must: [] } } }
  end

  def with_owner(owner_id)
    @query[:query][:bool][:must] << { match: { owner_id: owner_id } }
    self
  end

  def under_quota
    from_api
    @query[:query][:bool][:must] << { terms: { status: [200, 422, 500, 503] } }
    self
  end

  def over_quota
    from_api
    @query[:query][:bool][:must] << { terms: { status: [429] } }
    self
  end

  def from_api
    @query[:query][:bool][:must_not] = {
      terms: {
        "context.client_type": ["console", "regression_test", "play_ui"]
      }
    }
    self
  end

  def between(from, to)
    @from = from
    @to = to
    @query[:query][:bool][:must] << {
      range: {
        timestamp: {
          gte: from.to_s,
          lte: to.to_s,
          "time_zone": from.zone
        }
      }
    }
    self
  end

  def count_per_hours
    @query["from"] = 0
    @query["size"] = 0
    @query["aggregations"] = {
      over_time: {
        date_histogram: {
          field: "timestamp",
          interval: "1h",
          format: "date_time",
          time_zone: @from.zone,
          min_doc_count: 0,
          extended_bounds: { min: @from, max: @to }
        }
      }
    }
    proceed['aggregations']['over_time']['buckets']
  end

  def count_per_agent_and_per_day
    @query["from"] = 0
    @query["size"] = 0
    @query["aggregations"] = {
      agents: {
        terms: { "field": "agent_slug" },
        aggregations: {
          requests: {
            date_histogram: {
              field: "timestamp",
              interval: "1d",
              format: "date_time",
              time_zone: @from.zone,
              min_doc_count: 0,
              extended_bounds: { min: @from, max: @to }
            }
          }
        }
      }
    }
    proceed['aggregations']['agents']['buckets']
  end

  def count_per_owner
    @query["from"] = 0
    @query["size"] = 0
    @query["aggregations"] = {
      "owners": {
        "terms": { "field": "owner_id" }
      }
    }
    proceed['aggregations']['owners']['buckets']
  end

  def count
    client = InterpretRequestLogClient.build_client
    client.count_documents(@query)
  end


  private

    def proceed
      client = InterpretRequestLogClient.build_client
      client.search_documents(query)
    end

end
