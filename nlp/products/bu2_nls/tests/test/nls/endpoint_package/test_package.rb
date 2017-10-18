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

        assert_json Nls.expected_delete_package, actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = fixture_path("several_packages_several_intents_after_remove.json")

        assert_json expected_dump, json_dump

      end

      def test_package_add

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_add = json_from_fixture("package_to_add.json")

        actual = Nls.package(json_package_to_add)

        assert_json Nls.expected_added_package, actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = fixture_path("several_packages_several_intents_after_add.json")

        assert_json expected_dump, json_dump

      end

      def test_package_update

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_update = json_from_fixture("package_to_update.json")

        actual = Nls.package(json_package_to_update)

        assert_json Nls.expected_update_package, actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = fixture_path("several_packages_several_intents_after_update.json")

        assert_json expected_dump, json_dump

      end

      def test_package_mismatch

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_update = json_from_fixture("package_to_update.json")

        expected_error = "OgNlsEndpoints : request error on endpoint"

        url_add = "#{Nls.url_packages}/voqal.ai:datetime1"
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(url_add, json_package_to_update)
        end
        assert_response_has_error expected_error, exception

      end

      def test_package_lock_parallel

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_update = json_from_fixture("package_to_update.json")

        tab = (0..10).to_a
        Parallel.map(tab, in_threads: 20) do |i|

          # querying
          actual_interpret_result = Nls.interpret(Nls.json_interpret_body)
          assert_json Nls.expected_interpret_result, actual_interpret_result, "querying #{i}"

          # updating
          actual_update_result = Nls.package(json_package_to_update)
          assert_json Nls.expected_update_package, actual_update_result, "updating #{i}"

          # re-querying
          actual_interpret_result = Nls.interpret(Nls.json_interpret_body)
          assert_json Nls.expected_interpret_result, actual_interpret_result, "re-querying #{i}"

          # re-updating
          actual_update_result = Nls.package(json_package_to_update)
          assert_json Nls.expected_update_package, actual_update_result, "re-updating #{i}"

        end
      end

      def test_package_simplelock
        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        json_package_to_update = json_from_fixture("package_to_update.json")

        # querying
        actual_interpret_result = Nls.interpret(Nls.json_interpret_body)
        assert_json Nls.expected_interpret_result, actual_interpret_result, "querying"

        # updating
        actual_update_result = Nls.package(json_package_to_update)
        assert_json Nls.expected_update_package, actual_update_result, "updating"

        # re-querying
        actual_interpret_result = Nls.interpret(Nls.json_interpret_body)
        assert_json Nls.expected_interpret_result, actual_interpret_result, "re-querying"

        # re-updating
        actual_update_result = Nls.package(json_package_to_update)
        assert_json Nls.expected_update_package, actual_update_result, "re-updating"

      end

    end

  end

end
