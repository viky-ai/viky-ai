require 'test_helper'

module Nls

  module FeedInit

    class TestDump < NlsTestCommon

      def test_package_dump

        several_packages_several_intents

        Nls.restart

        actual  = Nls.query_get(Nls.url_dump)

        assert_equal JSON.parse(@packages_dump.to_json), actual
      end

    end

  end

end
