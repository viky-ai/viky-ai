require 'test_helper'

module Valgrind

  class ValgrindCommon < Nls::NlsTestCommon

    def setup

      resetDir

      Nls::Interpretation.default_locale = "en-GB"

    end

    def interpret_queries(nb_request_factor, skip_timeout = false)
      filename = File.basename(__FILE__)

      ruby_log_append(filename, "building packages")

      # several_packages_several_intents
      pg_building_feature_any = create_building_feature_any
      Nls::Nls.package_update(pg_building_feature_any)

      numbers_package = create_numbers_package
      Nls::Nls.package_update(numbers_package)

      regex_package = create_regex_package
      Nls::Nls.package_update(regex_package)

      @main_package = @available_packages["pg-building-feature"]
      @main_uuid = @main_package.id.to_s

      @number_package = @available_packages["numbers"]
      @number_uuid = @number_package.id.to_s

      @regex_package = @available_packages["regex_package"]
      @regex_uuid = @regex_package.id.to_s

      ruby_log_append(filename, "building queries")
      interpret_query=
      {
        "packages" => [ @main_uuid ],
        "sentence" => "with swimming pool with golf",
        "Accept-Language" => Nls::Interpretation.default_locale
      }

      numbers_query=
      {
        "packages" => [ @number_uuid ],
        "sentence" => "a 12,345.678 b",
        "Accept-Language" => Nls::Interpretation.default_locale
      }

      regex_query=
      {
        "packages" => [ @regex_uuid ],
        "sentence" => "mail patrick@pertimm.com sebastien@pertimm.com",
        "Accept-Language" => Nls::Interpretation.default_locale
      }

      error_query=
      {
        "packages" => [ @main_uuid ],
        "sentence" => "with error js piscine",
        "Accept-Language" => Nls::Interpretation.default_locale
      }

      expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

      params = {
        timeout: 20000,
      }

      ruby_log_append(filename, "test dump packages")
      # dump packages
      (10 * nb_request_factor).times do |i|
        actual_dump_result = Nls::Nls.dump
        assert !actual_dump_result.nil?, "dump #{i}"
      end

      ruby_log_append(filename, "test simple query")
      # launch simple query
      (10 * nb_request_factor).times do
        response = Nls::Nls.interpret(interpret_query, params)
        assert !response.nil?
      end

      ruby_log_append(filename, "test number query")
      # launch number query
      (10 * nb_request_factor).times do
        response = Nls::Nls.interpret(numbers_query, params)
        assert !response.nil?
      end

      ruby_log_append(filename, "test regex query")
      # launch regex query
      (10 * nb_request_factor).times do
        response = Nls::Nls.interpret(regex_query, params)
        assert !response.nil?
      end

      ruby_log_append(filename, "test error query")
      # launch regex query
      (10 * nb_request_factor).times do
        assert_raises RestClient::InternalServerError, "'undef_variable'" do
          Nls::Nls.interpret(error_query, params)
        end
      end

      ruby_log_append(filename, "test package update")
      # package update
      url_add = Nls::Nls.url_packages + "/#{@main_uuid}"

      expected_update_result =
      {
        "status" => "Package '#{@main_uuid}' successfully updated"
      }

      (10 * nb_request_factor).times do |i|
        actual_update_result = Nls::Nls.query_post(url_add, @main_package)
        assert_json expected_update_result, actual_update_result, "updating #{i}"
      end

      # launch queries with timeouts

      if !skip_timeout
        ruby_log_append(filename, "tests with timeout")

        ruby_log_append(filename, "timeout NlpPackageGet for nlp recursive query")
        # timeout NlpPackageGet for nlp recursive query
        params = {
          timeout: 20,
          timeout_in: "NlpPackageGet"
        }

        (10 * nb_request_factor).times do |i|
          exception = assert_raises RestClient::ExceptionWithResponse do
            Nls::Nls.interpret(interpret_query, params)
          end
          assert_response_has_error expected_error, exception, "Timeout #{i}"
        end

        ruby_log_append(filename, "timeout NlpInterpretRequestParse")
        # timeout NlpInterpretRequestParse
        params = {
          timeout: 20,
          timeout_in: "NlpInterpretRequestParse"
        }

        (10 * nb_request_factor).times do |i|
          exception = assert_raises RestClient::ExceptionWithResponse do
            Nls::Nls.interpret(interpret_query, params)
          end
          assert_response_has_error expected_error, exception, "Timeout #{i}"
        end

        ruby_log_append(filename, "timeout NlpMatchExpressions")
        # timeout NlpMatchExpressions
        params = {
          timeout: 20,
          timeout_in: "NlpMatchExpressions"
        }

        (10 * nb_request_factor).times do |i|
          exception = assert_raises RestClient::ExceptionWithResponse do
            Nls::Nls.interpret(interpret_query, params)
          end
          assert_response_has_error expected_error, exception, "Timeout #{i}"
        end

        ruby_log_append(filename, "timeout NlpRequestInterpretationBuild")
        # timeout NlpRequestInterpretationBuild
        params = {
          timeout: 20,
          timeout_in: "NlpRequestInterpretationBuild"
        }

        (10 * nb_request_factor).times do |i|
          exception = assert_raises RestClient::ExceptionWithResponse do
            Nls::Nls.interpret(interpret_query, params)
          end
          assert_response_has_error expected_error, exception, "Timeout #{i}"
        end

        ruby_log_append(filename, "timeout NlpPackageAddOrReplace")
        # timeout NlpPackageAddOrReplace
        params = {
          timeout: 20,
          timeout_in: "NlpPackageAddOrReplace"
        }

        (10 * nb_request_factor).times do |i|
          exception = assert_raises RestClient::ExceptionWithResponse do
            Nls::Nls.package_update(@main_package, params)
          end
          assert_response_has_error expected_error, exception, "Timeout #{i}"
        end

      end

    end

  end
end
