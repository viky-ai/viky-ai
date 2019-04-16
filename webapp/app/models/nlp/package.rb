require 'net/http'
include ActiveSupport::Benchmarkable

class Nlp::Package
  # Used to disable nlp sync if necessary via
  # Nlp::Package.sync_active = false
  class_attribute :sync_active
  self.sync_active = true

  VERSION = 1  # Used to invalidate cache

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  REDIS_URL = ENV.fetch("VIKYAPP_REDIS_PACKAGE_NOTIFIER") { 'redis://localhost:6379/3' }

  def initialize(agent)
    @agent = agent
  end

  def self.reinit
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info '  Skipping push_all packages to NLP because sync is deactivated'
      return
    end
    event = :reinit
    redis_opts = {
      url: REDIS_URL
    }
    redis = Redis.new(redis_opts)
    redis.publish(:viky_packages_change_notifications, { event: event }.to_json)
    Rails.logger.info "  | Redis notify agent's #{event}"
  end

  def destroy
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info "  Skipping destroy of package #{@agent.id} to NLP because sync is deactivated"
      return
    end
    notify(:delete)
  end

  def push
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info "  Skipping push of package #{@agent.id} to NLP because sync is deactivated"
      return
    end
    notify(:update)
  end

  def generate_json(io)
    encoder = io.instance_of?(JsonChunkEncoder) ? io : JsonChunkEncoder.new(io)
    encoder.wrap_object do
      encoder.write_value('id', @agent.id)
      encoder.write_value('slug', @agent.slug)
      encoder.wrap_array('interpretations') do
        write_intent(encoder)
        write_entities_list(encoder)
      end
    end
  end

  def full_json_export(io)
    packages_list = full_packages_map(@agent).values
    encoder = JsonChunkEncoder.new io
    encoder.wrap_array do
      packages_list.each do |package|
        Nlp::Package.new(package).generate_json(encoder)
      end
    end
  end

  def logger
    Rails.logger
  end


  private

    def notify event
      redis_opts = {
        url: REDIS_URL
      }
      redis = Redis.new(redis_opts)
      redis.publish(:viky_packages_change_notifications, { event: event, id: @agent.id }.to_json  )
      Rails.logger.info "  | Redis notify agent's #{event} #{@agent.id}"
    end

    def full_packages_map(agent)
      return { agent.id => agent } if agent.successors.nil?
      result = { agent.id => agent }
      agent.successors.each do |successor|
        result.merge! full_packages_map(successor)
      end
      result
    end

    def write_intent(encoder)
      @agent.intents.order(position: :desc).each do |intent|
        cache_key = ['pkg', VERSION, @agent.slug, 'intent'.freeze, intent.id, (intent.updated_at.to_f * 1000).to_i].join('/')
        Rails.cache.fetch("#{cache_key}/build_internals_list_nodes") do
          build_internals_list_nodes(intent, encoder)
        end
        Rails.cache.fetch("#{cache_key}/build_node"){ build_intent(intent, encoder) }
      end
    end

    def write_entities_list(encoder)
      @agent.entities_lists.order(position: :desc).each do |elist|
        cache_key = ['pkg', VERSION, @agent.slug, 'entities_list'.freeze, elist.id, (elist.updated_at.to_f * 1000).to_i].join('/')
        Rails.cache.fetch("#{cache_key}/build_node"){ build_entities_list(elist, encoder) }
      end
    end

    def build_internals_list_nodes(intent, encoder)
      InterpretationAlias
        .includes(:interpretation)
        .where(is_list: true, interpretations: { intent_id: intent.id })
        .order('interpretations.position DESC, interpretations.locale ASC')
        .order(:position_start).each do |ialias|

        interpretation_hash = {}
        interpretation_hash['id']   = "#{ialias.interpretation_aliasable.id}_#{ialias.id}_recursive"
        interpretation_hash['slug'] = "#{ialias.interpretation_aliasable.slug}_#{ialias.id}_recursive"
        interpretation_hash['scope'] = 'hidden'

        expressions = []

        expression = {}
        expression['expression'] = "@{#{ialias.aliasname}}"
        expression['id'] = ialias.interpretation.id
        expression['aliases'] = []
        expression['aliases'] << build_internal_alias(ialias)
        expression['keep-order'] = ialias.interpretation.keep_order if ialias.interpretation.keep_order
        expression['glue-distance'] = ialias.interpretation.proximity.get_distance
        expression['glue-strength'] = 'punctuation'.freeze if ialias.interpretation.proximity_accepts_punctuations?
        expressions << expression

        expression = {}
        expression['expression'] = "@{#{ialias.aliasname}} @{#{ialias.aliasname}_recursive}"
        expression['id'] = ialias.interpretation.id
        expression['aliases'] = []
        expression['aliases'] << build_internal_alias(ialias)
        expression['aliases'] << build_internal_alias(ialias, true)
        expression['keep-order'] = ialias.interpretation.keep_order if ialias.interpretation.keep_order
        expression['glue-distance'] = ialias.interpretation.proximity.get_distance
        expression['glue-strength'] = 'punctuation'.freeze if ialias.interpretation.proximity_accepts_punctuations?
        expressions << expression

        interpretation_hash[:expressions] = expressions

        encoder.write_object interpretation_hash
      end
    end

    def build_intent(intent, encoder)
      encoder.wrap_object do
        encoder.write_value('id', intent.id)
        encoder.write_value('slug', intent.slug)
        encoder.write_value('scope', intent.is_public? ? 'public' : 'private')
        encoder.wrap_array('expressions') do
          intent.interpretations.order(position: :desc, locale: :asc).each do |interpretation|
            expression = {}
            expression['expression']    = interpretation.expression_with_aliases
            expression['id']            = interpretation.id
            aliases = build_aliases(interpretation)
            expression['aliases']       = aliases unless aliases.empty?
            expression['locale']        = interpretation.locale unless interpretation.locale == Locales::ANY
            expression['keep-order']    = interpretation.keep_order if interpretation.keep_order
            expression['glue-distance'] = interpretation.proximity.get_distance
            expression['glue-strength'] = 'punctuation'.freeze if interpretation.proximity_accepts_punctuations?
            solution = build_interpretation_solution(interpretation)
            expression['solution']      = solution unless solution.blank?

            encoder.write_object expression

            interpretation.interpretation_aliases
              .where(any_enabled: true, is_list: false)
              .order(position_start: :asc).each do |ialias|
              any_node = build_any_node(ialias, expression)
              encoder.write_object any_node
            end
          end
        end
      end
    end

    def build_entities_list(elist, encoder)
      encoder.wrap_object do
        encoder.write_value('id', elist.id)
        encoder.write_value('slug', elist.slug)
        encoder.write_value('scope', elist.is_public? ? 'public' : 'private')
        encoder.wrap_array('expressions') do
          elist.entities_in_ordered_batchs.each do |batch|
            batch.each do |entity|
              entity.terms.each do |term|
                expression = {}
                expression[:expression] = term['term']
                expression[:id] = entity.id
                expression[:locale] = term['locale'] unless term['locale'] == Locales::ANY
                expression[:solution] = build_entities_list_solution(entity)
                expression['keep-order'] = true
                expression['glue-distance'] = elist.proximity.get_distance
                expression['glue-strength'] = 'punctuation'.freeze if elist.proximity_glued?

                encoder.write_object expression
              end
            end
          end
        end
      end
    end

    def build_any_node(ialias, expression)
      any_aliasname = ialias.aliasname
      any_expression = expression.deep_dup
      old_aliases = expression['aliases']
      any_expression['aliases'] = []
      old_aliases.each do |jsonalias|
        if jsonalias['alias'] == any_aliasname
          any_expression['aliases'] << {
            'alias': any_aliasname,
            'type': 'any'
          }
        else
          any_expression['aliases'] << jsonalias
        end
      end
      any_expression
    end

    def build_internal_alias(ialias, recursive=false)
      if recursive
        {
          'alias': "#{ialias.aliasname}_recursive",
          'slug': "#{ialias.interpretation_aliasable.slug}_#{ialias.id}_recursive",
          'id': "#{ialias.interpretation_aliasable.id}_#{ialias.id}_recursive",
          'package': @agent.id
        }
      else
        {
          'alias': ialias.aliasname,
          'slug': ialias.interpretation_aliasable.slug,
          'id': ialias.interpretation_aliasable.id,
          'package': @agent.id
        }
      end
    end

    def build_aliases(interpretation)
      interpretation.interpretation_aliases
        .order(:position_start)
        .collect { |ialias| build_alias(ialias) }
    end

    def build_alias(ialias)
      result = {}
      result['alias'] = ialias.aliasname
      if ialias.type_number?
        result['type'] = 'number'
      elsif ialias.type_regex?
        result['type'] = 'regex'
        result['regex'] = ialias.reg_exp
      else
        result['package'] = @agent.id
        if ialias.is_list
          result['slug'] = "#{ialias.interpretation_aliasable.slug}_#{ialias.id}_recursive"
          result['id'] = "#{ialias.interpretation_aliasable.id}_#{ialias.id}_recursive"
        else
          result['slug'] = ialias.interpretation_aliasable.slug
          result['id'] = ialias.interpretation_aliasable.id
        end
      end
      result
    end

    def build_interpretation_solution(interpretation)
      if interpretation.auto_solution_enabled and interpretation.interpretation_aliases.empty?
        interpretation.expression
      elsif interpretation.solution.present?
        "`#{interpretation.solution}`"
      else
        ''
      end
    end

    def build_entities_list_solution(entity)
      if entity.auto_solution_enabled
        entity.terms.first['term']
      elsif entity.solution.present?
        "`#{entity.solution}`"
      else
        ''
      end
    end
end
