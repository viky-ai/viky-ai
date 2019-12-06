# frozen_string_literal: true

require 'fluent-logger'
require 'securerandom'

class InterpretRequestLog
  include ActiveModel::Model

  attr_accessor :id, :agents, :timestamp, :sentence, :language, :spellchecking, :now, :status, :body, :context

  validates :context_to_s, length: {
    maximum: 1000
  }

  def self.count(params = {})
    client = InterpretRequestLogClient.build_client
    client.count_documents(params)
  end

  def self.find(id)
    client = InterpretRequestLogClient.build_client
    result = client.get_document id
    params = result['_source'].symbolize_keys
    params[:id] = result['_id']
    params.delete(:agent_slug)
    params.delete(:owner_id)
    InterpretRequestLog.new params
  end

  def initialize(attributes = {})
    if attributes[:agents].blank?
      attributes[:agents] = Agent.where(id: attributes[:agent_id])
      attributes.delete(:agent_id)
    end
    super
    @id = SecureRandom.uuid
    @agents ||= Agent.where(id: attributes[:agent_id])
    @sentence ||= ''
    @context ||= {}
    @context['agent_version'] = @agents.map(&:updated_at)
    @persisted = false
  end

  def with_response(status, body)
    @status = status
    @body = body
    self
  end

  def save
    return false unless valid?

    # fluent does not support multiple index target
    # needed for parallel testing
    if Rails.env.test?

      client = InterpretRequestLogClient.build_client
      client.save_document(to_json, @id)

    else

      fluentbit_uri = URI(ENV.fetch('VIKYAPP_STATISTICS_FLUENTBIT_URL') { 'tcp://127.0.0.1:24224' })
      fluentbit_log = Fluent::Logger::FluentLogger.open(
        'stats',
        host: fluentbit_uri.host,
        port: fluentbit_uri.port,
        use_nonblock: true,
        wait_writeable: false,
        logger: Rails.logger
      )

      begin
        fluentbit_log.post(InterpretRequestLogClient.index_alias_name, to_json)
      rescue IO::EAGAINWaitWritable => e
        # wait code for avoding "Resource temporarily unavailable"
        # Passed records are stored into logger's internal buffer so don't re-post same event.
        Rails.logger.warn("Error on InterpretRequestLog.save : #{e.inspect}", to_json)
      end
    end

    @persisted = true
  end

  def persisted?
    @persisted
  end

  private

    def to_json
      result = {
        timestamp: @timestamp,
        sentence: @sentence,
        language: @language,
        spellchecking: @spellchecking,
        agent_id: @agents.map(&:id),
        agent_slug: @agents.map(&:slug),
        owner_id: @agents.map { |agent| agent.owner.id },
        status: @status,
        body: @body,
        context: @context.flatten_by_keys
      }
      result[:now] = @now if @now.present?
      result
    end

    def context_to_s
      @context.to_s
    end
end
