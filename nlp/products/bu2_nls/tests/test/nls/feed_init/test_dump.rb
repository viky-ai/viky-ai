require 'test_helper'

module Nls

  module FeedInit

    class TestDump < NlsTestCommon

      def test_package_dump

        Nls.remove_all_packages

        several_packages_several_intents.each do |p|
          Nls.package_update(p)
        end

        actual  = Nls.query_get(Nls.url_dump)

        assert_equal @packages_dump, actual
      end

    end

  end

end
