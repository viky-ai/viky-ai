class EntitiesListsExportsController < ApplicationController
  include ActionController::Live

  def show
    @owner = User.friendly.find(params[:user_id])
    @agent = Agent.owned_by(@owner).friendly.find(params[:agent_id])
    @entities_list = @agent.entities_lists.friendly.find(params[:id])

    access_denied unless current_user.can? :show, @agent

    respond_to do |format|
      format.csv do
        # Stream CSV data in order to minimize RAM usage
        # and display the save file popup immediately.
        filename = [
          @owner.username,
          @agent.agentname,
          @entities_list.listname,
          Time.current.strftime('%Y-%m-%d'),
        ].join("_") + ".csv"
        response.headers['Content-Disposition'] = 'attachment; filename="' + filename + '"'
        response.headers['Content-Type'] = 'text/csv'
        begin
          @entities_list.to_csv_in_io(response.stream)
        ensure
          response.stream.close
        end
      end
    end
  end

end
