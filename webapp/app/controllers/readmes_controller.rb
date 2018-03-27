class ReadmesController < ApplicationController
  before_action :set_owner_and_agent
  before_action :set_readme, except: [:new, :create]
  before_action :check_user_rights

  def new
    @readme = Readme.new
    render partial: 'new'
  end

  def create
    @readme = Readme.new(readme_params)
    @readme.agent = @agent
    respond_to do |format|
      if @readme.save
        format.json do
          redirect_to user_agent_path(@owner, @agent), notice: t('views.readme.new.success_message')
        end
      else
        format.json do
          render json: {
            replace_modal_content_with: render_to_string(partial: 'new', formats: :html)
          }, status: 422
        end
      end
    end
  end

  def edit
    render partial: 'edit'
  end

  def update
    respond_to do |format|
      if @readme.update(readme_params)
        format.json {
          redirect_to user_agent_path(@owner, @agent), notice: t('views.readme.edit.success_message')
        }
      else
        format.json {
          render json: {
            replace_modal_content_with: render_to_string(partial: 'edit', formats: :html),
          }, status: 422
        }
      end
    end
  end

  def confirm_destroy
    render partial: 'confirm_destroy', locals: { readme: @readme }
  end

  def destroy
    if @readme.destroy
      redirect_to user_agent_path(@agent.owner, @agent), notice: t(
        'views.readme.destroy.success_message'
      )
    else
      redirect_to user_agent_intents_path(@agent.owner, @agent), alert: t(
        'views.readme.destroy.errors_message',
        errors: @readme.errors.full_messages.join(', ')
      )
    end
  end


  private

    def set_readme
      @readme = @agent.readme
    end

    def set_owner_and_agent
      begin
        @owner = User.friendly.find(params[:user_id])
        @agent = @owner.agents.friendly.find(params[:agent_id])
      rescue ActiveRecord::RecordNotFound
        redirect_to '/404'
      end
    end

    def readme_params
      params.require(:readme).permit(:content)
    end

    def check_user_rights
      case action_name
      when 'show'
        access_denied unless current_user.can? :show, @agent
      when 'new', 'create', 'edit', 'update', 'confirm_destroy', 'destroy'
        access_denied unless current_user.can? :edit, @agent
      else
        access_denied
      end
    end

end
