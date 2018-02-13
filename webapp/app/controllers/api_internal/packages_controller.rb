class ApiInternal::PackagesController < ApiInternal::ApplicationController

  def index
    @agents = Agent.all.select(:id)
  end

  def show
    @agent = Agent.find(params[:id])
    render json: Nlp::Package.new(@agent).generate_json
  end
end
