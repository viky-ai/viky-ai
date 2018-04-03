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

  def save
    @user.ui_state = @state
    @user.save(touch: false)
  end
end
