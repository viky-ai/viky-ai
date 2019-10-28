# frozen_string_literal: true

require_relative 'lib/task'

namespace :populate do
  desc 'Populate you viky with dummy agent for performance tests'
  task :agents, %i[email nb_agents] => [:environment] do |_t, args|
    user = User.find_by_email(args[:email])

    opts = {
      owner: user,
      nb_interpretation: 15,
      nb_formulation: 6,
      nb_entity_list: 4,
      nb_entity_list_entites: 100,
      nb_big_entity_list: 1,
      nb_big_entity_list_entites: 2000
    }

    args[:nb_agents].to_i.times do |i|
      Task::Populate.agent(opts)
      puts "Agent #{i} created"
    end

  end
end

class Task::Populate
  RAND_CORPUS = ('a'..'z').to_a
  def self.quick_rand(size)
    (0...size).map { RAND_CORPUS[rand(RAND_CORPUS.size)] }.join
  end

  def self.agent(opts)
    Nlp::Package.sync_active = false

    owner = opts[:owner]
    agent_name = "populate_#{quick_rand(10)}"
    agent = Agent.new(
      name: agent_name,
      agentname: agent_name,
      description: 'Dummy agent used for populate performance tests',
      visibility: 'is_private',
      memberships: [
        Membership.new(user_id: owner.id, rights: 'all')
      ]
    )
    agent.save!

    formulations(agent, opts)
    big_entiy_lists(agent, opts)
    entiy_lists(agent, opts)

    Nlp::Package.sync_active = true

    Nlp::Package.new(agent).push
  end

  def self.formulations(agent, opts)
    last_intent = nil
    before_last = nil

    opts[:nb_interpretation].times do
      intent = Intent.new(
        intentname: quick_rand(10),
        visibility: %w[is_public is_private].sample,
        description: 'Formulation of the world'
      )
      intent.agent = agent
      intent.save!

      opts[:nb_formulation].times do
        expressions = []
        rand(2..4).times do
          expressions << quick_rand(6)
        end

        formulation = Formulation.new(
          expression: expressions.join(' '),
          locale: ['fr', 'en', '*'].sample
        )
        formulation.intent = intent
        formulation.save!

        unless last_intent.nil?

          interpretation_alias = InterpretationAlias.new(
            aliasname: 'dummy_a',
            position_start: 0,
            position_end: 6
          )
          interpretation_alias.formulation = formulation
          interpretation_alias.interpretation_aliasable = last_intent
          interpretation_alias.save!

        end

        next if before_last.nil?

        interpretation_alias = InterpretationAlias.new(
          aliasname: 'dummy_b',
          position_start: 7,
          position_end: 13
        )
        interpretation_alias.formulation = formulation
        interpretation_alias.interpretation_aliasable = before_last
        interpretation_alias.save!
      end

      before_last = last_intent
      last_intent = intent
    end
  end

  def self.big_entiy_lists(agent, opts)
    opts[:nb_big_entity_list].times do
      entiy_lists_internal(agent, 'Big entities of the world', opts[:nb_big_entity_list_entites])
    end
  end

  def self.entiy_lists(agent, opts)
    opts[:nb_entity_list].times do
      entiy_lists_internal(agent, 'Entities of the world', opts[:nb_entity_list_entites])
    end
  end

  def self.entiy_lists_internal(agent, _desc, nb)
    entities_list = EntitiesList.new(
      listname: quick_rand(10),
      description: 'Entities of the world',
      visibility: 'is_private',
      agent: agent,
      proximity: 'glued'
    )
    entities_list.save!

    columns = %i[terms auto_solution_enabled entities_list_id position searchable_terms]
    entities = []
    count = 0
    nb.times do
      term = []
      rand(1..4).times do
        term << quick_rand(6)
      end
      terms = EntityTermsParser.new(term.join(' ') + ':' + %w[fr en].sample).proceed
      searchable_terms = Entity.extract_searchable_terms(terms)
      entities << [terms, true, entities_list.id, count, searchable_terms]
      count += 1
    end
    Entity.import(columns, entities, validate_with_context: :import)
    EntitiesList.update_counters(entities_list.id, entities_count: count)
  end
end
