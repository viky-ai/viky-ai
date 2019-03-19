class UserUiState

  def initialize(current_user)
    @user = current_user
    @state = current_user.ui_state
  end

  def agent_search=(new_state)
    @state.merge!(
      agent_search: {
        query: new_state[:query],
        sort_by: new_state[:sort_by],
        filter_owner: new_state[:filter_owner],
        filter_visibility: new_state[:filter_visibility]
      }
    )
  end

  def agent_search
    (@state['agent_search'] || {}).with_indifferent_access
  end

  def agent_locale=(locale)
    @state.merge!(
      agent_locale: locale
    )
  end

  def agent_locale(current_agent)
    if current_agent.locales.include? @state['agent_locale']
      @state['agent_locale']
    else
      current_agent.ordered_locales.first
    end
  end

  def chatbot_search=(new_state)
    @state.merge!(
      chatbot_search: {
        query: new_state[:query],
        filter_wip: new_state[:filter_wip],
      }
    )
  end

  def chatbot_search
    (@state['chatbot_search'] || {}).with_indifferent_access
  end

  def last_destination_agent=(new_state)
    @state.merge!(
      last_agent_move: new_state
    )
  end

  def last_destination_agent(current_agent)
    result = nil
    if @state['last_agent_move'].present?
      last_agent = Agent.find_by id: @state['last_agent_move']
      if last_agent
        is_last_agent_available = current_agent.available_destinations({ user_id: @user.id }).one? { |agent| agent.id == last_agent.id }
        result = last_agent if is_last_agent_available
      end
    end
    result
  end

  def save
    @user.ui_state = @state
    @user.save(touch: false)
  end
end
