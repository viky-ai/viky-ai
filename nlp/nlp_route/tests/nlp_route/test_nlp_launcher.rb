require 'simplecov'
SimpleCov.start

require 'minitest/autorun'
require 'mocha/mini_test'

require 'redis'
require 'nlp_route'

class NlpRoute::TestNlpLauncher < MiniTest::Unit::TestCase

  def test_init
    package_list = ["1", "2"]
    package_1 = { id: "1" }
    package_2 = { id: "2" }

    mock = NlpRoute::PackageApiWrapper.any_instance
    mock.expects(:list_id).returns(package_list)
    mock.expects(:get_package).with("1").returns(package_1)
    mock.expects(:update_or_create).with("1", package_1).returns(true)
    mock.expects(:get_package).with("2").returns(package_2)
    mock.expects(:update_or_create).with("2", package_2).returns(true)

    launcher = NlpRoute::NlpLauncher.new
    launcher.init

  end

  def test_subscribe

    Thread.new {
      package_1 = { id: "1" }
      package_2 = { id: "2" }

      mock = NlpRoute::PackageApiWrapper.any_instance
      mock.expects(:get_package).with("1").returns(package_1)
      mock.expects(:update_or_create).with("1", package_1).returns(true)

      mock.expects(:get_package).with("2").returns(package_2)
      mock.expects(:update_or_create).with("2", package_2).returns(true)

      mock.expects(:delete).with("2").returns(true)

      launcher = NlpRoute::NlpLauncher.new
      launcher.subscribe()
    }

    sleep 0.2

    notify("1", :update)
    sleep 0.1
    notify("2", :update)
    sleep 0.1
    notify("2", :delete)
    sleep 0.1
    notify(nil, :unsubscribe)
  end

  def notify id, event
    redis_opts = {
      url: ENV.fetch("VIKYAPP_REDIS_PACKAGE_NOTIFIER") { 'redis://localhost:6379/3' }
    }
    redis = Redis.new(redis_opts)

    redis.publish(:viky_packages_change_notifications, { event: event, id: id }.to_json  )
  end

end
