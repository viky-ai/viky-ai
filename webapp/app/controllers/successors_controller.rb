class SuccessorsController < ApplicationController
  before_action :set_owner_and_agent

  require 'rgl/dot' # write_to_graphic_file

  def new
    @available_successors = @agent.available_successors(current_user)
    render partial: 'new'
  end

  def create
    arc = AgentArc.new(source: @agent, target_id: params[:id])
    if arc.save
      redirect_to user_agent_path(@agent.owner, @agent.agentname)
    else
      error_msg  = "Dependency addition failed: "
      error_msg += arc.errors.full_messages.join(', ')
      error_msg += "."
      redirect_to user_agent_path(@agent.owner, @agent.agentname), alert: error_msg
    end
  end

  def successors_graph
    image_name = "#{Rails.root.join('tmp')}/#{@agent.id}_#{Time.now.to_i}"
    image = @agent.to_graph(&:slug).write_to_graphic_file('svg', image_name)
    @svg = File.open(image, 'rb').read
    File.delete(image, "#{image_name}.dot")
    render partial: 'graph', locals: { svg: @svg }
  end

  def predecessors_graph
    image_name = "#{Rails.root.join('tmp')}/#{@agent.id}_#{Time.now.to_i}"
    image = @agent.to_predecessors_graph(&:slug).write_to_graphic_file('svg', image_name)
    @svg = File.open(image, 'rb').read
    File.delete(image, "#{image_name}.dot")
    render partial: 'graph', locals: { svg: @svg }
  end

  def confirm_destroy
    @successor = Agent.friendly.find(params[:successor_id])
    render partial: 'confirm_destroy', locals: {
      owner: @owner,
      agent: @agent,
      successor: @successor
    }
  end

  def destroy
    @successor = Agent.friendly.find(params[:id])
    arc = AgentArc.where(source: @agent.id, target: @successor).first
    if arc.destroy
      redirect_to user_agent_path(@agent.owner, @agent.agentname)
    else
      redirect_to user_agent_path(@agent.owner, @agent.agentname), alert: t('views.successors.destroy.error_message')
    end
  end


  private

    def set_owner_and_agent
      begin
        @owner = User.friendly.find(params[:user_id])
        @agent = @owner.agents.friendly.find(params[:agent_id])
      rescue ActiveRecord::RecordNotFound
        redirect_to '/404'
      end
    end

end
