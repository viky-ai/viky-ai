require 'test_helper'

module Valgrind

  class ValgrindCommon < Nls::NlsTestCommon

    def setup

      resetDir

      several_packages_several_intents

      @main_package = @available_packages["datetime1"]
      @main_uuid = @main_package.id

    end

    def interpret_queries(nb_request_factor)

      # dump packages
      (30 * nb_request_factor).times do |i|
        actual_dump_result = Nls::Nls.query_get(Nls::Nls.url_dump)
        assert !actual_dump_result.nil?, "dump #{i}"
      end

      # launch simple query
      interpret_simple_query=
      {
        "packages" => [ @main_uuid ],
        "sentence" => "Hello Jean Marie",
        "Accept-Language" => "fr-FR"
      }

      (100 * nb_request_factor).times do
        response = Nls::Nls.interpret(interpret_simple_query)
        assert !response.nil?
      end

      # package update
      url_add = Nls::Nls.url_packages + "/#{@main_uuid}"

      expected_update_result =
      {
        "status" => "Package '#{@main_uuid}' successfully updated"
      }

      (30 * nb_request_factor).times do |i|
        actual_update_result = Nls::Nls.query_post(url_add, @main_package)
        assert_json expected_update_result, actual_update_result, "updating #{i}"
      end

      # launch queries with timeouts

      expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

      # timeout NlpPackageGet for hello world query
      hello_world_query = {
        timeout: 20,
        timeout_in: "NlpPackageGet"
      }

      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.interpret(interpret_simple_query, hello_world_query)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

      # timeout NlpPackageAddOrReplace
      params = {
        timeout: 20,
        timeout_in: "NlpPackageAddOrReplace"
      }

      json_package_to_update = full_minimal_package("titi1", "toto1", "Hello zorglub")

      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.package_update(json_package_to_update, params)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

      resetDir
      Nls::Interpretation.default_locale = "en-GB"

      # feed building feature_any
      pg_building_feature_any = create_building_feature_any
      pg_building_feature_any.to_file(importDir)

      Nls::Nls.restart

      # timeout NlpInterpretRequestParse
      params = {
        timeout: 20,
        timeout_in: "NlpInterpretRequestParse"
      }

      request = json_interpret_body(pg_building_feature_any, "with swimming pool with golf with sea view with spa", Nls::Interpretation.default_locale)

      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.interpret(request, params)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

      # timeout NlpMatchExpressions
      params = {
        timeout: 20,
        timeout_in: "NlpMatchExpressions"
      }

      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.interpret(request, params)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

      # timeout NlpRequestInterpretationBuild
      params = {
        timeout: 20,
        timeout_in: "NlpRequestInterpretationBuild"
      }

      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.interpret(request, params)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

      # timeout NlpInterpretRequestReset
      params = {
        timeout: 20,
        timeout_in: "NlpInterpretRequestReset"
      }

      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.interpret(request, params)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

    end

  end
end
