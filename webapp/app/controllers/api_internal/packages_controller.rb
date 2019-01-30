class ApiInternal::PackagesController < ApiInternal::ApplicationController

  def index
    @agents = Agent.all.select(:id)
  end

  def show
    Rails.logger.silence(Logger::INFO) do
      time = Benchmark.measure do
        @agent = Agent.find(params[:id])
        json = Nlp::Package.new(@agent).generate_json
        headers['ETag'] = @agent.updated_at.iso8601(9)
        render json: json
      end
      Rails.logger.info("  Generate package #{@agent.id} - #{@agent.slug} in #{(time.real*1000.0).round(1)} ms")
    end
  end

  def updated
    if params['status'] == 'success'
      agent = Agent.find(params['id'])
      unless agent.synced_with_nlp?
        if DateTime.parse(params['version']) >= agent.updated_at.to_datetime
          agent.nlp_updated_at = params['nlp_updated_at']
          agent.save
          agent.run_tests
        end
      end
    else
      Rails.logger.warn("  Error while updating package (#{params['id']}) on NLS : #{params['error']}")
    end
  end
end
