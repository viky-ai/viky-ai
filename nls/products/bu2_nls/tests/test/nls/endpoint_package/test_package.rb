# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointPackage

    class TestPackage < Common

      def test_package_delete

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        url_delete = Nls.url_packages + "/voqal.ai:datetime2"
        actual = Nls.delete(url_delete)

        expected =
        {
          "status" => "Package 'voqal.ai:datetime2' successfully deleted"
        }

        assert_json expected, actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = fixture_path("several_packages_several_intents_after_remove.json")

        assert_json expected_dump, json_dump

      end

      def test_package_add

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_add = JSON.parse(File.read(fixture_path("package_to_add.json")))

        url_add = Nls.url_packages + "/voqal.ai:datetime"
        actual = Nls.query_post(url_add, json_package_to_add)

        expected =
        {
          "status" => "Package 'voqal.ai:datetime' successfully updated"
        }

        assert_json expected, actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = fixture_path("several_packages_several_intents_after_add.json")

        assert_json expected_dump, json_dump

      end

      def test_package_update

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_update = JSON.parse(File.read(fixture_path("package_to_update.json")))

        url_add = Nls.url_packages + "/voqal.ai:datetime2"
        actual = Nls.query_post(url_add, json_package_to_update)

        expected =
        {
          "status" => "Package 'voqal.ai:datetime2' successfully updated"
        }

        assert_json expected, actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = fixture_path("several_packages_several_intents_after_update.json")

        assert_json expected_dump, json_dump

      end

      def test_package_mismatch

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_update = JSON.parse(File.read(fixture_path("package_to_update.json")))

        expected_error = "OgNlsEndpoints : request error on endpoint"

        url_add = "#{Nls.url_packages}/voqal.ai:datetime1"
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(url_add, json_package_to_update)
        end
        assert_response_has_error expected_error, exception

      end

    end

  end

end
