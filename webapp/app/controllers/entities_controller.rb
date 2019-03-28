class EntitiesController < ApplicationController
  skip_before_action :verify_authenticity_token, only: [:update_positions]
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_entities_list
  before_action :set_entity, except: [:create, :update_positions, :new_import, :create_import]

  def create
    entity = Entity.new(entity_params)
    entity.entities_list = @entities_list
    respond_to do |format|
      if entity.save
        format.js do
          redirect_to user_agent_entities_list_path(@owner, @agent, @entities_list),
            notice: t('views.entities.new.success_message')
        end
      else
        format.js do
          @html_form = render_to_string(partial: 'form', locals: {
            agent: @agent,
            entities_list: @entities_list,
            entity: entity
          })
          render partial: 'create_failed', locals: { entity: entity }
        end
      end
    end
  end

  def edit
    respond_to do |format|
      format.js {
        @form = render_to_string(partial: 'edit.html', locals: { entity: @entity })
        render partial: 'edit'
      }
    end
  end

  def update
    respond_to do |format|
      if @entity.update(entity_params)
        format.js {
          @show = render_to_string(partial: 'entity', locals: {
            entity: @entity,
            can_edit: current_user.can?(:edit, @agent),
            entities_list: @entities_list,
            agent: @agent,
            owner: @owner
          })
          render partial: 'show'
        }
      else
        format.js do
          @form = render_to_string(partial: 'edit.html', locals: { entity: @entity })
          render partial: 'edit'
        end
      end
    end
  end


  def show
    respond_to do |format|
      format.js {
        @show = render_to_string(partial: 'entity', locals:  {
          entity: @entity,
          can_edit: current_user.can?(:edit, @agent),
          entities_list: @entities_list,
          agent: @agent,
          owner: @owner
        })
        render partial: 'show'
      }
    end
  end

  def show_detailed
    respond_to do |format|
      format.js {
        @show = render_to_string(partial: 'show_detailed.html', locals: { entity: @entity })
        render partial: 'show_detailed'
      }
    end
  end

  def destroy
    respond_to do |format|
      if @entity.destroy
        format.js { render partial: 'destroy_succeed' }
      else
        format.js { render partial: 'destroy_failed' }
      end
    end
  end

  def update_positions
    Entity.update_positions(@entities_list, params[:ids])
  end

  def new_import
    @entities_import = EntitiesImport.new
    render partial: 'new_import'
  end

  def create_import
    @entities_import = EntitiesImport.new(import_params)
    respond_to do |format|
      if @entities_list.from_csv @entities_import
        format.json {
          redirect_to user_agent_entities_list_path(@owner, @agent, @entities_list),
                      notice: t('views.entities_lists.show.import.select_import.success', count: @entities_import.count)
        }
      else
        format.json {
          render json: {
            replace_modal_content_with: render_to_string(partial: 'new_import', formats: :html),
          }, status: 422
        }
      end
    end
  end


  private

    def entity_params
      params.require(:entity).permit(:auto_solution_enabled, :terms, :solution)
    end

    def import_params
      params.permit(import: [:file, :mode])[:import]
    end

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = @owner.agents.friendly.find(params[:agent_id])
    end

    def set_entities_list
      @entities_list = @agent.entities_lists.friendly.find(params[:entities_list_id])
    end

    def set_entity
      @entity = @entities_list.entities.find(params[:id])
    end

    def check_user_rights
      case action_name
        when 'show', 'show_detailed'
          access_denied unless current_user.can? :show, @agent
        when 'create', 'edit', 'update', 'destroy', 'update_positions', 'new_import', 'create_import'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end
end
