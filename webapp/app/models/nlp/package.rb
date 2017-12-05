require 'net/http'
include ActiveSupport::Benchmarkable

class Nlp::Package
  # Used to disable nlp sync if necessary via
  # Nlp::Package.sync_active = false
  class_attribute :sync_active
  self.sync_active = true

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  def initialize(agent)
    @agent = agent
  end

  def self.push_all
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info '  Skipping push_all packages to NLP because sync is deactivated'
      return
    end
    FileUtils.rm Dir.glob(File.join(Rails.root, 'import', '/*'))
    Agent.all.each do |agent|
      Nlp::Package.new(agent).push
    end
  end

  def destroy
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info "  Skipping destroy of package #{@agent.id} to NLP because sync is deactivated"
      return
    end
    FileUtils.rm(path)
    Rails.logger.info "  | Started DELETE: #{url} at #{Time.now}"
    uri = URI.parse(url)
    http = Net::HTTP.new(uri.host, uri.port)
    req = Net::HTTP::Delete.new(uri.path)
    res = http.request(req)
    Rails.logger.info "  | Completed from NLP, status: #{res.code}"
  end

  def push
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info "  Skipping push of package #{@agent.id} to NLP because sync is deactivated"
      return
    end
    benchmark "  NLP generate and push package: #{@agent.id}", level: :info do
      json = generate_json
      push_in_import_directory(json)

      Rails.logger.info "  | Started POST: #{url} at #{Time.now}"
      uri = URI.parse(url)
      http = Net::HTTP.new(uri.host, uri.port)
      res = http.post(uri.path, json, JSON_HEADERS)
      Rails.logger.info "  | Completed #{res.code}"
      Rails.logger.info "  | Error: #{JSON.parse(res.body)}" if res.code != "200"
      {
        status: res.code,
        body: JSON.parse(res.body)
      }
    end
  end

  def generate_json
    JSON.pretty_generate(
      JSON.parse(
        ApplicationController.render(
          template: 'agents/package',
          assigns: { agent: @agent, interpretations: build_tree },
          format: :json
        )
      )
    )
  end

  def endpoint
    ENV.fetch('VOQALAPP_NLP_URL') { 'http://localhost:9345' }
  end

  def url
    "#{endpoint}/packages/#{@agent.id}"
  end

  def path
    outdirname = File.join(Rails.root, 'import')
    FileUtils.mkdir outdirname unless File.exist?(outdirname)
    File.join(outdirname, "#{@agent.id}.json")
  end

  def logger
    Rails.logger
  end


  private

    def push_in_import_directory(json)
      File.open(path, 'w') { |file| file.write json }
    end

    def build_tree
      interpretations = []
      @agent.intents.each do |intent|
        interpretations += build_internals_list_nodes(intent)
        interpretations << build_node(intent)
      end
      interpretations
    end

    def build_internals_list_nodes(intent)
      interpretations = []
      intent.interpretations.each do |interpretation|
        interpretation.interpretation_aliases.where(is_list: true).order(:position_start).each do |ialias|
          interpretation_hash = {}
          interpretation_hash[:id] = "#{ialias.intent.id}_#{ialias.id}_recursive"
          interpretation_hash[:slug] = "#{ialias.intent.slug}_#{ialias.id}_recursive"

          expressions = []

          expression = {}
          expression[:expression] = "@{#{ialias.aliasname}}"
          expression[:aliases] = []
          expression[:aliases] << build_internal_alias(ialias)
          expression[:solution] = "`({ #{ialias.aliasname}: #{ialias.aliasname} })`"
          expressions << expression

          expression = {}
          expression[:expression] = "@{#{ialias.aliasname}} @{#{ialias.aliasname}_recursive}"
          expression[:aliases] = []
          expression[:aliases] << build_internal_alias(ialias)
          expression[:aliases] << build_internal_alias(ialias, true)
          expressions << expression

          if ialias.any_enabled
            expression = {}
            expression[:expression] = "@{#{ialias.aliasname}} @{#{ialias.aliasname}_recursive}"
            expression[:aliases] = []
            expression[:aliases] << {
              alias: ialias.aliasname,
              type: 'any'
            }
            expression[:aliases] << build_internal_alias(ialias, true)
            expressions << expression
          end

          interpretation_hash[:expressions] = expressions
          interpretations << interpretation_hash
        end
      end
      interpretations
    end

    def build_node(intent)
      interpretation_hash = {}
      interpretation_hash[:id] = intent.id
      interpretation_hash[:slug] = intent.slug
      expressions = []
      intent.interpretations.each do |interpretation|
        expression = {}
        expression[:expression] = interpretation.expression_with_aliases
        expression[:aliases] = build_aliases(interpretation)
        expression[:locale] = interpretation.locale unless interpretation.locale == '*'
        expression[:keep_order] = interpretation.keep_order if interpretation.keep_order
        expression[:glued] = interpretation.glued if interpretation.glued
        expression[:solution] = build_solution(interpretation)
        expressions << expression
        if interpretation.interpretation_aliases.where(any_enabled: true, is_list: false).count > 0
          interpretation.interpretation_aliases.where(any_enabled: true, is_list: false).each do |ialias|
            expressions << build_any_node(ialias, expression)
          end
        end
      end
      interpretation_hash[:expressions] = expressions
      interpretation_hash
    end

    def build_any_node(ialias, expression)
      any_aliasname = ialias.aliasname
      any_expression = expression.deep_dup
      old_aliases = expression[:aliases]
      any_expression[:aliases] = []
      old_aliases.each do |jsonalias|
        if jsonalias[:alias] == any_aliasname
          any_expression[:aliases] << {
            alias: any_aliasname,
            type: 'any'
          }
        else
          any_expression[:aliases] << jsonalias
        end
      end
      any_expression
    end

    def build_internal_alias(ialias, recursive=false)
      if recursive
        {
          alias: "#{ialias.aliasname}_recursive",
          slug: "#{ialias.intent.slug}_#{ialias.id}_recursive",
          id: "#{ialias.intent.id}_#{ialias.id}_recursive",
          package: @agent.id
        }
      else
        {
          alias: ialias.aliasname,
          slug: ialias.intent.slug,
          id: ialias.intent.id,
          package: @agent.id
        }
      end

    end

    def build_aliases(interpretation)
      result = []
      unless interpretation.interpretation_aliases.empty?
        result = interpretation.interpretation_aliases.order(:position_start).collect { |ialias| build_alias(ialias) }
      end
      result
    end

    def build_alias(ialias)
      if ialias.type_intent?
        result = {}
        result[:package] = @agent.id
        result[:alias] = ialias.aliasname
        if ialias.is_list
          result[:slug] = "#{ialias.intent.slug}_#{ialias.id}_recursive"
          result[:id] = "#{ialias.intent.id}_#{ialias.id}_recursive"
        else
          result[:slug] = ialias.intent.slug
          result[:id] = ialias.intent.id
        end
      end
      if ialias.type_digit?
        result = {
          alias: ialias.aliasname,
          type: 'digit'
        }
      end
      result
    end

    def build_solution(interpretation)
      result = ''
      if interpretation.auto_solution_enabled
        if interpretation.interpretation_aliases.empty?
          result = "`\"#{interpretation.expression.gsub('"', '\\"')}\"`"
        end
      else
        result = "`#{interpretation.solution}`" unless interpretation.solution.blank?
      end
      result
    end
end
