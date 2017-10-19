require 'test_helper'

module Nls

  module FeedInit

    class TestDump < Common

      def test_package_dump

        cp_import_fixture("package_without_error.json")

        Nls.restart

        actual  = Nls.query_get(Nls.url_dump)

        expected =
        [
          {
            "id" => "voqal.ai:datetime",
            "slug" => "datetime",
            "interpretations" =>
            [
              {
                "id"=>"0d981484-9313-11e7-abc4-cec278b6b50b",
                "slug"=>"hello",
                "expressions" =>
                [
                  {
                    "expression" => "Hello Brice",
                    "locale"=>"fr-FR"
                  }
                ]
              }
            ]
          }
        ];

        assert_equal expected, actual
      end

    end

  end

end
