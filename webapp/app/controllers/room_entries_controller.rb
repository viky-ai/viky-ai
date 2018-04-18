class RoomEntriesController < ApplicationController

  def create
    @content = entry_params[:content]
    respond_to do |format|
      format.js {
        unless @content.blank?
          @html = render_to_string(
            partial: 'bubble',
            locals: { content: @content }
          )
          render partial: 'create_succeed'
        end
      }
    end
  end

  private

    def entry_params
      params.require(:entry).permit(:content)
    end

end
