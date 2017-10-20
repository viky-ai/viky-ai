# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointPackage

    class TestPackage < Common

      def test_package_delete

        import_json = []
        import_package = Nls.package_to_add("titi", "toto", "Hello Brice")
        import_json << import_package
        import_package2 = Nls.package_to_add("titi1", "toto1", "Hello Jean Marie")
        import_json << import_package2

        generate_multiple_package_file(import_json)

        Nls.restart

        url_delete = Nls.url_packages + "/" + import_package2["id"]
        actual = Nls.delete(url_delete)

        assert_json Nls.expected_delete_package(import_package2["id"]), actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = []
        expected_dump << import_package

        assert_json expected_dump, json_dump

      end

      def test_package_add

        import_json = []
        import_package = Nls.package_to_add("titi", "toto", "Hello Brice")
        import_json << import_package
        generate_multiple_package_file(import_json)

        Nls.restart

        json_package_to_update = Nls.package_to_add("titi1", "toto1", "Hello zorglub")
        generate_single_package_file(json_package_to_update)

        actual = Nls.package(json_package_to_update)

        assert_json Nls.expected_update_package(json_package_to_update["id"]), actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump  = []
        expected_dump << import_package
        expected_dump << json_package_to_update

        assert_json expected_dump, json_dump

      end

      def test_package_update

        import_json = []
        import_package = Nls.package_to_add("titi", "toto", "Hello Brice")
        import_json << import_package
        generate_multiple_package_file(import_json)

        Nls.restart

        json_package_to_update = Nls.package_to_update(import_package["id"], "Hello zorglub")
        generate_single_package_file(json_package_to_update)

        actual = Nls.package(json_package_to_update)

        assert_json Nls.expected_update_package(json_package_to_update["id"]), actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump  = []
        expected_dump << json_package_to_update

        assert_json expected_dump, json_dump

      end

      def test_package_mismatch

        import_json = []
        import_package = Nls.package_to_add("titi", "toto", "Hello Brice")
        import_json << import_package
        import_package2 = Nls.package_to_add("titi1", "toto1", "Hello Jean Marie")
        import_json << import_package2

        generate_multiple_package_file(import_json)

        Nls.restart

        json_package_to_update = Nls.package_to_update(import_package["id"], "Hello zorglub")

        expected_error = "OgNlsEndpoints : request error on endpoint"

        url_add = "#{Nls.url_packages}/nimportequoi"
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(url_add, json_package_to_update)
        end
        assert_response_has_error expected_error, exception

      end

      def test_package_lock_parallel

        json_structure = Nls.several_packages_several_intents
        generate_multiple_package_file(json_structure)

        Nls.restart

        json_package_to_update = json_structure[2]

        package_id = json_package_to_update["id"]
        expression = json_package_to_update["interpretations"][0]["expressions"][1]["expression"]
        interpretation_id = json_package_to_update["interpretations"][0]["id"]
        slug = json_package_to_update["interpretations"][0]["slug"]

        json_interpret_body = Nls.json_interpret_body(package_id, expression)
        expected_interpret_result = Nls.expected_interpret_result(package_id, interpretation_id, slug)

        tab = (0..10).to_a
        Parallel.map(tab, in_threads: 20) do |i|

          # querying
          actual_interpret_result = Nls.interpret(json_interpret_body)
          assert_json expected_interpret_result, actual_interpret_result, "querying #{i}"

          # updating
          actual_update_result = Nls.package(json_package_to_update)
          assert_json Nls.expected_update_package(json_package_to_update["id"]), actual_update_result, "updating #{i}"

          # re-querying
          actual_interpret_result = Nls.interpret(json_interpret_body)
          assert_json expected_interpret_result, actual_interpret_result, "re-querying #{i}"

          # re-updating
          actual_update_result = Nls.package(json_package_to_update)
          assert_json Nls.expected_update_package(json_package_to_update["id"]), actual_update_result, "re-updating #{i}"

        end
      end

      def test_package_simplelock

        json_structure = Nls.several_packages_several_intents
        generate_multiple_package_file(json_structure)

        Nls.restart

        json_package_to_update = json_structure[2]

        package_id = json_package_to_update["id"]
        expression = json_package_to_update["interpretations"][0]["expressions"][1]["expression"]
        interpretation_id = json_package_to_update["interpretations"][0]["id"]
        slug = json_package_to_update["interpretations"][0]["slug"]

        json_interpret_body = Nls.json_interpret_body(package_id, expression)
        expected_interpret_result = Nls.expected_interpret_result(package_id, interpretation_id, slug)

        # querying
        actual_interpret_result = Nls.interpret(json_interpret_body)
        assert_json expected_interpret_result, actual_interpret_result, "querying"

        # updating
        actual_update_result = Nls.package(json_package_to_update)
        assert_json Nls.expected_update_package(json_package_to_update["id"]), actual_update_result, "updating"

        # re-querying
        actual_interpret_result = Nls.interpret(json_interpret_body)
        assert_json expected_interpret_result, actual_interpret_result, "re-querying"

        # re-updating
        actual_update_result = Nls.package(json_package_to_update)
        assert_json Nls.expected_update_package(json_package_to_update["id"]), actual_update_result, "re-updating"

      end

    end

  end

end
