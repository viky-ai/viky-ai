class HealthCheck

  def initialize(app)
    @app = app
  end

  def call(env)
    case env['PATH_INFO']
    when '/database_check.json'
      begin
        database_up?
        return [200, { 'Content-Type' => 'application/json' }, [{ message: 'Database connection is up' }.to_json ]]
      rescue => e
        return [500, { 'Content-Type' => 'application/json' }, [{ message: "Database connection is down : #{e.message}", info: e }.to_json ] ]
      end
    end
    @app.call(env)
  end


  private

    def database_up?
      ActiveRecord::Base.connection_pool.with_connection { |con| con.active? }
    end
end
