require 'test_helper'

module Nls

  module FeedInit

    class TestDump < Common

      def test_package_dump

        json_structure = Nls.several_packages_several_intents
        generate_multiple_package_file(json_structure)

        Nls.restart

        actual  = Nls.query_get(Nls.url_dump)

        assert_equal json_structure, actual
      end

    end

  end

end
