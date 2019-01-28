class ApiInternal::PackagesController < ApiInternal::ApplicationController

  def index
    @agents = Agent.all.select(:id)
  end

  def show
    Rails.logger.silence(Logger::INFO) do
      time = Benchmark.measure do
        @agent = Agent.find(params[:id])
        json = Nlp::Package.new(@agent).generate_json
        headers['ETag'] = @agent.updated_at
        render json: json
      end
      Rails.logger.info("  Generate package #{@agent.id} - #{@agent.slug} in #{(time.real*1000.0).round(1)} ms")
    end
  end
end
