# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointPackage

    class TestPackage < NlsTestCommon

      def test_package_delete

        import_package = full_minimal_package("titi","toto","Hello Brice")
        import_package.to_file(importDir)

        import_package2 = full_minimal_package("titi1","toto1","Hello Jean Marie")
        import_package2.to_file(importDir)

        Nls.restart

        url_delete = Nls.url_packages + "/" + import_package2.id
        actual = Nls.delete(url_delete)

        assert_json Nls.expected_delete_package(import_package2.id), actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump = []
        expected_dump << import_package.to_h

        assert_json expected_dump, json_dump

      end

      def test_package_add

        import_package = full_minimal_package("titi","toto","Hello Brice")
        import_package.to_file(importDir)

        Nls.restart

        json_package_to_update = full_minimal_package("titi1", "toto1", "Hello zorglub")

        actual = Nls.package_update(json_package_to_update)

        assert_json Nls.expected_update_package(json_package_to_update.id), actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump  = []
        expected_dump << import_package.to_h
        expected_dump << json_package_to_update.to_h

        assert_json expected_dump, json_dump

      end

      def test_package_update

        import_package = full_minimal_package("titi1", "toto1", "Hello Brice")
        import_package.to_file(importDir)

        Nls.restart

        json_package_to_update = package_to_update(import_package.id, "titi1", "toto1", "Hello zorglub")

        actual = Nls.package_update(json_package_to_update)

        assert_json Nls.expected_update_package(json_package_to_update.id), actual

        json_dump = Nls.query_get(Nls.url_dump)

        expected_dump  = []
        expected_dump << json_package_to_update.to_h

        assert_json expected_dump, json_dump

      end

      def test_package_mismatch

        import_package = full_minimal_package("titi", "toto", "Hello Brice")
        import_package.to_file(importDir)
        import_package2 = full_minimal_package("titi1", "toto1", "Hello Jean Marie")
        import_package2.to_file(importDir)

        Nls.restart

        json_package_to_update = package_to_update(import_package.id, "titi", "toto", "Hello zorglub")

        expected_error = "OgNlsEndpoints : request error on endpoint"

        url_add = "#{Nls.url_packages}/nimportequoi"
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(url_add, json_package_to_update)
        end
        assert_response_has_error expected_error, exception

      end

      def test_package_lock_parallel

        several_packages_several_intents
        json_package_to_update = available_packages['datetime3']

        Nls.restart

        json_interpretation = json_package_to_update.interpretation('hello2')
        sentence = "Hello Sebastien"

        expected_interpret_result = {
                  "interpretations" => [ json_interpretation.to_match  ]
                }

        tab = (0..10).to_a
        Parallel.map(tab, in_threads: 20) do |i|

          # querying
          actual_interpret_result = Nls.interpret_package(json_package_to_update, sentence)
          assert_json expected_interpret_result, actual_interpret_result, "querying #{i}"

          # updating
          actual_update_result = Nls.package_update(json_package_to_update)
          assert_json Nls.expected_update_package(json_package_to_update.id), actual_update_result, "updating #{i}"

          # re-querying
          actual_interpret_result = Nls.interpret_package(json_package_to_update, sentence)
          assert_json expected_interpret_result, actual_interpret_result, "re-querying #{i}"

          # re-updating
          actual_update_result = Nls.package_update(json_package_to_update)
          assert_json Nls.expected_update_package(json_package_to_update.id), actual_update_result, "re-updating #{i}"

        end
      end

      def test_package_simplelock

        several_packages_several_intents
        json_package_to_update = available_packages['datetime3']

        Nls.restart

        json_interpretation = json_package_to_update.interpretation('hello2')
        sentence = "Hello Sebastien"

        expected_interpret_result = {
                  "interpretations" => [ json_interpretation.to_match  ]
                }

        # querying
        actual_interpret_result = Nls.interpret_package(json_package_to_update, sentence)
        assert_json expected_interpret_result, actual_interpret_result, "querying"

        # updating
        actual_update_result = Nls.package_update(json_package_to_update)
        assert_json Nls.expected_update_package(json_package_to_update.id), actual_update_result, "updating"

        # re-querying
        actual_interpret_result = Nls.interpret_package(json_package_to_update, sentence)
        assert_json expected_interpret_result, actual_interpret_result, "re-querying"

        # re-updating
        actual_update_result = Nls.package_update(json_package_to_update)
        assert_json Nls.expected_update_package(json_package_to_update.id), actual_update_result, "re-updating"

      end

    end

  end

end
