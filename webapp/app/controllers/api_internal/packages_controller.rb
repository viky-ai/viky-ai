class ApiInternal::PackagesController < ApiInternal::ApplicationController

  def index
    @agents = Agent.all.select(:id)
  end

  def show
    @agent = Agent.find(params[:id])
    @interpretations = Nlp::Package.new(@agent).build_tree
  end

end
