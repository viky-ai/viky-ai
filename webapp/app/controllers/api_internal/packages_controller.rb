class ApiInternal::PackagesController < ApiInternal::ApplicationController
  include ActionController::Live

  def index
    @agents = Agent.all.select(:id)
  end

  def show
    Rails.logger.silence(Logger::INFO) do
      time = Benchmark.measure do
        @agent = Agent.find(params[:id])
        response.headers['Content-Type'] = 'application/json'
        headers['ETag'] = @agent.updated_at.iso8601(9)
        begin
          Nlp::Package.new(@agent).generate_json(response.stream)
        ensure
          response.stream.close
        end
      end
      Rails.logger.info("  Generate package #{@agent.id} - #{@agent.slug} in #{(time.real*1000.0).round(1)} ms")
    end
  end

  def updated
    if params['status'] == 'success'
      agent = Agent.find(params['id'])
      unless agent.synced_with_nlp?
        if DateTime.parse(params['version']) >= agent.updated_at.to_datetime
          agent.update_columns(nlp_updated_at: params['nlp_updated_at'])
          agent.run_regression_checks
        end
      end
    else
      Rails.logger.warn("  Error while updating package (#{params['id']}) on NLS : #{params['error']}")
    end
    head :no_content
  end
end
